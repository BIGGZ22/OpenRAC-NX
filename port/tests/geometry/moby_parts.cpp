// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), the unit tests of
// crates/rc-formats/src/moby_collision.rs, moby_shadow.rs and moby_light.rs: ISC License,
// Copyright (c) 2026 ReRAC contributors.
//
// Moby class collision and shadow blocks, and moby lighting, on synthetic
// data.

#include <cmath>
#include <numbers>

#include "assets/geometry/moby_collision.h"
#include "assets/geometry/moby_lighting.h"
#include "assets/geometry/moby_shadow.h"
#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::assets::rac1;
namespace ps2 = openrac::ps2;

namespace {

u32 fb(f32 x) {
    return ps2::bits(x);
}

std::vector<u8> collision_blob(
    const std::vector<std::array<u32, 8>>& prims,
    const std::vector<std::array<s16, 4>>& verts,
    const std::vector<std::array<u8, 4>>& faces,
    std::array<u16, 2> counts
) {
    ByteWriter w;
    w.put(counts[0]);
    w.put(counts[1]);
    w.put(static_cast<s32>(prims.size() * 0x20));
    w.put(static_cast<s32>(faces.size() * 4));
    w.put(static_cast<s32>(verts.size() * 8));
    for (const auto& p : prims) {
        w.put(p);
    }
    for (const auto& v : verts) {
        w.put(v);
    }
    for (const auto& f : faces) {
        w.put(f);
    }
    return w.bytes();
}

template <typename F>
bool throws(F&& f) {
    try {
        f();
    } catch (const AssetError&) {
        return true;
    }
    return false;
}

void collision_sections_and_fields() {
    const std::array<u32, 8> sphere = {0x8003'0401, 0, 0, 0, 0, 0, fb(1.5f), fb(2.0f)};
    const std::array<u32, 8> cylinder = {0x0002'0403, fb(3.0f), 0, 0, 0, 0, 0, fb(0.5f)};
    const auto b = collision_blob(
        {cylinder, sphere},
        {{0, 0, 0, 0}, {1024, 0, 0, 0}, {0, 1024, 0, 0}},
        {{0, 1, 2, 0x1f}},
        {0, 6}
    );
    const MobyCollision c = parse_moby_collision(b);
    CHECK(c.size() == b.size());
    CHECK((c.joint_counts == std::array<u16, 2>{0, 6}));
    CHECK(
        c.primitives[0].kind() == 3 && c.primitives[0].mask() == 2 && c.primitives[0].byte1() == 4
    );
    CHECK(c.primitives[0].f(1) == 3.0f);
    CHECK(c.primitives[1].kind() == 1 && c.primitives[1].mask() == -0x7ffd);
    CHECK(c.primitives[1].f(7) == 2.0f);
    CHECK((c.vertices[1] == std::array<s16, 4>{1024, 0, 0, 0}));
    CHECK(c.faces.size() == 1 && c.faces[0][3] == 0x1f);
    const MobyCollisionPrimitive joint{{0x8002'0404, 0x0007'0005, 0, 0, 0, 0, 0, 0}};
    CHECK((joint.joints() == std::array<s16, 2>{5, 7}) && joint.word4() == 0x0007'0005);
}

void collision_rejects() {
    const std::array<u32, 8> open = {0x0002'0401, 0, 0, 0, 0, 0, 0, 0};
    const std::array<u32, 8> end = {0x8002'0401, 0, 0, 0, 0, 0, 0, 0};
    CHECK(throws([&] { parse_moby_collision(collision_blob({open}, {}, {}, {0, 0})); }));
    CHECK(throws([&] { parse_moby_collision(collision_blob({end, open}, {}, {}, {0, 0})); }));
    CHECK(throws([&] {
        parse_moby_collision(collision_blob({}, {{0, 0, 0, 0}}, {{0, 0, 1, 0}}, {0, 0}));
    }));
    CHECK(!throws([&] { parse_moby_collision(collision_blob({open, end}, {}, {}, {0, 0})); }));
}

void shadow_record(ByteWriter& w, u8 type, u8 last, const std::vector<u32>& words) {
    w.put(type);
    w.put(last);
    w.put(static_cast<u16>(type == 0 ? 0x20 : 0x30));
    for (const u32 x : words) {
        w.put(x);
    }
}

void shadow_blocks() {
    ByteWriter w;
    shadow_record(
        w,
        1,
        0,
        {0x0007'0003,
         6,
         static_cast<u32>(-10),
         fb(1.0f),
         fb(2.0f),
         fb(3.0f),
         fb(0.5f),
         fb(4.0f),
         fb(5.0f),
         fb(6.0f),
         fb(0.25f)}
    );
    shadow_record(w, 0, 1, {9, 5, 0, fb(7.0f), fb(8.0f), fb(9.0f), fb(1.5f)});
    const std::vector<u8> b = w.bytes();
    const ShadowBlock s = parse_shadow_block(b);
    CHECK(s.primitives.size() == 2);
    CHECK(
        (s.primitives[0]
         == ShadowPrimitive{ShadowCapsule{{3, 7}, {6, -10}, {1, 2, 3, 0.5f}, {4, 5, 6, 0.25f}}})
    );
    CHECK((s.primitives[1] == ShadowPrimitive{ShadowSphere{9, 5, {7, 8, 9, 1.5f}}}));

    std::vector<u8> longer = b;
    longer.resize(b.size() + 16);
    CHECK(throws([&] { parse_shadow_block(longer); }));
    std::vector<u8> bad = b;
    bad[0] = 2;
    CHECK(throws([&] { parse_shadow_block(bad); }));
    std::vector<u8> open = b;
    open[0x31] = 0;
    CHECK(throws([&] { parse_shadow_block(open); }));
    std::vector<u8> size = b;
    size[2] = 0x20;
    CHECK(throws([&] { parse_shadow_block(size); }));

    // A block of two quadwords before a skeleton at 0x60.
    std::vector<u8> blob(0x60, 0);
    blob[0xf] = 2;
    blob[0x14] = 0x60;
    ByteWriter one;
    shadow_record(one, 0, 1, {1, 4, 0, 0, 0, 0, fb(2.0f)});
    std::copy(one.bytes().begin(), one.bytes().end(), blob.begin() + 0x40);
    const auto block = moby_class_shadow(blob);
    CHECK(block && block->primitives.size() == 1);
    blob[0xf] = 0;
    CHECK(!moby_class_shadow(blob));
}

NormalTable test_table() {
    NormalTable t;
    for (std::size_t i = 0; i < 256; ++i) {
        const double a = static_cast<double>(i) * 2 * std::numbers::pi / 256.0;
        t.entries[i] = {fb(static_cast<f32>(std::cos(a))), fb(static_cast<f32>(std::sin(a)))};
    }
    t.entries[0] = {fb(1.0f), 0};
    t.entries[64] = {0, fb(1.0f)};
    t.entries[128] = {fb(-1.0f), 0};
    t.entries[192] = {0, fb(-1.0f)};
    return t;
}

LightBank bank_with(const DirLightSet& set) {
    LightBank b;
    b.sets[1] = set;
    b.count = 2;
    return b;
}

void sine_and_rotation() {
    for (const f32 a : {-3.1f, -2.0f, -0.5f, 0.25f, 1.0f, 1.5f, 2.5f, 3.1f, -5.183544f}) {
        const auto [s, c] = vu0_sin_cos(fb(a));
        CHECK(std::abs(ps2::to_float(s) - std::sin(a)) < 2e-4f);
        CHECK(std::abs(ps2::to_float(c) - std::cos(a)) < 2e-4f);
    }
    CHECK(rotation_rows({0, 0, 0}) == kIdentityRows);
    // +90 degrees about Z maps model +X to world +Y, model +Y to world -X.
    const Rows r = rotation_rows({0, 0, std::numbers::pi_v<f32> / 2});
    CHECK(std::abs(ps2::to_float(r[0][0])) < 1e-4f && std::abs(ps2::to_float(r[0][1]) - 1) < 1e-4f);
    CHECK(std::abs(ps2::to_float(r[1][0]) + 1) < 1e-4f && std::abs(ps2::to_float(r[1][1])) < 1e-4f);
    // A generic triple against Rz Ry Rx in double precision: row i = column i.
    const double x = 0.3;
    const double y = -1.1;
    const double z = 2.7;
    const double rx[3][3] =
        {{1, 0, 0}, {0, std::cos(x), -std::sin(x)}, {0, std::sin(x), std::cos(x)}};
    const double ry[3][3] =
        {{std::cos(y), 0, std::sin(y)}, {0, 1, 0}, {-std::sin(y), 0, std::cos(y)}};
    const double rz[3][3] =
        {{std::cos(z), -std::sin(z), 0}, {std::sin(z), std::cos(z), 0}, {0, 0, 1}};
    double ryx[3][3] = {};
    double m[3][3] = {};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                ryx[i][j] += ry[i][k] * rx[k][j];
            }
        }
    }
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                m[i][j] += rz[i][k] * ryx[k][j];
            }
        }
    }
    const Rows g = rotation_rows({static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(z)});
    for (std::size_t i = 0; i < 3; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            CHECK(std::abs(static_cast<double>(ps2::to_float(g[i][j])) - m[j][i]) < 3e-4);
        }
    }
    const Rows mirrored = instance_rows({0, 0, 0}, 0x8000);
    CHECK(ps2::to_float(mirrored[1][1]) == -1.0f);
}

void lighting() {
    const NormalTable t = test_table();
    const std::array<u8, 4> full = {0x80, 0x80, 0x80, 0x80};
    // Light A travels straight down, colour (1, 0.5, 0.25), back factor 0.5;
    // light B along +X, colour 0.25.
    const LightBank bank =
        bank_with({{1.0f, 0.5f, 0.25f, 0.5f}, {0, 0, -1, 0}, {0.25f, 0.25f, 0.25f, 0}, {1, 0, 0, 0}}
        );
    const MobyLights l = moby_lights(kIdentityRows, bank, 1, {41, 42, 43}, 0x80);
    CHECK(ps2::to_float(l.rows[2][0]) == 1.0f && ps2::to_float(l.rows[2][1]) == 0.0f);
    CHECK(ps2::to_float(l.rows[0][1]) == -1.0f);
    CHECK((ps2::to_floats(l.neg_k) == std::array<f32, 4>{-0.5f, 0, 0, 1}));
    CHECK((light_vertex(l, t, kIdentityRows, 0, 64, full) == std::array<u8, 4>{169, 106, 75, 0x80})
    );
    CHECK((light_vertex(l, t, kIdentityRows, 0, 192, full) == std::array<u8, 4>{105, 74, 59, 0x80})
    );
    CHECK((light_vertex(l, t, kIdentityRows, 128, 0, full) == std::array<u8, 4>{73, 74, 75, 0x80}));
    CHECK(
        (light_vertex(l, t, kIdentityRows, 0, 64, {0x40, 0x40, 0x40, 0x40})
         == std::array<u8, 4>{84, 53, 37, 0x40})
    );
    const MobyLights hot = moby_lights(kIdentityRows, bank, 1, {200, 0, 0}, 0x80);
    CHECK(light_vertex(hot, t, kIdentityRows, 0, 64, full)[0] == 255);

    // Rolled 180 degrees about X: a model up-facing normal faces away.
    const LightBank sun = bank_with({{1, 1, 1, 0.25f}, {0, 0, -1, 0}, {}, {}});
    const MobyLights rolled = moby_lights(
        rotation_rows({std::numbers::pi_v<f32>, 0, 0}), sun, 1, {0x40, 0x40, 0x40}, 0x80
    );
    const u8 back = light_vertex(rolled, t, kIdentityRows, 0, 64, full)[0];
    CHECK(back >= 95 && back <= 96);
    const u8 front = light_vertex(rolled, t, kIdentityRows, 0, 192, full)[0];
    CHECK(front >= 191 && front <= 192);
    // A 50% cross-fade with an empty set halves the colour.
    const MobyLights faded = moby_lights(kIdentityRows, sun, 0x80'02'01, {0x40, 0x40, 0x40}, 0x80);
    CHECK((
        light_vertex(faded, t, kIdentityRows, 0, 64, full) == std::array<u8, 4>{128, 128, 128, 0x80}
    ));

    CHECK(
        (pack_colour({0x4780'0000 + 300, 0x4780'0000 + 255, 0x4780'0000, 0x4780'0080}, full)
         == std::array<u8, 4>{255, 255, 0, 0x80})
    );
    CHECK(pack_colour({0x4780'8000, 0, 0, 0}, full)[0] == 0);
}

}  // namespace

int main() {
    collision_sections_and_fields();
    collision_rejects();
    shadow_blocks();
    sine_and_rotation();
    lighting();
    return openrac::test::result();
}
