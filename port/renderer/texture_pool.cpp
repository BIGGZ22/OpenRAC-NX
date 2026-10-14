// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/texture_pool.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "common/log.h"
#include "renderer/gl.h"

namespace openrac::renderer {

using namespace openrac::gl;

std::uint64_t hash_bytes(const std::uint8_t* data, std::size_t size) {
    std::uint64_t h = 0xcbf29ce484222325ull;
    for (std::size_t i = 0; i < size; ++i) {
        h ^= data[i];
        h *= 0x100000001b3ull;
    }
    return h;
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

TextureHandle TexturePool::add(Rgba8Image image, AlphaScale scale, std::string name, bool mipmaps) {
    const TextureHandle h = new_entry();
    Entry& e = m_entries[h - 1];
    e = Entry{};
    e.live = true;
    e.image = std::move(image);
    e.scale = scale;
    e.name = std::move(name);
    e.mipmaps = mipmaps;
    e.last_used = m_frame;
    return h;
}

void TexturePool::place(std::uint32_t tbp, TextureHandle texture) {
    m_placed[tbp] = texture;
}

void TexturePool::clear_placements() {
    m_placed.clear();
}

void TexturePool::upload(ImageUpload upload) {
    if (upload.x != 0 || upload.y != 0) {
        // Uploads into the middle of a buffer (texture atlases) would need the
        // rectangle placed by its offset; the 2D path seen so far sends whole
        // textures. Kept, and read as if at the origin.
        log::debug("image transfer at offset {},{} in block {:#x}", upload.x, upload.y, upload.dbp);
    }
    Stored stored;
    const std::uint64_t shape[] = {upload.width, upload.height, upload.dpsm, upload.dbw};
    stored.hash = hash_bytes(upload.data.data(), upload.data.size())
                  ^ (hash_bytes(reinterpret_cast<const std::uint8_t*>(shape), sizeof(shape)) << 1);
    stored.upload = std::move(upload);
    const std::uint32_t dbp = stored.upload.dbp;
    m_uploads[dbp] = std::move(stored);
    // An upload replaces a texture placed at the same base pointer.
    m_placed.erase(dbp);
}

std::vector<std::uint8_t> TexturePool::texture_raster(const Stored& stored, const gs::Tex0& tex0)
    const {
    const ImageUpload& up = stored.upload;
    const int tw = tex0.width();
    const int th = tex0.height();
    const int up_w = static_cast<int>(up.width);
    const int up_h = static_cast<int>(up.height);
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

std::vector<std::uint32_t> TexturePool::palette(
    const gs::Tex0& tex0, const gs::Texa& texa, std::uint64_t& hash
) {
    const int entries = palette_entries(tex0.psm);
    hash = 0;
    if (entries == 0) {
        return {};
    }
    auto it = m_uploads.find(tex0.cbp);
    if (it == m_uploads.end()) {
        log::debug("no CLUT at block {:#x}", tex0.cbp);
        return std::vector<std::uint32_t>(static_cast<std::size_t>(entries), 0);
    }
    if (tex0.csm) {
        // CSM2 (a CLUT as one row of a larger image, placed by TEXCLUT) is
        // not used by the games read so far.
        log::debug("CSM2 CLUT at block {:#x} read as CSM1", tex0.cbp);
    }
    hash = it->second.hash;
    const ImageUpload& up = it->second.upload;
    return clut_csm1(up.data, up.dpsm, std::max<int>(static_cast<int>(up.width), 1), entries, texa);
}

TextureHandle TexturePool::convert_upload(const gs::Tex0& tex0, const gs::Texa& texa) {
    auto it = m_uploads.find(tex0.tbp0);
    if (it == m_uploads.end()) {
        return 0;
    }
    std::uint64_t clut_hash = 0;
    std::vector<std::uint32_t> colours;
    const bool indexed = palette_entries(tex0.psm) != 0;
    if (indexed) {
        colours = palette(tex0, texa, clut_hash);
    }
    const Key key{
        tex0.tbp0,
        tex0.tbw,
        tex0.tw,
        tex0.th,
        indexed ? tex0.cbp : 0,
        tex0.psm,
        indexed ? tex0.cpsm : std::uint8_t{0},
        texa,
        it->second.hash,
        clut_hash
    };
    if (auto found = m_converted.find(key); found != m_converted.end()) {
        return found->second;
    }
    const std::vector<std::uint8_t> raster = texture_raster(it->second, tex0);
    Rgba8Image image =
        convert(raster, tex0.psm, tex0.width(), tex0.height(), colours, texa, AlphaScale::Gs);
    // OPENRAC_DUMP_TEXTURE=BLOCK (hex): each conversion of a texture at that block is written as
    // a PPM (RGB) and its alpha as a PGM, for checking uploads and palettes.
    static const char* dump = std::getenv("OPENRAC_DUMP_TEXTURE");
    if (dump != nullptr && std::strtoul(dump, nullptr, 16) == tex0.tbp0) {
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

unsigned TexturePool::gl_texture(TextureHandle texture) {
    Entry& e = m_entries[texture - 1];
    e.last_used = m_frame;
    if (e.gl == 0) {
        glGenTextures(1, &e.gl);
        glBindTexture(GL_TEXTURE_2D, e.gl);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
        if (e.mipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR_MIPMAP_LINEAR)
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_REPEAT));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_REPEAT));
            if (m_anisotropy > 1.0f) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, m_anisotropy);
            }
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_NEAREST));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_NEAREST));
        }
    }
    return e.gl;
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
    for (Entry& e : m_entries) {
        if (e.gl != 0) {
            glDeleteTextures(1, &e.gl);
            e.gl = 0;
        }
    }
}

void TexturePool::end_frame() {
    ++m_frame;
    for (std::size_t i = 0; i < m_entries.size(); ++i) {
        Entry& e = m_entries[i];
        if (!e.live || !e.from_upload || m_frame - e.last_used < kKeepFrames) {
            continue;
        }
        if (e.gl != 0 && gl::loaded()) {
            glDeleteTextures(1, &e.gl);
        }
        m_converted.erase(e.key);
        e = Entry{};
        m_free.push_back(static_cast<TextureHandle>(i + 1));
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

// Squared Euclidean distance transform of one row or column (Felzenszwalb and Huttenlocher):
// f in, d out, both n long; f is 0 at feature texels and a large value elsewhere.
void distance_1d(const float* f, float* d, int n, int* v, float* z) {
    int k = 0;
    v[0] = 0;
    z[0] = -1e20f;
    z[1] = 1e20f;
    for (int q = 1; q < n; ++q) {
        float s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k])))
                  / static_cast<float>(2 * q - 2 * v[k]);
        while (s <= z[k]) {
            --k;
            s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k])))
                / static_cast<float>(2 * q - 2 * v[k]);
        }
        ++k;
        v[k] = q;
        z[k] = s;
        z[k + 1] = 1e20f;
    }
    k = 0;
    for (int q = 0; q < n; ++q) {
        while (z[k + 1] < static_cast<float>(q)) {
            ++k;
        }
        const float dq = static_cast<float>(q - v[k]);
        d[q] = dq * dq + f[v[k]];
    }
}

// The squared distance of every texel to the nearest texel where `inside` is set.
std::vector<float> distance_2d(const std::vector<std::uint8_t>& inside, int w, int h) {
    const int n = std::max(w, h);
    std::vector<float> grid(static_cast<std::size_t>(w * h));
    for (std::size_t i = 0; i < grid.size(); ++i) {
        grid[i] = inside[i] ? 0.0f : 1e20f;
    }
    std::vector<float> f(static_cast<std::size_t>(n)), d(static_cast<std::size_t>(n)), z(static_cast<std::size_t>(n + 1));
    std::vector<int> v(static_cast<std::size_t>(n));
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            f[static_cast<std::size_t>(y)] = grid[static_cast<std::size_t>(y * w + x)];
        }
        distance_1d(f.data(), d.data(), h, v.data(), z.data());
        for (int y = 0; y < h; ++y) {
            grid[static_cast<std::size_t>(y * w + x)] = d[static_cast<std::size_t>(y)];
        }
    }
    for (int y = 0; y < h; ++y) {
        distance_1d(&grid[static_cast<std::size_t>(y * w)], d.data(), w, v.data(), z.data());
        std::copy(d.begin(), d.begin() + w, grid.begin() + y * w);
    }
    return grid;
}

}  // namespace

TextureHandle TexturePool::distance_field(TextureHandle source) {
    const Rgba8Image& src = image(source);
    if (src.width <= 0 || src.height <= 0) {
        return source;
    }
    // Looked up by the source and a sample of its texels (a handle can be given again to another
    // texture once its own is dropped); the full field is made only when that misses.
    std::uint64_t hash = (static_cast<std::uint64_t>(source) << 32) ^ static_cast<std::uint64_t>(src.width * 65536 + src.height);
    const std::size_t step = std::max<std::size_t>(4, src.pixels.size() / 256 / 4 * 4);
    for (std::size_t i = 0; i < src.pixels.size(); i += step) {
        std::uint32_t texel_word;
        std::memcpy(&texel_word, &src.pixels[i], 4);
        hash = (hash ^ texel_word) * 1099511628211ull;
    }
    if (auto it = m_distance_fields.find(hash); it != m_distance_fields.end()) {
        return it->second;
    }
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
    for (int y = 0; y < h; ++y) {
        const float sy = (static_cast<float>(y) + 0.5f) / kScale - 0.5f;
        const int y0 = static_cast<int>(std::floor(sy));
        const float fy = sy - static_cast<float>(y0);
        for (int x = 0; x < w; ++x) {
            const float sx = (static_cast<float>(x) + 0.5f) / kScale - 0.5f;
            const int x0 = static_cast<int>(std::floor(sx));
            const float fx = sx - static_cast<float>(x0);
            float c[4];
            for (int k = 0; k < 4; ++k) {
                const float top = texel(x0, y0, k) * (1.0f - fx) + texel(x0 + 1, y0, k) * fx;
                const float bottom = texel(x0, y0 + 1, k) * (1.0f - fx) + texel(x0 + 1, y0 + 1, k) * fx;
                c[k] = top * (1.0f - fy) + bottom * fy;
            }
            const std::size_t i = static_cast<std::size_t>(y * w + x);
            // R: the glyph's own opacity here, from the nearest opaque-enough texel of the four (so
            // it holds up to the edge instead of fading with the filter), 0..1 as 0..255.
            float near_alpha = 0.0f;
            for (int k = 0; k < 4; ++k) {
                const float a = texel(x0 + (k & 1), y0 + (k >> 1), 3);
                near_alpha = std::max(near_alpha, a);
            }
            const float near_local = (local_at(x0, y0) * (1.0f - fx) + local_at(x0 + 1, y0) * fx) * (1.0f - fy)
                                     + (local_at(x0, y0 + 1) * (1.0f - fx) + local_at(x0 + 1, y0 + 1) * fx) * fy;
            const float edge = std::max(near_local * 0.5f, opaque / 32.0f);
            const float own = c[3] >= edge ? std::max(c[3], std::min(near_alpha, c[3] * 2.0f)) : near_alpha;
            out.pixels[i * 4 + 0] = static_cast<std::uint8_t>(std::clamp(own / opaque, 0.0f, 1.0f) * 255.0f + 0.5f);
            out.pixels[i * 4 + 1] = 0;
            out.pixels[i * 4 + 2] = 0;
            inside[i] = c[3] >= edge;
            outside[i] = !inside[i];
        }
    }
    const std::vector<float> to_inside = distance_2d(inside, w, h);
    const std::vector<float> to_outside = distance_2d(outside, w, h);
    const float spread = 2.0f * kScale;  // two source texels either side of the edge
    for (std::size_t i = 0; i < inside.size(); ++i) {
        const float signed_distance = inside[i] ? std::sqrt(to_outside[i]) : -std::sqrt(to_inside[i]);
        const float value = 0.5f + signed_distance / (2.0f * spread);
        out.pixels[i * 4 + 3] = static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
    }
    const TextureHandle handle = add(std::move(out), AlphaScale::Full, name(source) + " (distance field)");
    m_distance_fields.emplace(hash, handle);
    return handle;
}

}  // namespace openrac::renderer
