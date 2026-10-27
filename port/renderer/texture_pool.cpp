// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/texture_pool.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "common/env.h"
#include "common/log.h"
#include "common/parallel.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/shader.h"

namespace openrac::renderer {

using namespace openrac::gl;

namespace {

// The bytes of an RGBA8 image of `width` by `height` texels.
std::size_t texel_bytes(int width, int height) {
    constexpr std::size_t kRgba8Bytes = 4;
    return static_cast<std::size_t>(std::max(width, 0))
           * static_cast<std::size_t>(std::max(height, 0)) * kRgba8Bytes;
}

// The bytes of an RGBA8 image with its whole mip chain, each level half the one above down to one
// texel, as glGenerateMipmap makes it.
std::size_t chain_bytes(int width, int height) {
    std::size_t bytes = texel_bytes(width, height);

    // One level a turn, until both sides are a texel.
    while (width > 1 || height > 1) {
        width = std::max(width / 2, 1);
        height = std::max(height / 2, 1);
        bytes += texel_bytes(width, height);
    }

    return bytes;
}

}  // namespace

std::uint64_t hash_bytes(const std::uint8_t* data, std::size_t size) {
    // Eight bytes a step (a byte a step made it one of the render thread's costliest functions on
    // a phone), each word mixed in by a multiply and a rotate, and the whole finished by
    // splitmix64's mix so that every input bit reaches every output bit.
    constexpr std::uint64_t kMul = 0x9E3779B97F4A7C15ull;
    std::uint64_t h = 0xcbf29ce484222325ull ^ (size * kMul);
    std::size_t i = 0;
    for (; i + 8 <= size; i += 8) {
        std::uint64_t w;
        std::memcpy(&w, data + i, 8);
        h = std::rotl(h ^ (w * kMul), 29) * 0xBF58476D1CE4E5B9ull;
    }
    if (i < size) {
        std::uint64_t w = 0;
        std::memcpy(&w, data + i, size - i);
        h = std::rotl(h ^ (w * kMul), 29) * 0xBF58476D1CE4E5B9ull;
    }
    h ^= h >> 30;
    h *= 0xBF58476D1CE4E5B9ull;
    h ^= h >> 27;
    h *= 0x94D049BB133111EBull;
    return h ^ (h >> 31);
}

std::size_t TexturePool::KeyHash::operator()(const Key& key) const {
    const std::uint64_t words[] = {
        key.tbp | (std::uint64_t{key.tbw} << 16) | (std::uint64_t{key.tw} << 32)
            | (std::uint64_t{key.th} << 40) | (std::uint64_t{key.psm} << 48)
            | (std::uint64_t{key.cpsm} << 56),
        key.cbp | (std::uint64_t{key.texa.ta0} << 16) | (std::uint64_t{key.texa.ta1} << 24)
            | (std::uint64_t{key.texa.aem} << 32),
        key.pixels_hash,
        key.clut_hash,
    };
    return static_cast<std::size_t>(
        hash_bytes(reinterpret_cast<const std::uint8_t*>(words), sizeof(words))
    );
}

TexturePool::TexturePool() {
    // Magenta and black, 8 x 8 texels of 4 x 4: obviously wrong on screen.
    Rgba8Image checker(8, 8);
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const bool on = ((x / 4) + (y / 4)) % 2 == 0;
            checker.set_texel(x, y, on ? rgba(0xFF, 0x00, 0xFF, 0x80) : rgba(0, 0, 0, 0x80));
        }
    }
    m_placeholder = add(std::move(checker), AlphaScale::Gs, "placeholder");
}

TexturePool::~TexturePool() {
    // GL objects are released by release_gl() while the context exists; by
    // now it may not.
}

TextureHandle TexturePool::new_entry() {
    if (!m_free.empty()) {
        const TextureHandle h = m_free.back();
        m_free.pop_back();
        return h;
    }
    m_entries.emplace_back();
    return static_cast<TextureHandle>(m_entries.size());
}

TextureHandle TexturePool::add(Rgba8Image image, AlphaScale scale, std::string name, bool mipmaps, bool linear) {
    const TextureHandle h = new_entry();
    Entry& e = m_entries[h - 1];
    e = Entry{};
    e.live = true;
    e.image = std::move(image);
    e.scale = scale;
    e.name = std::move(name);
    e.mipmaps = mipmaps;
    e.linear = linear;
    e.last_used = m_frame;
    return h;
}

void TexturePool::place(std::uint32_t tbp, TextureHandle texture) {
    m_placed[tbp] = texture;
}

void TexturePool::clear_placements() {
    m_placed.clear();
}

// ---- Source uploads ----

bool TexturePool::repeat_upload(const ImageUpload& upload) {
    auto found = m_uploads.find(upload.dbp);
    if (found == m_uploads.end()) {
        return false;
    }
    Stored& stored = found->second;
    const ImageUpload& old = stored.upload;
    if (old.dbw != upload.dbw || old.dpsm != upload.dpsm || old.x != upload.x || old.y != upload.y
        || old.width != upload.width || old.height != upload.height || old.data != upload.data) {
        return false;
    }
    // Even unchanged transfers are ordering events. A later overlapping upload must not hide this
    // one, nor can a placement survive it.
    stored.sequence = ++m_sequence;
    m_placed.erase(upload.dbp);
    return true;
}

void TexturePool::upload(ImageUpload upload) {
    // The same transfer again (the 2D layer sends its pages every frame): only its order changes,
    // without hashing the bytes again.
    if (repeat_upload(upload)) {
        return;
    }
    Stored stored;
    const std::uint64_t shape[] = {upload.width, upload.height, upload.dpsm, upload.dbw, upload.x, upload.y};
    stored.hash = hash_bytes(upload.data.data(), upload.data.size())
                  ^ (hash_bytes(reinterpret_cast<const std::uint8_t*>(shape), sizeof(shape)) << 1);
    stored.sequence = ++m_sequence;
    // The pages the rectangle's last row reaches, in its format's page size (a page is 32 blocks
    // whatever the format: 64 x 32 pixels of 32 bits, 64 x 64 of 16, 128 x 64 of 8, 128 x 128 of 4).
    const int bits = std::max(gs::bits_per_pixel(upload.dpsm), 4);
    const std::uint64_t line = static_cast<std::uint64_t>(std::max<std::uint32_t>(upload.dbw, 1)) * 64 * bits / 8;
    const std::uint64_t bytes = (static_cast<std::uint64_t>(upload.y) + std::max<std::uint32_t>(upload.height, 1)) * line;
    stored.end = upload.dbp + static_cast<std::uint32_t>((bytes + 8191) / 8192 * 32);
    stored.upload = std::move(upload);
    const std::uint32_t dbp = stored.upload.dbp;
    m_uploads[dbp] = std::move(stored);
    // An upload replaces a texture placed at the same base pointer.
    m_placed.erase(dbp);
}

bool TexturePool::upload_cached(const ImageUpload& upload) {
    if (repeat_upload(upload)) {
        return true;
    }
    this->upload(upload);
    return false;
}

const TexturePool::Stored* TexturePool::upload_at(std::uint32_t block, std::uint32_t& offset) const {
    offset = 0;
    if (auto it = m_uploads.find(block); it != m_uploads.end()) {
        return &it->second;
    }
    const Stored* best = nullptr;
    for (const auto& [base, stored] : m_uploads) {
        if (base < block && block < stored.end && base < 0x10000u && (best == nullptr || stored.sequence > best->sequence)) {
            best = &stored;
        }
    }
    if (best != nullptr) {
        offset = block - best->upload.dbp;
    }
    return best;
}

std::vector<std::uint8_t> TexturePool::texture_raster(const Stored& stored, const gs::Tex0& tex0)
    const {
    const ImageUpload& up = stored.upload;
    const int tw = tex0.width();
    const int th = tex0.height();
    const int up_w = static_cast<int>(up.width);
    const int up_h = static_cast<int>(up.height);
    if (tex0.tbp0 != up.dbp || up.x != 0 || up.y != 0) {
        // Inside a larger transfer, or a transfer into the middle of a buffer: through the chip's
        // layout, from where the rectangle lies to where the texture starts.
        return reinterpret_at(up.data, up.dpsm, static_cast<int>(up.x), static_cast<int>(up.y), up_w, up_h, up.dbw,
                              tex0.tbp0 - up.dbp, tex0.psm, tw, th, std::max<std::uint32_t>(tex0.tbw, 1));
    }
    const bool both_16 = gs::bits_per_pixel(up.dpsm) == 16 && gs::bits_per_pixel(tex0.psm) == 16;
    if (up.dpsm != tex0.psm && !both_16) {
        // Sent in one format and read in another (an 8-bit texture sent as
        // 32-bit pixels, say): the only case that needs the chip's layout.
        return reinterpret(
            up.data,
            up.dpsm,
            up_w,
            up_h,
            up.dbw,
            tex0.psm,
            tw,
            th,
            std::max<std::uint32_t>(tex0.tbw, 1)
        );
    }
    // Same format: the rectangle, cropped or padded to the texture's size.
    std::vector<std::uint8_t> out(image_bytes(tex0.psm, tw, th));
    const int raster = raster_bits(tex0.psm);
    if (raster % 8 == 0) {
        const auto bytes = static_cast<std::size_t>(raster / 8);
        for (int y = 0; y < std::min(th, up_h); ++y) {
            const std::size_t src = static_cast<std::size_t>(y * up_w) * bytes;
            const std::size_t dst = static_cast<std::size_t>(y * tw) * bytes;
            const std::size_t count = static_cast<std::size_t>(std::min(tw, up_w)) * bytes;
            if (src + count <= up.data.size()) {
                std::copy_n(
                    up.data.begin() + static_cast<std::ptrdiff_t>(src),
                    count,
                    out.begin() + static_cast<std::ptrdiff_t>(dst)
                );
            }
        }
    } else {
        // 4-bit: nibble by nibble.
        for (int y = 0; y < std::min(th, up_h); ++y) {
            for (int x = 0; x < std::min(tw, up_w); ++x) {
                const auto s = static_cast<std::size_t>(y * up_w + x);
                const auto d = static_cast<std::size_t>(y * tw + x);
                if (s / 2 >= up.data.size()) {
                    continue;
                }
                const std::uint8_t v = (s & 1) != 0 ? up.data[s / 2] >> 4 : up.data[s / 2] & 0x0F;
                out[d / 2] = static_cast<std::uint8_t>(
                    (d & 1) != 0 ? (out[d / 2] & 0x0F) | (v << 4) : (out[d / 2] & 0xF0) | v
                );
            }
        }
    }
    return out;
}

bool TexturePool::clut_in_memory(const gs::Tex0& tex0, Clut& out) const {
    const int entries = palette_entries(tex0.psm);
    std::uint32_t offset = 0;
    const Stored* stored = entries == 0 ? nullptr : upload_at(tex0.cbp, offset);
    if (stored == nullptr) {
        return false;
    }
    const ImageUpload& up = stored->upload;
    out.cpsm = tex0.cpsm;
    out.entries = entries;
    if (offset == 0 && up.x == 0 && up.y == 0) {
        out.data = up.data;
        out.cpsm = up.dpsm;
        out.width = std::max<int>(static_cast<int>(up.width), 1);
        out.hash = stored->hash;
        return true;
    }
    // Inside a larger transfer: the CLUT's own rectangle (16 x 16 for 256 entries, 8 x 2 for 16),
    // read out of it through the chip's layout.
    out.width = entries == 256 ? 16 : 8;
    const int height = entries == 256 ? 16 : 2;
    out.data = reinterpret_at(up.data, up.dpsm, static_cast<int>(up.x), static_cast<int>(up.y),
                              static_cast<int>(up.width), static_cast<int>(up.height), up.dbw, offset, tex0.cpsm,
                              out.width, height, 1);
    out.hash = stored->hash ^ (static_cast<std::uint64_t>(offset) * 0x9E3779B97F4A7C15ull);
    return true;
}

void TexturePool::load_clut(const gs::Tex0& tex0) {
    Clut clut;
    if (clut_in_memory(tex0, clut)) {
        m_cluts[tex0.cbp] = std::move(clut);
    } else {
        // Nothing there yet: the draws read memory as before (the chip would load whatever it held).
        m_cluts.erase(tex0.cbp);
    }
}

const TexturePool::Clut* TexturePool::palette_clut(const gs::Tex0& tex0, Clut& now) {
    const int entries = palette_entries(tex0.psm);
    if (tex0.csm) {
        // CSM2 (a CLUT as one row of a larger image, placed by TEXCLUT) is
        // not used by the games read so far.
        log::debug("CSM2 CLUT at block {:#x} read as CSM1", tex0.cbp);
    }
    // The CLUT as last loaded from this block, else as memory holds it now.
    if (auto it = m_cluts.find(tex0.cbp); it != m_cluts.end() && it->second.entries >= entries) {
        return &it->second;
    }
    if (clut_in_memory(tex0, now)) {
        return &now;
    }
    static log::Capped told(50);
    if (log::verbose_on() && told()) {
        log::verbose(
            "2D: no CLUT at block {:#x} for the texture at {:#x}: drawn transparent",
            tex0.cbp,
            tex0.tbp0
        );
    }
    return nullptr;
}

TextureHandle TexturePool::convert_upload(const gs::Tex0& tex0, const gs::Texa& texa) {
    // The chip samples textures up to 1024 x 1024 (TW and TH up to 10); a
    // larger claim only arrives in a garbage register write, and converting
    // one would allocate up to gigabytes (a 32768 x 32768 RGBA8 image).
    if (tex0.tw > 10 || tex0.th > 10) {
        log::debug("texture at block {:#x} claims {} x {} texels; not converted", tex0.tbp0,
                   tex0.width(), tex0.height());
        return 0;
    }
    std::uint32_t offset = 0;
    const Stored* stored = upload_at(tex0.tbp0, offset);
    if (stored == nullptr) {
        return 0;
    }
    // The palette's identity is part of the key; its colours are worked out only for a conversion.
    const int entries = palette_entries(tex0.psm);
    const bool indexed = entries != 0;
    Clut now;
    const Clut* clut = indexed ? palette_clut(tex0, now) : nullptr;
    const std::uint64_t clut_hash = clut != nullptr ? clut->hash : 0;
    const Key key{
        tex0.tbp0,
        tex0.tbw,
        tex0.tw,
        tex0.th,
        indexed ? tex0.cbp : 0,
        tex0.psm,
        indexed ? tex0.cpsm : std::uint8_t{0},
        texa,
        stored->hash ^ (static_cast<std::uint64_t>(offset) << 40),
        clut_hash
    };
    if (auto found = m_converted.find(key); found != m_converted.end()) {
        return found->second;
    }
    std::vector<std::uint32_t> colours;
    if (indexed) {
        colours = clut != nullptr
                      ? clut_csm1(clut->data, clut->cpsm, clut->width, entries, texa)
                      : std::vector<std::uint32_t>(static_cast<std::size_t>(entries), 0);
    }
    const std::vector<std::uint8_t> raster = texture_raster(*stored, tex0);
    Rgba8Image image =
        convert(raster, tex0.psm, tex0.width(), tex0.height(), colours, texa, AlphaScale::Gs);
    if (image.pixels.empty()) {
        return 0;  // no texture the chip could sample (convert())
    }
    // OPENRAC_DUMP_TEXTURE=BLOCK (hex): each conversion of a texture at that block is written as
    // a PPM (RGB) and its alpha as a PGM, for checking uploads and palettes.
    // Unset, it names no block, which is never negative.
    constexpr std::int64_t kNoBlock = -1;
    if (env::integer(env::Var::DumpTexture, kNoBlock) == tex0.tbp0) {
        static int n = 0;
        const std::string base = std::format("texture_{:04x}_{}", tex0.tbp0, n++);
        std::FILE* rgb = std::fopen((base + ".ppm").c_str(), "wb");
        std::FILE* a = std::fopen((base + ".pgm").c_str(), "wb");
        if (rgb != nullptr && a != nullptr) {
            std::fprintf(rgb, "P6 %d %d 255\n", image.width, image.height);
            std::fprintf(a, "P5 %d %d 255\n", image.width, image.height);
            for (int y = 0; y < image.height; ++y) {
                for (int x = 0; x < image.width; ++x) {
                    const std::uint32_t t = image.texel(x, y);
                    const unsigned char px[3] = {static_cast<unsigned char>(t), static_cast<unsigned char>(t >> 8),
                                                 static_cast<unsigned char>(t >> 16)};
                    std::fwrite(px, 1, 3, rgb);
                    std::fputc(static_cast<int>(t >> 24), a);
                }
            }
            log::info("texture {:#x} ({}x{}, psm {:#x}, clut {:#x} cpsm {:#x}) written to {}", tex0.tbp0,
                      image.width, image.height, tex0.psm, tex0.cbp, tex0.cpsm, base);
        }
        if (rgb != nullptr) std::fclose(rgb);
        if (a != nullptr) std::fclose(a);
    }
    const TextureHandle h = add(std::move(image), AlphaScale::Gs, "upload");
    m_entries[h - 1].from_upload = true;
    m_entries[h - 1].key = key;
    m_converted.emplace(key, h);
    return h;
}

TextureHandle TexturePool::resolve(const gs::Tex0& tex0, const gs::Texa& texa) {
    TextureHandle h = 0;
    if (auto placed = m_placed.find(tex0.tbp0); placed != m_placed.end()) {
        h = placed->second;
    } else {
        h = convert_upload(tex0, texa);
    }
    if (h == 0) {
        h = m_placeholder;
    }
    m_entries[h - 1].last_used = m_frame;
    return h;
}

const Rgba8Image& TexturePool::image(TextureHandle texture) const {
    return m_entries[texture - 1].image;
}

AlphaScale TexturePool::alpha_scale(TextureHandle texture) const {
    return m_entries[texture - 1].scale;
}

const std::string& TexturePool::name(TextureHandle texture) const {
    return m_entries[texture - 1].name;
}

// ---- GPU textures ----

unsigned TexturePool::gl_texture(TextureHandle texture) {
    Entry& e = m_entries[texture - 1];
    e.last_used = m_frame;

    // The first use of a texture prepare() did not make: it is made now, inside the frame.
    if (e.gl == 0) {
        create_gl(e);
    }

    return e.gl;
}

PreparedTextures TexturePool::prepare(std::span<const TextureHandle> textures) {
    PreparedTextures made;
    std::size_t batch_bytes = 0;

    // Each texture in the order given; one already on the GPU (or given twice) is left alone.
    for (const TextureHandle texture : textures) {
        // 0 is never a texture (texture_pool.h).
        if (texture == 0) {
            continue;
        }

        Entry& e = m_entries[texture - 1];

        // Made already: by a draw, or earlier in this list.
        if (e.gl != 0) {
            continue;
        }

        const std::size_t bytes = texel_bytes(e.image.width, e.image.height);

        // The batch is full: the GPU takes it before more is staged (one larger texture alone
        // is a batch of its own).
        if (batch_bytes > 0 && batch_bytes + bytes > kPrepareBatchBytes) {
            glFinish();
            batch_bytes = 0;
        }

        // A new batch begins with this texture.
        if (batch_bytes == 0) {
            ++made.batches;
        }

        create_gl(e);
        batch_bytes += bytes;
        ++made.textures;
        made.staged_bytes += bytes;
        made.resident_bytes += e.mipmaps ? chain_bytes(e.image.width, e.image.height) : bytes;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return made;
}

void TexturePool::create_gl(Entry& e) {
    glGenTextures(1, &e.gl);
    e.wrap = 0;  // a new texture repeats (set_wrap)
    glBindTexture(GL_TEXTURE_2D, e.gl);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // rows of whole RGBA8 texels, four bytes each
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(GL_RGBA8),
        e.image.width,
        e.image.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        e.image.pixels.data()
    );

    // 3D geometry's: a mip chain, sampled trilinear and repeated (add()).
    if (e.mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR_MIPMAP_LINEAR)
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_REPEAT));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_REPEAT));

        // 1 is no anisotropic filter, which a driver without the extension also has.
        if (m_anisotropy > 1.0f) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, m_anisotropy);
        }
    } else {
        // One level, nearest or bilinear as add() said, until a draw sets its own filter.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        const GLint filter = static_cast<GLint>(e.linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }
}

// The bound texture `e`'s wrap, set only when it changes: a material clamps or repeats its
// texture as the game's GS setup does, and materials share textures.
void TexturePool::set_wrap(Entry& e, bool repeat_s, bool repeat_t) {
    const std::uint8_t wrap = (repeat_s ? 0 : 1) | (repeat_t ? 0 : 2);
    if (e.wrap == wrap) {
        return;
    }
    const auto mode = [](bool repeat) {
        return static_cast<GLint>(repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    };
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode(repeat_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode(repeat_t));
    e.wrap = wrap;
}

// ---- Optional colour reconstruction ----

void TexturePool::set_upscaled_texture_colours(std::unique_ptr<UpscaledTextureColours> source) {
    if (m_upscaled_texture_colours) {
        m_upscaled_texture_colours->release();
    }
    m_upscaled_texture_colours = std::move(source);
}

void TexturePool::bind_upscaled_texture_colours(
    TextureHandle texture, const Shader& shader, TextureFilter filter, bool repeat_s, bool repeat_t
) {
    glActiveTexture(GL_TEXTURE0);
    const unsigned source = gl_texture(texture);
    glBindTexture(GL_TEXTURE_2D, source);
    if (texture != 0 && texture <= m_entries.size()) {
        set_wrap(m_entries[texture - 1], repeat_s, repeat_t);
    }
    if (!m_upscaled_texture_colours) {
        glUniform1i(shader.uniform("upscaled_texture_colours_ready"), 0);
        return;
    }
    const unsigned wrap = (repeat_s ? 1u : 0u) | (repeat_t ? 2u : 0u);
    const auto& image = m_entries[texture - 1].image;
    const unsigned cached =
        filter != TextureFilter::Normal && !m_entries[texture - 1].field
            ? m_upscaled_texture_colours->prepare(source, image.width, image.height, wrap, m_frame)
            : 0;
    // Cache creation uses its compute program. Restore the material before its uniforms.
    shader.use();
    glActiveTexture(GL_TEXTURE0 + kUpscaledTextureColoursUnit);
    glBindTexture(GL_TEXTURE_2D, cached ? cached : source);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source);
    glUniform1i(shader.uniform("upscaled_texture_colours_ready"), cached != 0);
}

float TexturePool::max_anisotropy() {
    // The filter is an extension every desktop driver has (core since OpenGL 4.6); one without it
    // answers the query with an error and leaves the value alone.
    static float most = -1.0f;
    if (most < 0.0f) {
        while (glGetError() != GL_NO_ERROR) {
        }
        GLfloat value = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);
        most = glGetError() == GL_NO_ERROR && value >= 1.0f ? value : 1.0f;
    }
    return most;
}

void TexturePool::set_anisotropy(float samples) {
    samples = std::clamp(samples, 1.0f, max_anisotropy());
    if (samples == m_anisotropy) {
        return;
    }
    m_anisotropy = samples;
    for (const Entry& e : m_entries) {
        if (e.live && e.mipmaps && e.gl != 0) {
            glBindTexture(GL_TEXTURE_2D, e.gl);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, m_anisotropy);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TexturePool::release_gl() {
    if (m_upscaled_texture_colours) {
        m_upscaled_texture_colours->release();
    }
    for (Entry& e : m_entries) {
        if (e.gl != 0) {
            glDeleteTextures(1, &e.gl);
            e.gl = 0;
        }
    }
}

// ---- Texture retirement ----

void TexturePool::end_frame() {
    ++m_frame;
    for (std::size_t i = 0; i < m_entries.size(); ++i) {
        Entry& e = m_entries[i];
        if (!e.live || !(e.from_upload || e.field) || m_frame - e.last_used < kKeepFrames) {
            continue;
        }
        if (e.gl != 0 && gl::loaded()) {
            if (m_upscaled_texture_colours) {
                m_upscaled_texture_colours->forget(e.gl);
            }
            glDeleteTextures(1, &e.gl);
        }
        const auto handle = static_cast<TextureHandle>(i + 1);
        if (e.from_upload) {
            m_converted.erase(e.key);
        }
        if (e.field) {
            // A field nobody drew with for a while: made again if its glyphs come back. Kept for
            // good, every content a text texture ever had took a field four times its size, for
            // the rest of the session.
            if (auto it = m_distance_fields.find(e.field_hash); it != m_distance_fields.end() && it->second == handle) {
                m_distance_fields.erase(it);
            }
            // Its texels rest a while, in case the text comes back (kRestingFields).
            if (m_resting_fields.size() == kRestingFields) {
                m_resting_fields.erase(m_resting_fields.begin());
            }
            m_resting_fields.push_back({e.field_hash, std::move(e.image), std::move(e.name)});
            std::erase_if(m_field_of_handle, [&](const auto& seen) { return seen.second.second == handle; });
        }
        // The handle goes to the next conversion, and the field found by it (distance_field) with
        // it: a glyph page of the same size, whose sparse texels sampled the same, was drawn with
        // this one's field (the HUD's digits as letters, or as nothing).
        m_field_of_handle.erase(handle);
        e = Entry{};
        m_free.push_back(handle);
    }
}

std::size_t TexturePool::size() const {
    return static_cast<std::size_t>(
        std::count_if(m_entries.begin(), m_entries.end(), [](const Entry& e) { return e.live; })
    );
}

}  // namespace openrac::renderer

namespace openrac::renderer {

namespace {

// Runs body(begin, end) over [0, n) in bands, on the threads of common/parallel.h.
void in_bands(int n, const std::function<void(int, int)>& body) {
    constexpr int kBands = 16;
    parallel::for_each(kBands, [&](std::size_t band) {
        const int begin =
            static_cast<int>(static_cast<long long>(n) * static_cast<int>(band) / kBands);
        const int end =
            static_cast<int>(static_cast<long long>(n) * (static_cast<int>(band) + 1) / kBands);
        if (begin < end) {
            body(begin, end);
        }
    });
}

// The squared distance of every texel to the nearest texel where `inside` is set, exact up to
// `reach` texels; a farther one is (reach + 1)^2, only known to be farther. The field needs no more
// (its value is clamped there), and bounding it costs a few operations a texel where the full
// transform (Felzenszwalb and Huttenlocher) took about 20 ms for a 1024 x 512 field. A texel's
// distance is the least dx^2 + dy^2 over the set texels, so the columns' nearest dy^2 first, then
// the least dx^2 + column within reach along each row: whole numbers, the same as the full one.
std::vector<float> distance_2d(const std::vector<std::uint8_t>& inside, int w, int h, int reach) {
    const int far = reach + 1;
    std::vector<int> column(static_cast<std::size_t>(w * h));
    // Columns, a band of them on each thread: down, then up.
    in_bands(w, [&](int x0, int x1) {
        std::vector<int> run(static_cast<std::size_t>(x1 - x0), far);
        for (int y = 0; y < h; ++y) {
            for (int x = x0; x < x1; ++x) {
                const std::size_t i = static_cast<std::size_t>(y * w + x);
                int& r = run[static_cast<std::size_t>(x - x0)];
                r = inside[i] ? 0 : std::min(r + 1, far);
                column[i] = r;
            }
        }
        std::fill(run.begin(), run.end(), far);
        for (int y = h - 1; y >= 0; --y) {
            for (int x = x0; x < x1; ++x) {
                const std::size_t i = static_cast<std::size_t>(y * w + x);
                int& r = run[static_cast<std::size_t>(x - x0)];
                r = inside[i] ? 0 : std::min(r + 1, far);
                const int dy = std::min(column[i], r);
                column[i] = dy * dy;
            }
        }
    });
    // Rows, a band of them on each thread.
    std::vector<float> grid(static_cast<std::size_t>(w * h));
    in_bands(h, [&](int y0, int y1) {
        std::vector<int> gap(static_cast<std::size_t>(w));  // to the nearest column within reach
        for (int y = y0; y < y1; ++y) {
            const int* row = &column[static_cast<std::size_t>(y * w)];
            int last = far;
            for (int x = 0; x < w; ++x) {
                last = row[x] < far * far ? 0 : std::min(last + 1, far);
                gap[static_cast<std::size_t>(x)] = last;
            }
            last = far;
            for (int x = w - 1; x >= 0; --x) {
                last = row[x] < far * far ? 0 : std::min(last + 1, far);
                gap[static_cast<std::size_t>(x)] = std::min(gap[static_cast<std::size_t>(x)], last);
            }
            for (int x = 0; x < w; ++x) {
                int best = row[x];
                if (gap[static_cast<std::size_t>(x)] == far) {
                    grid[static_cast<std::size_t>(y * w + x)] = static_cast<float>(best);
                    continue;  // no set texel within reach
                }
                for (int dx = 1; dx <= reach && dx * dx < best; ++dx) {
                    if (x >= dx) {
                        best = std::min(best, dx * dx + row[x - dx]);
                    }
                    if (x + dx < w) {
                        best = std::min(best, dx * dx + row[x + dx]);
                    }
                }
                grid[static_cast<std::size_t>(y * w + x)] = static_cast<float>(best);
            }
        }
    });
    return grid;
}

}  // namespace

TextureHandle TexturePool::distance_field(TextureHandle source) {
    const Rgba8Image& src = image(source);
    if (src.width <= 0 || src.height <= 0) {
        return source;
    }
    // Looked up first by the handle and a sample of its texels (cheap: this runs for every text
    // draw), then by the source's whole contents: the pool gives the same glyph page a new handle
    // whenever it converts the page again, and a field keyed by handle alone was made again each
    // time, about 15 ms of CPU for a 256 x 128 page (the frame drops while running around Veldin,
    // 30 ms where two pages came back at once). The full field is made only for new contents.
    std::uint64_t sample = static_cast<std::uint64_t>(src.width * 65536 + src.height);
    const std::size_t step = std::max<std::size_t>(4, src.pixels.size() / 256 / 4 * 4);
    for (std::size_t i = 0; i < src.pixels.size(); i += step) {
        std::uint32_t texel_word;
        std::memcpy(&texel_word, &src.pixels[i], 4);
        sample = (sample ^ texel_word) * 1099511628211ull;
    }
    if (auto seen = m_field_of_handle.find(source); seen != m_field_of_handle.end() && seen->second.first == sample) {
        m_entries[seen->second.second - 1].last_used = m_frame;
        return seen->second.second;
    }
    const std::uint64_t shape = static_cast<std::uint64_t>(src.width) << 32 | static_cast<std::uint64_t>(src.height) << 1
                                | (alpha_scale(source) == AlphaScale::Full ? 1u : 0u);
    const std::uint64_t hash = hash_bytes(src.pixels.data(), src.pixels.size()) ^ (shape * 0x9E3779B97F4A7C15ull);
    if (auto it = m_distance_fields.find(hash); it != m_distance_fields.end()) {
        m_field_of_handle[source] = {sample, it->second};
        m_entries[it->second - 1].last_used = m_frame;
        return it->second;
    }
    // Dropped a little while ago: its texels are kept.
    const auto resting = std::find_if(
        m_resting_fields.begin(), m_resting_fields.end(), [hash](const RestingField& field) {
            return field.hash == hash;
        }
    );
    if (resting != m_resting_fields.end()) {
        RestingField field = std::move(*resting);
        m_resting_fields.erase(resting);
        return add_field(std::move(field.image), std::move(field.name), hash, sample, source);
    }
    // Timed apart in a capture: the HUD's frame that makes one is a hitch.
    const stats::Scope timing("distance field");
    constexpr int kScale = 4;
    const int w = src.width * kScale;
    const int h = src.height * kScale;
    const float opaque = alpha_scale(source) == AlphaScale::Full ? 255.0f : 128.0f;
    auto texel = [&](int x, int y, int c) {
        x = std::clamp(x, 0, src.width - 1);
        y = std::clamp(y, 0, src.height - 1);
        return static_cast<float>(src.pixels[static_cast<std::size_t>((y * src.width + x) * 4 + c)]);
    };
    // The glyphs' shape: at least half as opaque as the glyph is nearby (a source texel's 5 x 5
    // neighbourhood), so a font that fades its glyphs downwards in their alpha keeps its lower part
    // and one that does not keeps its weight; below a 32nd of opaque is always outside.
    std::vector<float> local(static_cast<std::size_t>(src.width * src.height));
    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {
            float m = 0.0f;
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    m = std::max(m, texel(x + dx, y + dy, 3));
                }
            }
            local[static_cast<std::size_t>(y * src.width + x)] = m;
        }
    }
    auto local_at = [&](int x, int y) {
        x = std::clamp(x, 0, src.width - 1);
        y = std::clamp(y, 0, src.height - 1);
        return local[static_cast<std::size_t>(y * src.width + x)];
    };
    Rgba8Image out(w, h);
    std::vector<std::uint8_t> inside(static_cast<std::size_t>(w * h)), outside(static_cast<std::size_t>(w * h));
    // Each field column's two source columns and its weight, the same in every row.
    std::vector<int> column0(static_cast<std::size_t>(w));
    std::vector<float> column_weight(static_cast<std::size_t>(w));
    for (int x = 0; x < w; ++x) {
        const float sx = (static_cast<float>(x) + 0.5f) / kScale - 0.5f;
        column0[static_cast<std::size_t>(x)] = static_cast<int>(std::floor(sx));
        column_weight[static_cast<std::size_t>(x)] = sx - std::floor(sx);
    }
    in_bands(h, [&](int band_begin, int band_end) {
        for (int y = band_begin; y < band_end; ++y) {
            const float sy = (static_cast<float>(y) + 0.5f) / kScale - 0.5f;
            const int y0 = static_cast<int>(std::floor(sy));
            const float fy = sy - static_cast<float>(y0);
            for (int x = 0; x < w; ++x) {
                const int x0 = column0[static_cast<std::size_t>(x)];
                const float fx = column_weight[static_cast<std::size_t>(x)];
                // Only the alpha is filtered: the field is made from it alone.
                const float a00 = texel(x0, y0, 3), a10 = texel(x0 + 1, y0, 3);
                const float a01 = texel(x0, y0 + 1, 3), a11 = texel(x0 + 1, y0 + 1, 3);
                const float alpha = (a00 * (1.0f - fx) + a10 * fx) * (1.0f - fy)
                                    + (a01 * (1.0f - fx) + a11 * fx) * fy;
                const std::size_t i = static_cast<std::size_t>(y * w + x);
                // R: the glyph's own opacity here, from the nearest opaque-enough texel of the four (so
                // it holds up to the edge instead of fading with the filter), 0..1 as 0..255.
                const float near_alpha = std::max(std::max(a00, a10), std::max(a01, a11));
                const float near_local =
                    (local_at(x0, y0) * (1.0f - fx) + local_at(x0 + 1, y0) * fx) * (1.0f - fy)
                    + (local_at(x0, y0 + 1) * (1.0f - fx) + local_at(x0 + 1, y0 + 1) * fx) * fy;
                const float edge = std::max(near_local * 0.5f, opaque / 32.0f);
                const float own = alpha >= edge
                                      ? std::max(alpha, std::min(near_alpha, alpha * 2.0f))
                                      : near_alpha;
                out.pixels[i * 4 + 0] =
                    static_cast<std::uint8_t>(std::clamp(own / opaque, 0.0f, 1.0f) * 255.0f + 0.5f);
                out.pixels[i * 4 + 1] = 0;
                out.pixels[i * 4 + 2] = 0;
                inside[i] = alpha >= edge;
                outside[i] = !inside[i];
            }
        }
    });
    const float spread = 2.0f * kScale;  // two source texels either side of the edge
    const std::vector<float> to_inside = distance_2d(inside, w, h, 2 * kScale);
    const std::vector<float> to_outside = distance_2d(outside, w, h, 2 * kScale);
    in_bands(h, [&](int band_begin, int band_end) {
        for (std::size_t i = static_cast<std::size_t>(band_begin * w);
             i < static_cast<std::size_t>(band_end * w);
             ++i) {
            const float signed_distance =
                inside[i] ? std::sqrt(to_outside[i]) : -std::sqrt(to_inside[i]);
            const float value = 0.5f + signed_distance / (2.0f * spread);
            out.pixels[i * 4 + 3] =
                static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
        }
    });
    ++m_fields_made;
    return add_field(std::move(out), name(source) + " (distance field)", hash, sample, source);
}

TextureHandle TexturePool::add_field(
    Rgba8Image image,
    std::string name,
    std::uint64_t hash,
    std::uint64_t sample,
    TextureHandle source
) {
    const TextureHandle handle = add(std::move(image), AlphaScale::Full, std::move(name));
    m_entries[handle - 1].field = true;
    m_entries[handle - 1].field_hash = hash;
    m_distance_fields.emplace(hash, handle);
    m_field_of_handle[source] = {sample, handle};
    return handle;
}

}  // namespace openrac::renderer
