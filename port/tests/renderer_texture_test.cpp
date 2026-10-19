// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Texture conversion and the texture pool, on synthetic textures: every
// format to RGBA8, CLUT order, TEXA alpha, the chip's layout for textures
// uploaded in another format, and the pool's identities and cache. No GL.

#include <cstdio>
#include <set>
#include <vector>

#include "check.h"
#include "renderer/texture.h"
#include "renderer/texture_pool.h"

namespace {

using namespace openrac::renderer;

constexpr gs::Texa kTexa{0x10, false, 0x70};

void direct_colour() {
    // PSMCT32: R, G, B, A bytes.
    const std::vector<std::uint8_t> ct32 = {1, 2, 3, 0x80, 4, 5, 6, 0x40};
    Rgba8Image a = convert_ct32(ct32, 2, 1, AlphaScale::Gs);
    CHECK(a.texel(0, 0) == rgba(1, 2, 3, 0x80));
    CHECK(a.texel(1, 0) == rgba(4, 5, 6, 0x40));
    Rgba8Image full = convert_ct32(ct32, 2, 1, AlphaScale::Full);
    CHECK(full.texel(0, 0) == rgba(1, 2, 3, 0xFF));  // 0x80 doubles to 0x100, clamped
    CHECK(full.texel(1, 0) == rgba(4, 5, 6, 0x80));

    // PSMCT24: three bytes, alpha from TEXA.TA0; with AEM black is clear.
    const std::vector<std::uint8_t> ct24 = {9, 8, 7, 0, 0, 0};
    Rgba8Image b = convert_ct24(ct24, 2, 1, kTexa, AlphaScale::Gs);
    CHECK(b.texel(0, 0) == rgba(9, 8, 7, 0x10));
    CHECK(b.texel(1, 0) == rgba(0, 0, 0, 0x10));
    Rgba8Image c = convert_ct24(ct24, 2, 1, {0x10, true, 0x70}, AlphaScale::Gs);
    CHECK(c.texel(1, 0) == rgba(0, 0, 0, 0));

    // PSMCT16: A1 B5 G5 R5, each channel shifted left 3.
    const std::uint16_t red_opaque = 0x801F;   // A=1, R=31
    const std::uint16_t green_clear = 0x03E0;  // A=0, G=31
    const std::vector<std::uint8_t> ct16 =
        {static_cast<std::uint8_t>(red_opaque & 0xFF),
         static_cast<std::uint8_t>(red_opaque >> 8),
         static_cast<std::uint8_t>(green_clear & 0xFF),
         static_cast<std::uint8_t>(green_clear >> 8),
         0,
         0};
    Rgba8Image d = convert_ct16(ct16, 3, 1, kTexa, AlphaScale::Gs);
    CHECK(d.texel(0, 0) == rgba(0xF8, 0, 0, 0x70));  // TA1
    CHECK(d.texel(1, 0) == rgba(0, 0xF8, 0, 0x10));  // TA0
    CHECK(d.texel(2, 0) == rgba(0, 0, 0, 0x10));
    CHECK(convert_ct16(ct16, 3, 1, {0x10, true, 0x70}, AlphaScale::Gs).texel(2, 0) == 0u);
}

// A 16 x 16 CLUT image in CSM1 order: entry i at csm1_position(i).
std::vector<std::uint8_t> clut256() {
    std::vector<std::uint8_t> image(256 * 4);
    for (std::uint32_t i = 0; i < 256; ++i) {
        const std::uint32_t at = csm1_position(i) * 4;
        image[at] = static_cast<std::uint8_t>(i);
        image[at + 1] = static_cast<std::uint8_t>(255 - i);
        image[at + 2] = 7;
        image[at + 3] = 0x80;
    }
    return image;
}

void palettes() {
    CHECK(csm1_position(0x08) == 0x10);
    CHECK(csm1_position(0x10) == 0x08);
    CHECK(csm1_position(0x18) == 0x18);
    CHECK(csm1_position(0xE7) == 0xE7);
    CHECK(csm1_position(0x2F) == 0x37);

    const std::vector<std::uint32_t> palette = clut_csm1(clut256(), gs::kPsmct32, 16, 256, kTexa);
    for (std::uint32_t i = 0; i < 256; ++i) {
        CHECK(palette[i] == rgba(i, 255 - i, 7, 0x80));
    }
    // 16 entries: an 8 x 2 image in order, here inside a wider upload.
    std::vector<std::uint8_t> wide(16 * 2 * 4);
    for (std::uint32_t i = 0; i < 16; ++i) {
        const std::uint32_t at = ((i / 8) * 16 + i % 8) * 4;
        wide[at] = static_cast<std::uint8_t>(i * 10);
        wide[at + 3] = 0x80;
    }
    const auto small = clut_csm1(wide, gs::kPsmct32, 16, 16, kTexa);
    CHECK(small[9] == rgba(90, 0, 0, 0x80));
    // 16-bit CLUT entries.
    const std::vector<std::uint8_t> clut16 = {0x1F, 0x80, 0xE0, 0x03};
    const auto p16 = clut_csm1(clut16, gs::kPsmct16, 8, 16, kTexa);
    CHECK(p16[0] == rgba(0xF8, 0, 0, 0x70));
    CHECK(p16[1] == rgba(0, 0xF8, 0, 0x10));
}

void indexed() {
    const auto palette = clut_csm1(clut256(), gs::kPsmct32, 16, 256, kTexa);
    const std::vector<std::uint8_t> t8 = {0, 1, 0x08, 0x10, 0xFF, 0x80};
    Rgba8Image a = convert_t8(t8, 3, 2, palette, AlphaScale::Gs);
    CHECK(a.texel(2, 0) == rgba(8, 247, 7, 0x80));
    CHECK(a.texel(0, 1) == rgba(0x10, 0xEF, 7, 0x80));
    CHECK(a.texel(1, 1) == rgba(0xFF, 0, 7, 0x80));

    // PSMT4: the left texel in the low nibble.
    std::vector<std::uint32_t> p16(16);
    for (std::uint32_t i = 0; i < 16; ++i) {
        p16[i] = rgba(i, 0, 0, 0x80);
    }
    const std::vector<std::uint8_t> t4 = {0x21, 0x43};
    Rgba8Image b = convert_t4(t4, 4, 1, p16, AlphaScale::Gs);
    CHECK(b.texel(0, 0) == rgba(1, 0, 0, 0x80));
    CHECK(b.texel(1, 0) == rgba(2, 0, 0, 0x80));
    CHECK(b.texel(3, 0) == rgba(4, 0, 0, 0x80));
    CHECK(palette_entries(gs::kPsmt8h) == 256);
    CHECK(palette_entries(gs::kPsmt4hh) == 16);
    CHECK(palette_entries(gs::kPsmct32) == 0);
    CHECK(image_bytes(gs::kPsmt4, 3, 3) == 5);
    CHECK(image_bytes(gs::kPsmct24, 2, 2) == 12);
}

void layout() {
    using swizzle::nibble_address;
    // Facts of the chip's layout (the GS's public documentation):
    // PSMCT32: a page is 64 x 32, blocks of 8 x 8 numbered 0 1 4 5 16 17 20 21
    // along the top row; a block's first column holds rows 0 and 1.
    CHECK(nibble_address(gs::kPsmct32, 8, 0, 0, 1) == 1 * 512);
    CHECK(nibble_address(gs::kPsmct32, 16, 0, 0, 1) == 4 * 512);
    CHECK(nibble_address(gs::kPsmct32, 0, 8, 0, 1) == 2 * 512);
    CHECK(nibble_address(gs::kPsmct32, 64, 0, 0, 2) == 32 * 512);  // next page
    CHECK(nibble_address(gs::kPsmct32, 0, 32, 0, 1) == 32 * 512);
    CHECK(nibble_address(gs::kPsmct32, 2, 0, 0, 1) == 4 * 8);  // word 4
    CHECK(nibble_address(gs::kPsmct32, 0, 1, 0, 1) == 2 * 8);  // word 2
    // PSMT8: 16 x 16 blocks; rows 2-3 of a column are the odd bytes,
    // shifted four words.
    CHECK(nibble_address(gs::kPsmt8, 0, 2, 0, 2) == 33 * 2);
    CHECK(nibble_address(gs::kPsmt8, 8, 0, 0, 2) == 2 * 2);
    CHECK(nibble_address(gs::kPsmt8, 0, 4, 0, 2) == 96 * 2);
    // Base pointers add whole blocks.
    CHECK(
        nibble_address(gs::kPsmt4, 5, 7, 3, 2) == nibble_address(gs::kPsmt4, 5, 7, 0, 2) + 3 * 512
    );

    // Every format fills a page exactly once.
    struct Page {
        std::uint8_t psm;
        int width, height;
    };

    for (const Page& page :
         {Page{gs::kPsmct32, 64, 32},
          Page{gs::kPsmct16, 64, 64},
          Page{gs::kPsmt8, 128, 64},
          Page{gs::kPsmt4, 128, 128}}) {
        std::set<std::uint64_t> seen;
        const int bits = gs::bits_per_pixel(page.psm) / 4;
        bool inside = true;
        for (int y = 0; y < page.height; ++y) {
            for (int x = 0; x < page.width; ++x) {
                const std::uint64_t a = nibble_address(page.psm, x, y, 0, 2);
                inside = inside && a < 32 * 512;
                seen.insert(a / static_cast<std::uint64_t>(bits));
            }
        }
        CHECK(inside);
        CHECK(seen.size() == static_cast<std::size_t>(page.width * page.height));
    }

    // An 8-bit texture uploaded as 32-bit pixels (half the width and height)
    // reads back unchanged, and a 32-bit rectangle is not its raster.
    const int w = 128;
    const int h = 64;
    std::vector<std::uint8_t> texture(static_cast<std::size_t>(w * h));
    for (std::size_t i = 0; i < texture.size(); ++i) {
        texture[i] = static_cast<std::uint8_t>((i * 7 + i / 128) & 0xFF);
    }
    const auto as32 = reinterpret(texture, gs::kPsmt8, w, h, 2, gs::kPsmct32, w / 2, h / 2, 1);
    CHECK(as32.size() == static_cast<std::size_t>(w * h));
    CHECK(as32 != texture);
    const auto back = reinterpret(as32, gs::kPsmct32, w / 2, h / 2, 1, gs::kPsmt8, w, h, 2);
    CHECK(back == texture);
    // The same for 4-bit through 16-bit.
    std::vector<std::uint8_t> t4(128 * 128 / 2);
    for (std::size_t i = 0; i < t4.size(); ++i) {
        t4[i] = static_cast<std::uint8_t>(i * 13);
    }
    const auto as16 = reinterpret(t4, gs::kPsmt4, 128, 128, 2, gs::kPsmct16, 64, 64, 1);
    CHECK(reinterpret(as16, gs::kPsmct16, 64, 64, 1, gs::kPsmt4, 128, 128, 2) == t4);
    // PSMT8H: indices in the top byte of 32-bit pixels.
    std::vector<std::uint8_t> ct32(4 * 4 * 4, 0);
    ct32[3] = 0xAB;          // pixel (0, 0)
    ct32[4 * 5 + 3] = 0xCD;  // pixel (1, 1)
    const auto t8h = reinterpret(ct32, gs::kPsmct32, 4, 4, 1, gs::kPsmt8h, 4, 4, 1);
    CHECK(t8h.size() == 16);
    CHECK(t8h[0] == 0xAB);
    CHECK(t8h[5] == 0xCD);
    // PSMZ24 is three bytes a pixel in its raster and the low three of a 32-bit slot, as PSMCT24:
    // a 32-bit rectangle read as one is exactly its raster (8 x 16 is 384 bytes; the last pixel
    // once wrote a byte past it), and back again leaves the slots' top bytes empty.
    std::vector<std::uint8_t> z32(8 * 16 * 4);
    for (std::size_t i = 0; i < z32.size(); ++i) {
        z32[i] = static_cast<std::uint8_t>(i % 4 == 3 ? 0 : i * 5 + 1);
    }
    const auto z24 = reinterpret(z32, gs::kPsmz32, 8, 16, 1, gs::kPsmz24, 8, 16, 1);
    CHECK(z24.size() == 8 * 16 * 3);
    bool packed = true;
    for (std::size_t p = 0; p < 8 * 16; ++p) {
        for (std::size_t b = 0; b < 3; ++b) {
            packed = packed && z24[p * 3 + b] == z32[p * 4 + b];
        }
    }
    CHECK(packed);
    CHECK(reinterpret(z24, gs::kPsmz24, 8, 16, 1, gs::kPsmz32, 8, 16, 1) == z32);
}

ImageUpload upload(
    std::uint32_t dbp,
    std::uint8_t psm,
    std::uint32_t w,
    std::uint32_t h,
    std::vector<std::uint8_t> data
) {
    ImageUpload u;
    u.dbp = dbp;
    u.dbw = std::max<std::uint32_t>(1, w / 64);
    u.dpsm = psm;
    u.width = w;
    u.height = h;
    u.data = std::move(data);
    return u;
}

gs::Tex0 tex0(
    std::uint32_t tbp, std::uint8_t psm, std::uint32_t tw, std::uint32_t th, std::uint32_t cbp = 0
) {
    gs::Tex0 t{};
    t.tbp0 = tbp;
    t.tbw = 1;
    t.psm = psm;
    t.tw = tw;
    t.th = th;
    t.tcc = true;
    t.cbp = cbp;
    t.cpsm = gs::kPsmct32;
    return t;
}

void pool() {
    TexturePool textures;
    CHECK(textures.size() == 1);  // the placeholder
    const gs::Texa texa{0, false, 0x80};

    // Nothing there yet: the placeholder.
    CHECK(textures.resolve(tex0(0x100, gs::kPsmct32, 2, 2), texa) == textures.placeholder());

    // A 32-bit texture sent as itself.
    std::vector<std::uint8_t> pixels(4 * 4 * 4);
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = static_cast<std::uint8_t>(i);
    }
    textures.upload(upload(0x100, gs::kPsmct32, 4, 4, pixels));
    const TextureHandle a = textures.resolve(tex0(0x100, gs::kPsmct32, 2, 2), texa);
    CHECK(a != textures.placeholder());
    CHECK(textures.image(a).width == 4 && textures.image(a).height == 4);
    CHECK(textures.image(a).texel(1, 0) == rgba(4, 5, 6, 7));
    CHECK(textures.alpha_scale(a) == AlphaScale::Gs);
    // Asked again, or sent again unchanged: the same conversion.
    CHECK(textures.resolve(tex0(0x100, gs::kPsmct32, 2, 2), texa) == a);
    textures.upload(upload(0x100, gs::kPsmct32, 4, 4, pixels));
    CHECK(textures.resolve(tex0(0x100, gs::kPsmct32, 2, 2), texa) == a);
    // Different pixels at the same base pointer: a new texture.
    pixels[4] = 99;
    textures.upload(upload(0x100, gs::kPsmct32, 4, 4, pixels));
    const TextureHandle b = textures.resolve(tex0(0x100, gs::kPsmct32, 2, 2), texa);
    CHECK(b != a);
    CHECK(textures.image(b).texel(1, 0) == rgba(99, 5, 6, 7));
    // A larger TEX0 than the upload pads with zero.
    const TextureHandle c = textures.resolve(tex0(0x100, gs::kPsmct32, 3, 3), texa);
    CHECK(textures.image(c).width == 8 && textures.image(c).texel(7, 7) == 0u);
    // A TEX0 beyond the chip's 1024-texel limit (a garbage register write) is
    // not converted: it would allocate gigabytes (and once threw bad_alloc).
    CHECK(textures.resolve(tex0(0x100, gs::kPsmct32, 15, 14), texa) == textures.placeholder());
    CHECK(textures.resolve(tex0(0x100, gs::kPsmct32, 11, 2), texa) == textures.placeholder());

    // An 8-bit texture with its CLUT elsewhere.
    textures.upload(upload(0x200, gs::kPsmct32, 16, 16, clut256()));
    textures.upload(
        upload(0x300, gs::kPsmt8, 4, 4, {0, 8, 16, 255, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})
    );
    const TextureHandle d = textures.resolve(tex0(0x300, gs::kPsmt8, 2, 2, 0x200), texa);
    CHECK(textures.image(d).texel(1, 0) == rgba(8, 247, 7, 0x80));
    CHECK(textures.image(d).texel(3, 0) == rgba(255, 0, 7, 0x80));

    // The same 8-bit texture sent as 32-bit pixels (the swizzled case).
    std::vector<std::uint8_t> t8(128 * 64);
    for (std::size_t i = 0; i < t8.size(); ++i) {
        t8[i] = static_cast<std::uint8_t>(i % 251);
    }
    textures.upload(upload(
        0x400,
        gs::kPsmct32,
        64,
        32,
        reinterpret(t8, gs::kPsmt8, 128, 64, 2, gs::kPsmct32, 64, 32, 1)
    ));
    gs::Tex0 swizzled = tex0(0x400, gs::kPsmt8, 7, 6, 0x200);
    swizzled.tbw = 2;
    const TextureHandle e = textures.resolve(swizzled, texa);
    CHECK(textures.image(e).width == 128);
    CHECK(
        textures.image(e).texel(5, 3)
        == rgba((3 * 128 + 5) % 251, 255 - (3 * 128 + 5) % 251, 7, 0x80)
    );

    // A texture converted ahead and placed at a base pointer wins over
    // TEX0's format; a later upload there replaces it.
    Rgba8Image asset(2, 2);
    asset.set_texel(0, 0, rgba(1, 2, 3, 4));
    const TextureHandle f = textures.add(asset, AlphaScale::Full, "asset");
    textures.place(0x500, f);
    CHECK(textures.resolve(tex0(0x500, gs::kPsmt4, 4, 4), texa) == f);
    CHECK(textures.alpha_scale(f) == AlphaScale::Full);
    CHECK(textures.name(f) == "asset");
    textures.upload(upload(0x500, gs::kPsmct32, 4, 4, pixels));
    CHECK(textures.resolve(tex0(0x500, gs::kPsmct32, 2, 2), texa) != f);

    // Conversions nobody uses are dropped after a while; assets stay.
    const std::size_t before = textures.size();
    for (std::uint32_t i = 0; i <= TexturePool::kKeepFrames; ++i) {
        textures.end_frame();
    }
    CHECK(textures.size() < before);
    CHECK(textures.image(f).width == 2);
    CHECK(textures.image(textures.placeholder()).width == 8);
}

// A glyph page's distance field belongs to its contents. A conversion nobody used for a while is
// dropped and its handle given to the next one; a field remembered by that handle (and a sample of
// its texels, which a sparse page leaves mostly empty) was drawn for another page: the HUD's
// digits with a page of letters, or with nothing.
void glyph_fields() {
    TexturePool textures;
    const gs::Texa texa{0, false, 0x80};
    auto page = [](int glyph_x, int glyph_y) {
        std::vector<std::uint8_t> pixels(256 * 128 * 4);
        for (int y = glyph_y; y < glyph_y + 12; ++y) {
            for (int x = glyph_x; x < glyph_x + 6; ++x) {
                pixels[static_cast<std::size_t>((y * 256 + x) * 4 + 3)] = 0x80;
            }
        }
        return pixels;
    };
    gs::Tex0 t = tex0(0x600, gs::kPsmct32, 8, 7);
    t.tbw = 4;
    textures.upload(upload(0x600, gs::kPsmct32, 256, 128, page(10, 10)));
    const TextureHandle digits = textures.resolve(t, texa);
    const TextureHandle digits_field = textures.distance_field(digits);
    CHECK(digits_field != digits);
    for (std::uint32_t i = 0; i <= TexturePool::kKeepFrames; ++i) {
        textures.end_frame();
    }
    textures.upload(upload(0x600, gs::kPsmct32, 256, 128, page(40, 60)));
    const TextureHandle letters = textures.resolve(t, texa);
    CHECK(letters == digits);  // the handle given again, as the pool does
    const TextureHandle letters_field = textures.distance_field(letters);
    CHECK(letters_field != digits_field);
    CHECK(textures.image(letters_field).texel(43 * 4, 66 * 4) >> 24 > 0x80);  // inside the new glyph
}

// Sizes no texture the chip samples can have: an empty image, not an allocation. In 0.8.0 a
// garbage TEX0 (an unknown format, 32768 x 16384 texels) reached convert(), whose blank image for
// an unknown format computed w * h * 4 in int: -2^31, which operator new was asked for as
// 2^64 - 2^31 bytes (std::bad_alloc after 22 minutes on Kerwan, crash report 2026-10-19).
void absurd_sizes() {
    const std::vector<std::uint8_t> none;
    for (const int psm_value : {int{gs::kPsmct32}, int{gs::kPsmct16}, int{gs::kPsmt8}, int{gs::kPsmt4}, 0x3F}) {
        const auto psm = static_cast<std::uint8_t>(psm_value);
        const Rgba8Image huge = convert(none, psm, 32768, 16384, {}, kTexa, AlphaScale::Gs);
        CHECK(huge.width == 0 && huge.height == 0 && huge.pixels.empty());
        CHECK(convert(none, psm, 1025, 4, {}, kTexa, AlphaScale::Gs).pixels.empty());
        CHECK(convert(none, psm, 4, 0, {}, kTexa, AlphaScale::Gs).pixels.empty());
        CHECK(convert(none, psm, -8, 4, {}, kTexa, AlphaScale::Gs).pixels.empty());
        // The chip's largest is still converted.
        CHECK(convert(none, psm, 1024, 1024, {}, kTexa, AlphaScale::Gs).pixels.size() == 1024u * 1024u * 4u);
    }
    const Rgba8Image negative(-1, 5);
    CHECK(negative.width == 0 && negative.pixels.empty());

    // The pool, asked for such a texture in a format it does not know (the crash's case), draws
    // the placeholder.
    TexturePool textures;
    const gs::Texa texa{0, false, 0x80};
    textures.upload(upload(0x100, gs::kPsmct32, 4, 4, std::vector<std::uint8_t>(64, 1)));
    CHECK(textures.resolve(tex0(0x100, 0x3F, 15, 14), texa) == textures.placeholder());
    CHECK(textures.resolve(tex0(0x100, 0x3F, 14, 15), texa) == textures.placeholder());
    CHECK(textures.size() == 1);
}

}  // namespace

// What the 2D layer needs of the pool beyond whole textures at their own base: a texture or CLUT
// inside a larger transfer, and a CLUT read as it was when TEX0 loaded it.
void inside_and_loaded() {
    TexturePool textures;
    const gs::Texa texa{0, false, 0x80};

    // 64 x 64 32-bit pixels at block 0x200, one page wide: the texture at block 0x220 is its second
    // page, lines 32 to 63.
    ImageUpload big{0x200, 1, gs::kPsmct32, 0, 0, 64, 64, std::vector<std::uint8_t>(64 * 64 * 4)};
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            big.data[static_cast<std::size_t>((y * 64 + x) * 4)] = static_cast<std::uint8_t>(y);
            big.data[static_cast<std::size_t>((y * 64 + x) * 4 + 3)] = 0x80;
        }
    }
    textures.upload(big);
    const TextureHandle inner = textures.resolve(tex0(0x220, gs::kPsmct32, 6, 5), texa);
    CHECK(inner != textures.placeholder());
    if (inner != textures.placeholder()) {
        CHECK(textures.image(inner).texel(0, 0) == rgba(32, 0, 0, 0x80));
        CHECK(textures.image(inner).texel(5, 31) == rgba(63, 0, 0, 0x80));
    }

    // An 8-bit texture whose 256-colour CLUT is at block 0x300; TEX0 loads it, then the game puts
    // another CLUT there: the texture keeps the loaded colours.
    ImageUpload indices{0x280, 2, gs::kPsmt8, 0, 0, 2, 1, {1, 2}};
    textures.upload(indices);
    auto clut = [](std::uint8_t red) {
        ImageUpload c{0x300, 1, gs::kPsmct32, 0, 0, 16, 16, std::vector<std::uint8_t>(16 * 16 * 4)};
        for (std::size_t i = 0; i < 256; ++i) {
            c.data[i * 4] = red;
            c.data[i * 4 + 3] = 0x80;
        }
        return c;
    };
    gs::Tex0 t8 = tex0(0x280, gs::kPsmt8, 1, 0, 0x300);
    t8.tbw = 2;
    textures.upload(clut(10));
    textures.load_clut(t8);
    textures.upload(clut(99));
    const TextureHandle loaded = textures.resolve(t8, texa);
    CHECK(textures.image(loaded).texel(0, 0) == rgba(10, 0, 0, 0x80));
    textures.load_clut(t8);  // loaded again: the new one
    CHECK(textures.image(textures.resolve(t8, texa)).texel(0, 0) == rgba(99, 0, 0, 0x80));

    // A 16-colour CLUT sent inside a larger transfer: block 0x341 is the second block of the
    // upload at 0x340, pixels 8 to 15 of its first two rows.
    ImageUpload sheet{0x340, 1, gs::kPsmct32, 0, 0, 64, 32, std::vector<std::uint8_t>(64 * 32 * 4)};
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            sheet.data[static_cast<std::size_t>((y * 64 + x) * 4 + 1)] = static_cast<std::uint8_t>(x);
            sheet.data[static_cast<std::size_t>((y * 64 + x) * 4 + 3)] = 0x80;
        }
    }
    textures.upload(sheet);
    textures.upload(ImageUpload{0x380, 1, gs::kPsmt4, 0, 0, 2, 1, {0x10}});
    const TextureHandle four = textures.resolve(tex0(0x380, gs::kPsmt4, 1, 0, 0x341), texa);
    CHECK(textures.image(four).texel(0, 0) == rgba(0, 8, 0, 0x80));
    CHECK(textures.image(four).texel(1, 0) == rgba(0, 9, 0, 0x80));
}

int main() {
    direct_colour();
    palettes();
    indexed();
    layout();
    pool();
    glyph_fields();
    absurd_sizes();
    inside_and_loaded();
    return openrac::test::result();
}
