// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The direct renderer's CPU half: GIF packets in, triangles and draw groups
// out. Synthetic packets, laid out the way the game's 2D code builds them.

#include <cmath>
#include <cstdio>

#include "check.h"
#include "renderer/direct.h"
#include "renderer_gif_builder.h"

namespace {

using namespace openrac::renderer;
using openrac::test::GifBuilder;
using openrac::test::prim;
using openrac::test::rgbaq;
using openrac::test::xyz2;

bool near(float a, float b) {
    return std::fabs(a - b) < 1e-4f;
}

void sprite() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    b
        .ad({{gs::kPrim, prim(gs::PrimKind::Sprite, false, false, false)},
             {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
             {gs::kXyz2, xyz2(0, 0, 100)},
             {gs::kXyz2, xyz2(256, 224, 100)}},
            true);
    CHECK(gif.gif(b.bytes));
    CHECK(gif.vertices().size() == 6);
    CHECK(gif.draws().size() == 1);
    // At their own positions: the shader moves them on by a fraction of the target's pixel (the
    // chip samples pixels at their integer coordinates, GL at their centres).
    const DirectVertex& first = gif.vertices()[0];
    CHECK(near(first.x, -1.0f) && near(first.y, 1.0f));  // the top left corner
    const DirectVertex& last = gif.vertices()[4];
    CHECK(near(last.x, 0.0f) && near(last.y, 0.0f));  // the centre of the screen
    CHECK(first.rgba[0] == 0xFF && first.rgba[3] == 0x80);
    // Depth: 100 of a 24-bit buffer.
    CHECK(near(first.z, static_cast<float>(100.0 / 16777215.0 * 2.0 - 1.0)));
}

void strips_and_kicks() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    // A Gouraud strip of four vertices: two triangles.
    b.ad(
        {{gs::kPrim, prim(gs::PrimKind::TriangleStrip, true, false, false)},
         {gs::kRgbaq, rgbaq(10, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kRgbaq, rgbaq(20, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(10, 0, 0)},
         {gs::kRgbaq, rgbaq(30, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(0, 10, 0)},
         {gs::kRgbaq, rgbaq(40, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(10, 10, 0)}}
    );
    CHECK(gif.gif(b.bytes));
    CHECK(gif.vertices().size() == 6);
    CHECK(gif.vertices()[0].rgba[0] == 10);
    CHECK(gif.vertices()[5].rgba[0] == 40);
    gif.clear();

    // XYZ3 moves the strip along without drawing; flat shading takes the
    // last vertex's colour.
    GifBuilder c;
    c.ad(
        {{gs::kPrim, prim(gs::PrimKind::TriangleStrip, false, false, false)},
         {gs::kRgbaq, rgbaq(1, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kXyz2, xyz2(10, 0, 0)},
         {gs::kXyz3, xyz2(0, 10, 0)},
         {gs::kRgbaq, rgbaq(9, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(10, 10, 0)}}
    );
    CHECK(gif.gif(c.bytes));
    CHECK(gif.vertices().size() == 3);
    CHECK(gif.vertices()[0].rgba[0] == 9);
    gif.clear();

    // A fan of five vertices: three triangles around the first.
    GifBuilder d;
    d.ad(
        {{gs::kPrim, prim(gs::PrimKind::TriangleFan, true, false, false)},
         {gs::kXyz2, xyz2(50, 50, 0)},
         {gs::kXyz2, xyz2(60, 50, 0)},
         {gs::kXyz2, xyz2(60, 60, 0)},
         {gs::kXyz2, xyz2(50, 60, 0)},
         {gs::kXyz2, xyz2(40, 60, 0)}}
    );
    CHECK(gif.gif(d.bytes));
    CHECK(gif.vertices().size() == 9);
    CHECK(near(gif.vertices()[6].x, gif.vertices()[0].x));  // every triangle starts at the pivot
    gif.clear();

    // A PRIM write in the middle drops the vertices queued before it.
    GifBuilder e;
    e.ad(
        {{gs::kPrim, prim(gs::PrimKind::Triangle, true, false, false)},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kXyz2, xyz2(1, 0, 0)},
         {gs::kPrim, prim(gs::PrimKind::Triangle, true, false, false)},
         {gs::kXyz2, xyz2(0, 1, 0)}}
    );
    CHECK(gif.gif(e.bytes));
    CHECK(gif.vertices().empty());
    // Lines and points become quads.
    GifBuilder f;
    f.ad(
        {{gs::kPrim, prim(gs::PrimKind::Line, true, false, false)},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kXyz2, xyz2(100, 0, 0)},
         {gs::kPrim, prim(gs::PrimKind::Point, true, false, false)},
         {gs::kXyz2, xyz2(5, 5, 0)}}
    );
    CHECK(gif.gif(f.bytes));
    CHECK(gif.vertices().size() == 12);
}

void packed_and_reglist() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    // PACKED with PRE: PRIM from the tag; RGBAQ, then XYZ2 twice per loop.
    const std::uint64_t regs = gs::kPackedRgbaq | (gs::kPackedXyz2 << 4);
    b.tag(
        2,
        false,
        gs::GifTag::kPacked,
        2,
        regs,
        true,
        static_cast<std::uint32_t>(prim(gs::PrimKind::Sprite, false, false, false))
    );
    b.qword(0x11 | (std::uint64_t{0x22} << 32), 0x33 | (std::uint64_t{0x80} << 32));
    b.qword(
        static_cast<std::uint64_t>((2048 - 256) * 16)
            | (static_cast<std::uint64_t>((2048 - 224) * 16) << 32),
        5
    );
    b.qword(0x11 | (std::uint64_t{0x22} << 32), 0x33 | (std::uint64_t{0x80} << 32));
    b.qword(
        static_cast<std::uint64_t>((2048 - 256 + 8) * 16)
            | (static_cast<std::uint64_t>((2048 - 224 + 8) * 16) << 32),
        5
    );
    CHECK(gif.gif(b.bytes));
    CHECK(gif.vertices().size() == 6);
    CHECK(
        gif.vertices()[0].rgba[0] == 0x11 && gif.vertices()[0].rgba[1] == 0x22
        && gif.vertices()[0].rgba[2] == 0x33
    );
    gif.clear();

    // REGLIST: 64-bit register values, padded to a quadword.
    GifBuilder r;
    r.tag(1, true, gs::GifTag::kReglist, 3, gs::kPrim | (gs::kXyz2 << 4) | (gs::kXyz2 << 8));
    std::uint64_t words[4] =
        {prim(gs::PrimKind::Sprite, false, false, false), xyz2(0, 0, 0), xyz2(4, 4, 0), 0};
    r.qword(words[0], words[1]);
    r.qword(words[2], words[3]);
    CHECK(gif.gif(r.bytes));
    CHECK(gif.vertices().size() == 6);

    // A register payload can span raw GIF transfers. Nothing is drawn until
    // the rest arrives, and then the sprite is emitted exactly once.
    gif.clear();
    GifBuilder first;
    first.tag(3, true, gs::GifTag::kPacked, 1, gs::kPackedAd);
    first.qword(prim(gs::PrimKind::Sprite, false, false, false), gs::kPrim);
    CHECK(gif.gif(first.bytes));
    CHECK(gif.vertices().empty());
    CHECK(gif.error().empty());
    GifBuilder rest;
    rest.qword(xyz2(0, 0, 0), gs::kXyz2);
    rest.qword(xyz2(4, 4, 0), gs::kXyz2);
    CHECK(gif.gif(rest.bytes));
    CHECK(gif.vertices().size() == 6);
    CHECK(gif.error().empty());

    // A transfer ending inside the tag itself is still malformed.
    gif.clear();
    first.bytes.resize(8);
    CHECK(!gif.gif(first.bytes));
    CHECK(!gif.error().empty());
}

void draw_groups() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    const std::uint64_t sprite = prim(gs::PrimKind::Sprite, false, false, false);
    b.ad(
        {{gs::kPrim, sprite},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kXyz2, xyz2(4, 4, 0)},
         // Same state: merged into the first draw.
         {gs::kPrim, sprite},
         {gs::kXyz2, xyz2(8, 0, 0)},
         {gs::kXyz2, xyz2(12, 4, 0)},
         // Blending on: a new draw.
         {gs::kAlpha1, openrac::test::alpha_reg(0, 1, 0, 1)},
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, true)},
         {gs::kXyz2, xyz2(0, 8, 0)},
         {gs::kXyz2, xyz2(4, 12, 0)},
         // A new scissor: another.
         {gs::kScissor1, openrac::test::scissor(0, 100, 0, 100)},
         {gs::kXyz2, xyz2(0, 16, 0)},
         {gs::kXyz2, xyz2(4, 20, 0)},
         // Context 2 has its own (default) registers.
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, false, false, true)},
         {gs::kXyz2, xyz2(0, 24, 0)},
         {gs::kXyz2, xyz2(4, 28, 0)}}
    );
    CHECK(gif.gif(b.bytes));
    CHECK(gif.draws().size() == 4);
    CHECK(gif.draws()[0].count == 12);
    CHECK(gif.draws()[1].state.blend);
    CHECK(gif.draws()[2].state.scissor.x1 == 100);
    CHECK(gif.draws()[3].state.scissor.x1 == 511);
    CHECK(!gif.draws()[3].state.blend);
}

void textures_and_vif() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    std::vector<std::uint8_t> pixels(4 * 4 * 4, 0x40);
    b.image(0x800, 1, gs::kPsmct32, 4, 4, pixels);
    b.ad(
        {{gs::kTex0_1, openrac::test::tex0(0x800, 1, gs::kPsmct32, 2, 2, true)},
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
         {gs::kUv, openrac::test::uv(0, 0)},
         {gs::kXyz2, xyz2(0, 0, 0)},
         {gs::kUv, openrac::test::uv(2, 4)},
         {gs::kXyz2, xyz2(8, 8, 0)}}
    );

    // The same packet inside a VIF1 stream, as the display list carries it:
    // NOPs to align, then DIRECT with the size in quadwords.
    std::vector<std::uint8_t> vif(12, 0);
    const std::uint32_t direct = 0x50000000u | static_cast<std::uint32_t>(b.bytes.size() / 16);
    vif.push_back(static_cast<std::uint8_t>(direct));
    vif.push_back(static_cast<std::uint8_t>(direct >> 8));
    vif.push_back(static_cast<std::uint8_t>(direct >> 16));
    vif.push_back(static_cast<std::uint8_t>(direct >> 24));
    vif.insert(vif.end(), b.bytes.begin(), b.bytes.end());
    CHECK(gif.vif(vif));
    CHECK(gif.draws().size() == 1);
    const DirectState& s = gif.draws()[0].state;
    CHECK(s.textured && s.tcc);
    CHECK(s.texture != textures.placeholder());
    CHECK(textures.image(s.texture).texel(0, 0) == rgba(0x40, 0x40, 0x40, 0x40));
    // UV in texels over the texture's size: 2 of 4 across, 4 of 4 down.
    const DirectVertex& corner = gif.vertices()[4];
    CHECK(near(corner.s, 0.5f) && near(corner.t, 1.0f) && near(corner.q, 1.0f));

    // A VU program's command is not the direct path.
    std::vector<std::uint8_t> mscal = {0, 0, 0, 0x14};
    CHECK(!gif.vif(mscal));
}

// A block the game drew into off-screen, then uploaded a texture to (its 2D pages reuse that
// memory): a draw of the texture reads the upload, not the old target.
void offscreen_then_upload() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    constexpr std::uint32_t kBlock = 0x1000;
    const std::uint64_t offscreen = std::uint64_t{kBlock / 32} | (std::uint64_t{4} << 16);
    const std::uint64_t frame = std::uint64_t{8} << 16;  // the display buffer, 512 pixels wide
    b.ad({{gs::kFrame1, offscreen},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, false)},
          {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kXyz2, xyz2(16, 16, 0)},
          {gs::kFrame1, frame}});
    std::vector<std::uint8_t> texels(4 * 4 * 4, 0x40);
    b.image(kBlock, 1, gs::kPsmct32, 4, 4, texels);
    b.ad({{gs::kTex0_1, openrac::test::tex0(kBlock, 1, gs::kPsmct32, 2, 2, true)},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
          {gs::kUv, openrac::test::uv(0, 0)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kUv, openrac::test::uv(4, 4)},
          {gs::kXyz2, xyz2(8, 8, 0)}});
    CHECK(gif.gif(b.bytes));
    CHECK(!gif.draws().empty());
    if (!gif.draws().empty()) {
        const DirectState& s = gif.draws().back().state;
        CHECK(s.source == 0);
        CHECK(s.textured && s.texture != textures.placeholder());
    }
}

// The game names a block in FRAME (its register sets name a panel's target whether or not that
// panel is drawn) where a texture of its already is: with nothing drawn there, the block is still
// that texture.
void frame_named_without_draw() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    constexpr std::uint32_t kBlock = 0x2000;
    std::vector<std::uint8_t> texels(4 * 4 * 4, 0x40);
    b.image(kBlock, 1, gs::kPsmct32, 4, 4, texels);
    b.ad({{gs::kFrame1, std::uint64_t{kBlock / 32} | (std::uint64_t{4} << 16)},
          {gs::kFrame1, std::uint64_t{8} << 16},
          {gs::kTex0_1, openrac::test::tex0(kBlock, 1, gs::kPsmct32, 2, 2, true)},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
          {gs::kUv, openrac::test::uv(0, 0)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kUv, openrac::test::uv(4, 4)},
          {gs::kXyz2, xyz2(8, 8, 0)}});
    CHECK(gif.gif(b.bytes));
    CHECK(!gif.draws().empty());
    if (!gif.draws().empty()) {
        const DirectState& s = gif.draws().back().state;
        CHECK(s.source == 0);
        CHECK(s.textured && s.texture != textures.placeholder());
    }
}

// An upload that starts below an off-screen target and runs over it: what was drawn there is gone.
void upload_over_offscreen() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    constexpr std::uint32_t kBase = 0x2000;
    constexpr std::uint32_t kTarget = kBase + 64;  // two pages in: line 64 of a 256-pixel buffer
    b.ad({{gs::kFrame1, std::uint64_t{kTarget / 32} | (std::uint64_t{4} << 16)},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, false)},
          {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kXyz2, xyz2(16, 16, 0)},
          {gs::kFrame1, std::uint64_t{8} << 16}});
    std::vector<std::uint8_t> texels(256 * 128 * 4, 0x40);
    b.image(kBase, 4, gs::kPsmct32, 256, 128, texels);
    b.ad({{gs::kTex0_1, openrac::test::tex0(kTarget, 4, gs::kPsmct32, 4, 4, true)},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
          {gs::kUv, openrac::test::uv(0, 0)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kUv, openrac::test::uv(16, 16)},
          {gs::kXyz2, xyz2(8, 8, 0)}});
    CHECK(gif.gif(b.bytes));
    if (!gif.draws().empty()) {
        CHECK(gif.draws().back().state.source == 0);
    }
}

// A texture in the pages under the 448-line draw buffer (line 448 on) is a texture, not the frame.
void texture_under_draw_buffer() {
    TexturePool textures;
    GifInterpreter gif(textures);
    GifBuilder b;
    set_draw_buffer_block(0x1000, 448);
    const std::uint32_t block = 0x1000 + 14 * 8 * 32;  // page row 14: line 448
    std::vector<std::uint8_t> texels(4 * 4 * 4, 0x40);
    b.ad({{gs::kFrame1, std::uint64_t{0x1000 / 32} | (std::uint64_t{8} << 16)}});
    b.image(block, 1, gs::kPsmct32, 4, 4, texels);
    b.ad({{gs::kTex0_1, openrac::test::tex0(block, 1, gs::kPsmct32, 2, 2, true)},
          {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
          {gs::kUv, openrac::test::uv(0, 0)},
          {gs::kXyz2, xyz2(0, 0, 0)},
          {gs::kUv, openrac::test::uv(4, 4)},
          {gs::kXyz2, xyz2(8, 8, 0)}});
    CHECK(gif.gif(b.bytes));
    CHECK(!gif.draws().empty());
    if (!gif.draws().empty()) {
        const DirectState& s = gif.draws().back().state;
        CHECK(!s.frame_source);
        CHECK(s.textured && s.texture != textures.placeholder());
    }
}

}  // namespace

int main() {
    sprite();
    strips_and_kicks();
    packed_and_reglist();
    draw_groups();
    textures_and_vif();
    offscreen_then_upload();
    frame_named_without_draw();
    upload_over_offscreen();
    texture_under_draw_buffer();
    return openrac::test::result();
}
