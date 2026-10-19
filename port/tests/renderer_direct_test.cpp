// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The direct renderer on a real GPU context (headless: SDL's offscreen
// driver, Mesa's llvmpipe in CI): a few GIF packets drawn into a 512 x 448
// framebuffer through the Renderer, then pixels read back. Skipped (exit 77)
// where no OpenGL 4.1 context can be made.

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "check.h"
#include "renderer/direct.h"
#include "renderer/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer_gif_builder.h"
#include "renderer_gl_context.h"

namespace {

using namespace openrac::renderer;
using openrac::test::GifBuilder;
using openrac::test::prim;
using openrac::test::rgbaq;
using openrac::test::xyz2;

constexpr int kWidth = 512;
constexpr int kHeight = 448;

struct Pixels {
    std::vector<std::uint8_t> rgba;

    std::uint32_t at(int x, int y) const {
        const auto i = static_cast<std::size_t>((y * kWidth + x) * 4);
        return rgba[i] | (rgba[i + 1] << 8) | (rgba[i + 2] << 16);
    }
};

bool close(
    std::uint32_t got, std::uint32_t r, std::uint32_t g, std::uint32_t b, int where_x, int where_y
) {
    auto near = [](std::uint32_t a, std::uint32_t e) {
        return (a > e ? a - e : e - a) <= 3;
    };
    const bool ok =
        near(got & 0xFF, r) && near((got >> 8) & 0xFF, g) && near((got >> 16) & 0xFF, b);
    if (!ok) {
        std::fprintf(
            stderr,
            "pixel (%d, %d) is %06x, expected %02x%02x%02x (BGR order)\n",
            where_x,
            where_y,
            got,
            b,
            g,
            r
        );
    }
    return ok;
}

std::vector<std::uint8_t> packets() {
    GifBuilder p;
    const std::uint64_t sprite = prim(gs::PrimKind::Sprite, false, false, false);
    // The draw environment: depth on, always passing.
    p.ad(
        {{gs::kTest1,
          openrac::test::test_reg(
              false, gs::AlphaTest::Always, 0, gs::AlphaFail::Keep, gs::DepthTest::Always
          )},
         {gs::kZbuf1, std::uint64_t{1} << 24},
         {gs::kScissor1, openrac::test::scissor(0, 511, 0, 447)}}
    );

    // 1. A red sprite, 10..49 square.
    p.ad(
        {{gs::kPrim, sprite},
         {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
         {gs::kXyz2, xyz2(10, 10, 0)},
         {gs::kXyz2, xyz2(50, 50, 0)}}
    );
    // 2. Blue at alpha 0x40 (half) blended over its right half: (Cs - Cd) * As + Cd.
    p.ad(
        {{gs::kAlpha1, openrac::test::alpha_reg(0, 1, 0, 1)},
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, true)},
         {gs::kRgbaq, rgbaq(0, 0, 0xFF, 0x40)},
         {gs::kXyz2, xyz2(30, 10, 0)},
         {gs::kXyz2, xyz2(70, 50, 0)}}
    );

    // 3. A 4 x 4 32-bit texture sent by IMAGE transfer, drawn 40 x 40 with UV.
    std::vector<std::uint8_t> texels;
    for (int i = 0; i < 16; ++i) {
        texels.insert(
            texels.end(),
            {static_cast<std::uint8_t>(i * 16), static_cast<std::uint8_t>(255 - i * 16), 0x30, 0x80}
        );
    }
    p.image(0x1000, 1, gs::kPsmct32, 4, 4, texels);
    p.ad(
        {{gs::kTex0_1, openrac::test::tex0(0x1000, 1, gs::kPsmct32, 2, 2, true)},
         {gs::kTex1_1, 0},
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
         {gs::kRgbaq, rgbaq(0x80, 0x80, 0x80, 0x80)},
         {gs::kUv, openrac::test::uv(0, 0)},
         {gs::kXyz2, xyz2(100, 100, 0)},
         {gs::kUv, openrac::test::uv(4, 4)},
         {gs::kXyz2, xyz2(140, 140, 0)}}
    );

    // 4. An 8-bit texture with a 256-entry CLUT in CSM1 order.
    std::vector<std::uint8_t> clut(256 * 4);
    for (std::uint32_t i = 0; i < 256; ++i) {
        const std::uint32_t at = csm1_position(i) * 4;
        clut[at] = static_cast<std::uint8_t>(i);
        clut[at + 1] = 0x20;
        clut[at + 2] = static_cast<std::uint8_t>(255 - i);
        clut[at + 3] = 0x80;
    }
    p.image(0x2000, 1, gs::kPsmct32, 16, 16, clut);
    p.image(0x2100, 1, gs::kPsmt8, 2, 2, {0x08, 0x10, 0x00, 0xFF});
    p.ad(
        {{gs::kTex0_1, openrac::test::tex0(0x2100, 1, gs::kPsmt8, 1, 1, true, 0x2000, gs::kPsmct32)
         },
         {gs::kPrim, prim(gs::PrimKind::Sprite, false, true, false, true)},
         {gs::kUv, openrac::test::uv(0, 0)},
         {gs::kXyz2, xyz2(160, 100, 0)},
         {gs::kUv, openrac::test::uv(2, 2)},
         {gs::kXyz2, xyz2(200, 140, 0)}}
    );

    // 5. Depth, reversed as on the console: green near (z 1000), then blue
    // farther (z 500) with GEQUAL, overlapping it.
    p.ad(
        {{gs::kTest1,
          openrac::test::test_reg(
              false, gs::AlphaTest::Always, 0, gs::AlphaFail::Keep, gs::DepthTest::Gequal
          )},
         {gs::kPrim, sprite},
         {gs::kRgbaq, rgbaq(0, 0xFF, 0, 0x80)},
         {gs::kXyz2, xyz2(200, 200, 1000)},
         {gs::kXyz2, xyz2(260, 260, 1000)},
         {gs::kRgbaq, rgbaq(0, 0, 0xFF, 0x80)},
         {gs::kXyz2, xyz2(230, 230, 500)},
         {gs::kXyz2, xyz2(290, 290, 500)}}
    );

    // 6. The scissor clips a white sprite to 300..319.
    p.ad(
        {{gs::kTest1,
          openrac::test::test_reg(
              false, gs::AlphaTest::Always, 0, gs::AlphaFail::Keep, gs::DepthTest::Always
          )},
         {gs::kScissor1, openrac::test::scissor(300, 319, 300, 319)},
         {gs::kPrim, sprite},
         {gs::kRgbaq, rgbaq(0xFF, 0xFF, 0xFF, 0x80)},
         {gs::kXyz2, xyz2(280, 280, 0)},
         {gs::kXyz2, xyz2(340, 340, 0)},
         {gs::kScissor1, openrac::test::scissor(0, 511, 0, 447)}}
    );

    // 7. Alpha test GEQUAL 0x40: a sprite at alpha 0x20 fails and keeps
    // nothing; with FB_ONLY it still writes colour.
    p.ad(
        {{gs::kTest1,
          openrac::test::test_reg(
              true, gs::AlphaTest::Gequal, 0x40, gs::AlphaFail::Keep, gs::DepthTest::Always
          )},
         {gs::kPrim, sprite},
         {gs::kRgbaq, rgbaq(0xFF, 0xFF, 0, 0x20)},
         {gs::kXyz2, xyz2(400, 10, 0)},
         {gs::kXyz2, xyz2(440, 50, 0)},
         {gs::kTest1,
          openrac::test::test_reg(
              true, gs::AlphaTest::Gequal, 0x40, gs::AlphaFail::FbOnly, gs::DepthTest::Always
          )},
         {gs::kXyz2, xyz2(450, 10, 0)},
         {gs::kXyz2, xyz2(490, 50, 0)}}
    );

    // 8. A Gouraud triangle: red, green and blue corners.
    p
        .ad({{gs::kTest1,
              openrac::test::test_reg(
                  false, gs::AlphaTest::Always, 0, gs::AlphaFail::Keep, gs::DepthTest::Always
              )},
             {gs::kPrim, prim(gs::PrimKind::Triangle, true, false, false)},
             {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
             {gs::kXyz2, xyz2(20, 400, 0)},
             {gs::kRgbaq, rgbaq(0, 0xFF, 0, 0x80)},
             {gs::kXyz2, xyz2(120, 400, 0)},
             {gs::kRgbaq, rgbaq(0, 0, 0xFF, 0x80)},
             {gs::kXyz2, xyz2(20, 300, 0)}},
            true);
    return p.bytes;
}

}  // namespace

int main() {
    openrac::test::GlContext context;
    std::string why;
    if (!context.open(why)) {
        std::printf("SKIP: no OpenGL 4.1 core context: %s\n", why.c_str());
        return openrac::test::kSkip;
    }

    Renderer renderer;
    std::string error;
    CHECK(renderer.add_game_renderers(error));
    CHECK(renderer.init(error));
    if (!error.empty()) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    FrameBuffer frame;
    if (!frame.create(kWidth, kHeight, error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const std::vector<std::uint8_t> hud = packets();
    FrameInput input;
    input.packets[static_cast<std::size_t>(Bucket::Hud)] = hud;
    renderer.render(input, {frame.id(), kWidth, kHeight});
    const Pixels px{frame.read_rgba()};

    CHECK(renderer.last_stats().draw_calls >= 8);
    CHECK(close(px.at(15, 30), 0xFF, 0, 0, 15, 30));            // red
    CHECK(close(px.at(40, 30), 0x7F, 0, 0x7F, 40, 30));         // half blue over red
    CHECK(close(px.at(60, 30), 0, 0, 0x7F, 60, 30));            // half blue over black
    CHECK(close(px.at(5, 5), 0, 0, 0, 5, 5));                   // the clear colour
    CHECK(close(px.at(105, 105), 0, 0xFF, 0x30, 105, 105));     // texel (0, 0)
    CHECK(close(px.at(135, 105), 0x30, 0xCF, 0x30, 135, 105));  // texel (3, 0)
    CHECK(close(px.at(105, 135), 0xC0, 0x3F, 0x30, 105, 135));  // texel (0, 3)
    CHECK(close(px.at(165, 105), 0x08, 0x20, 0xF7, 165, 105));  // index 0x08
    CHECK(close(px.at(195, 105), 0x10, 0x20, 0xEF, 195, 105));  // index 0x10
    CHECK(close(px.at(195, 135), 0xFF, 0x20, 0x00, 195, 135));  // index 0xFF
    CHECK(close(px.at(240, 240), 0, 0xFF, 0, 240, 240));        // the nearer green stays
    CHECK(close(px.at(280, 280), 0, 0, 0xFF, 280, 280));        // blue where nothing is nearer
    CHECK(close(px.at(310, 310), 0xFF, 0xFF, 0xFF, 310, 310));
    CHECK(close(px.at(290, 310), 0, 0, 0, 290, 310));  // scissored out
    CHECK(close(px.at(330, 330), 0, 0, 0, 330, 330));
    CHECK(close(px.at(420, 30), 0, 0, 0, 420, 30));        // failed, KEEP
    CHECK(close(px.at(470, 30), 0xFF, 0xFF, 0, 470, 30));  // failed, FB_ONLY: colour written
    const std::uint32_t mixed = px.at(50, 370);
    CHECK((mixed & 0xFF) > 0x20 && ((mixed >> 8) & 0xFF) > 0x20 && ((mixed >> 16) & 0xFF) > 0x20);

    // A second frame from the same packets draws the same (the uploads are
    // cached, the registers keep their values).
    renderer.render(input, {frame.id(), kWidth, kHeight});
    CHECK(frame.read_rgba() == px.rgba);

    // Four times the chip's picture: a sprite from half a pixel past the screen's last column (the
    // HUD's air meter slid away) covers no pixel of the chip and leaves none in the window; one
    // from half a pixel before it fills the last game column (from half a game pixel before it, as
    // the whole 2D layer sits on the window: DirectRenderer, convert).
    {
        constexpr int kScale = 4;
        FrameBuffer big;
        CHECK(big.create(kWidth * kScale, kHeight * kScale, error));
        GifBuilder p;
        p.ad(
            {{gs::kTest1,
              openrac::test::test_reg(
                  false, gs::AlphaTest::Always, 0, gs::AlphaFail::Keep, gs::DepthTest::Always
              )},
             {gs::kScissor1, openrac::test::scissor(0, 511, 0, 447)},
             {gs::kPrim, prim(gs::PrimKind::Sprite, false, false, false)},
             {gs::kRgbaq, rgbaq(0xFF, 0, 0, 0x80)},
             {gs::kXyz2, xyz2(511.5, 100, 0)},
             {gs::kXyz2, xyz2(553.5, 200, 0)},
             {gs::kRgbaq, rgbaq(0, 0xFF, 0, 0x80)},
             {gs::kXyz2, xyz2(510.5, 300, 0)},
             {gs::kXyz2, xyz2(552.5, 400, 0)}},
            true
        );
        FrameInput scaled;
        scaled.packets[static_cast<std::size_t>(Bucket::Hud)] = p.bytes;
        renderer.render(scaled, {big.id(), kWidth * kScale, kHeight * kScale});
        const std::vector<std::uint8_t> rgba = big.read_rgba();
        auto at = [&](int x, int y) {
            const auto i = static_cast<std::size_t>((y * kWidth * kScale + x) * 4);
            return static_cast<std::uint32_t>(rgba[i] | (rgba[i + 1] << 8) | (rgba[i + 2] << 16));
        };
        for (int x = kWidth * kScale - 4; x < kWidth * kScale; ++x) {
            CHECK(close(at(x, 150 * kScale), 0, 0, 0, x, 150 * kScale));
            CHECK(close(at(x, 350 * kScale), 0, 0xFF, 0, x, 350 * kScale));
        }
        CHECK(close(at(kWidth * kScale - 7, 350 * kScale), 0, 0, 0, kWidth * kScale - 7, 350 * kScale));
        big.release();
    }

    frame.release();
    renderer.release();
    return openrac::test::result();
}
