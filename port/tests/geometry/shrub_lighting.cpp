// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), the unit tests of
// crates/rc-formats/src/shrub_light.rs: ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// Shrub instance lighting: the VU1 colour-address quirk, normals, light
// sets, the clamp, rotation and mirroring, point lights, the average.

#include "assets/geometry/shrub_lighting.h"

#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::assets::rac1;
namespace ps2 = openrac::ps2;

namespace {

using Rgba = std::array<u8, 4>;
using Kind = ShrubVu1Data::Kind;

u32 fb(f32 x) {
    return ps2::bits(x);
}

void vu1_quirk() {
    const u16 base = shrub_vu1_palette_base(0xee, 0);
    CHECK(base == 0xf3);
    CHECK(shrub_vu1_colour_address(6, 2, 7, base) == 0xf3 + 7);
    CHECK(shrub_vu1_colour_address(8, 3, 7, base) == 0xf3 + 7);
    CHECK((shrub_vu1_data(0xf3 + 7) == ShrubVu1Data{Kind::Palette, 0xee, 0, 7}));
    // Vertex 3 of a 6-vertex packet, slot 0 of buffer 0xee, lands in the
    // other buffer: slot 3's palette, slot 4's matrix or slot 4's palette.
    CHECK(
        (shrub_vu1_data(shrub_vu1_colour_address(6, 3, 2, base))
         == ShrubVu1Data{Kind::Palette, 0x17b, 3, 20})
    );
    CHECK(
        (shrub_vu1_data(shrub_vu1_colour_address(6, 3, 7, base))
         == ShrubVu1Data{Kind::Matrix, 0x17b, 4, 1})
    );
    CHECK(
        (shrub_vu1_data(shrub_vu1_colour_address(6, 3, 20, base))
         == ShrubVu1Data{Kind::Palette, 0x17b, 4, 10})
    );
    // Any later slot, or the 0x17b buffer, lands in a GS output buffer.
    const ShrubVu1Data a =
        shrub_vu1_data(shrub_vu1_colour_address(6, 3, 0, shrub_vu1_palette_base(0xee, 1)));
    CHECK(a.kind == Kind::Other && a.index == 0x21e);
    const ShrubVu1Data b =
        shrub_vu1_data(shrub_vu1_colour_address(6, 3, 23, shrub_vu1_palette_base(0x17b, 4)));
    CHECK(b.kind == Kind::Other && b.index == 0x3f7);
}

ShrubClass class_with_normals(std::initializer_list<std::array<s16, 4>> normals) {
    ShrubClass c;
    c.header.scale = 1.0f;
    c.normals.assign(kShrubNormals, {0, 0, 0, 0});
    std::size_t i = 0;
    for (const auto& n : normals) {
        c.normals[i++] = n;
    }
    return c;
}

ShrubInstance shrub_instance(
    const std::array<std::array<f32, 4>, 4>& m, s32 select, std::array<s32, 3> colour
) {
    ShrubInstance inst;
    inst.matrix = m;
    inst.dir_lights = select;
    inst.colour = colour;
    inst.draw_distance = 32;
    return inst;
}

constexpr std::array<std::array<f32, 4>, 4> kAtFiveSixSeven = {{
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {5, 6, 7, 0.01f},
}};

LightBank sun_bank() {
    LightBank bank;
    bank.sets[2] =
        {{1.0f, 0.5f, 0.25f, 0}, {0, 0, -1, 0}, {0.25f, 0.25f, 0.25f, -0.5f}, {1, 0, 0, 0}};
    bank.count = 3;
    return bank;
}

void normals_by_32768() {
    const ShrubClass c =
        class_with_normals({{0, 0, 16384, 0}, {0, 0, 32767, 0}, {0, 0, -32768, 0}});
    const ShrubPalette p =
        light_shrub_instance(c, shrub_instance(kAtFiveSixSeven, 2, {40, 48, 56}), sun_bank());
    CHECK((p[0] == Rgba{40 + 64, 48 + 32, 56 + 16, 0x80}));
    CHECK((p[1] == Rgba{40 + 127, 48 + 63, 56 + 31, 0x80}));
    CHECK((p[2] == Rgba{40, 48, 56, 0x80}));  // facing down, no back factor
    CHECK((p[3] == Rgba{40, 48, 56, 0x80}));  // a zero normal: ambient only
}

void back_factor() {
    const ShrubClass c = class_with_normals({{-32767, 0, 0, 0}, {32767, 0, 0, 0}});
    const ShrubPalette p =
        light_shrub_instance(c, shrub_instance(kAtFiveSixSeven, 2, {40, 48, 56}), sun_bank());
    CHECK((p[0] == Rgba{40 + 31, 48 + 31, 56 + 31, 0x80}));
    CHECK((p[1] == Rgba{40 + 15, 48 + 15, 56 + 15, 0x80}));
}

void clamp_and_alpha() {
    const ShrubClass c = class_with_normals({{0, 0, 32767, 0}});
    CHECK(
        (light_shrub_instance(c, shrub_instance(kAtFiveSixSeven, 2, {248, 200, 255}), sun_bank())[0]
         == Rgba{243, 243, 243, 0x80})
    );
    CHECK(shrub_packed_ambient(shrub_instance(kAtFiveSixSeven, 2, {1, 2, 3})) == 0x8003'0201);
    // The loader ORs the raw channels: 0x1ff red spills into green's byte.
    CHECK(shrub_packed_ambient(shrub_instance(kAtFiveSixSeven, 2, {0x1ff, 0, 0})) == 0x8000'01ff);
}

void blend() {
    const ShrubClass c = class_with_normals({{0, 0, 32767, 0}, {32767, 0, 0, 0}});
    const ShrubInstance inst =
        shrub_instance(kAtFiveSixSeven, 0x80 << 8 | 3 << 4 | 2, {40, 48, 56});
    const ShrubPalette p = light_shrub_instance(c, inst, sun_bank());
    CHECK((p[0] == Rgba{40 + 63, 48 + 31, 56 + 15, 0x80}));
    // B's back factor stays -0.5 (not -0.25 as a xyzw blend would give).
    const ShrubLightRegs regs = shrub_light_regs(c, inst, sun_bank());
    CHECK((regs.lights.back == std::array<u32, 3>{0, fb(-0.5f), 0}));
    CHECK((regs.lights.colors[1] == ps2::V4{fb(0.125f), fb(0.125f), fb(0.125f), 0}));
    CHECK((p[1] == Rgba{40 + 7, 48 + 7, 56 + 7, 0x80}));
    CHECK(
        (light_shrub_instance(c, shrub_instance(kAtFiveSixSeven, 15, {40, 48, 56}), sun_bank())[0]
         == Rgba{40, 48, 56, 0x80})
    );
}

void rotation_and_mirror() {
    const ShrubClass c = class_with_normals({{32767, 0, 0, 0}});
    std::array<std::array<f32, 4>, 4> m = {
        {{0, 0, 3, 0}, {0, 3, 0, 0}, {-3, 0, 0, 0}, {0, 0, 0, 0.01f}}
    };
    CHECK(
        (light_shrub_instance(c, shrub_instance(m, 2, {0, 0, 0}), sun_bank())[0]
         == Rgba{127, 63, 31, 0x80})
    );
    CHECK(
        shrub_light_regs(c, shrub_instance(m, 2, {0, 0, 0}), sun_bank()).lights.rows[0][0]
        == 0x3f7f'ffff
    );
    m[0] = {0, 0, -3, 0};  // mirrored: class +X faces world -Z
    CHECK(
        (light_shrub_instance(c, shrub_instance(m, 2, {0, 0, 0}), sun_bank())[0]
         == Rgba{0, 0, 0, 0x80})
    );
}

void point_lights() {
    ShrubClass c = class_with_normals({{0, 0, 32767, 0}});
    c.header.bsphere = {0, 0, 1, 1.5f};
    c.header.scale = 2.0f;
    const ShrubInstance inst = shrub_instance(kAtFiveSixSeven, 15, {0, 0, 0});
    CHECK((shrub_instance_centre(c, inst) == std::array<f32, 4>{5, 6, 9, 3}));
    PointLightBank points{};
    points[3] = {{1, 1, 1, 0}, {5, 6, 17, 16}};
    points[4] = {{1, 0, 0, 0}, {5, 6, 40, 16}};
    const ShrubPointLights list{&points, 0xff43};
    const ShrubLightRegs regs = shrub_light_regs(c, inst, LightBank{}, list);
    CHECK((regs.lights.colors[2] == ps2::V4{fb(0.5f), fb(0.5f), fb(0.5f), 0}));
    CHECK(
        regs.lights.rows[0][2] == 0 && regs.lights.rows[1][2] == 0
        && regs.lights.rows[2][2] == fb(1.0f)
    );
    CHECK((light_shrub_instance(c, inst, LightBank{}, list)[0] == Rgba{63, 63, 63, 0x80}));
    CHECK(
        (light_shrub_instance(c, inst, LightBank{}, ShrubPointLights{&points, 0xffff})[0]
         == Rgba{0, 0, 0, 0x80})
    );
}

void average() {
    ShrubPalette p{};
    p[0] = {24, 48, 47, 0x80};
    CHECK((shrub_average_colour(p) == std::array<u8, 3>{1, 2, 1}));
}

}  // namespace

int main() {
    vu1_quirk();
    normals_by_32768();
    back_factor();
    clamp_and_alpha();
    blend();
    rotation_and_mirror();
    point_lights();
    average();
    return openrac::test::result();
}
