// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), the unit tests of
// crates/rc-formats/src/tie_light.rs, sky.rs and moby.rs: ISC License, Copyright (c)
// 2026 ReRAC contributors.
//
// Tie instance lighting, the sky's shells and textures, and moby classes
// (normals, the vertex cache, the VU0 skinning slots, the index stream).

#include <cmath>
#include <cstring>

#include "assets/geometry/mesh.h"
#include "assets/geometry/moby.h"
#include "assets/geometry/sky.h"
#include "assets/geometry/tie_lighting.h"
#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::assets::rac1;
namespace ps2 = openrac::ps2;

namespace {

using Rgba = std::array<u8, 4>;

u32 fb(f32 x) {
    return ps2::bits(x);
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

// ---- tie lighting ----

TieClass class_with_normals(std::initializer_list<std::array<s16, 4>> normals) {
    TieClass c;
    c.header.scale = 1.0f;
    c.normals.assign(64, {0, 0, 0, 0});
    std::size_t i = 0;
    for (const auto& n : normals) {
        c.normals[i++] = n;
    }
    return c;
}

TieInstance tie_instance(const std::array<std::array<f32, 4>, 4>& m, s32 select, u16 ambient) {
    TieInstance inst{};
    inst.matrix = m;
    inst.directional_lights = select;
    inst.ambient_rgbas.fill(ambient);
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
    // A straight down, colour (1, 0.5, 0.25); B along +X, grey 0.25, back factor -0.5.
    bank.sets[2] =
        {{1.0f, 0.5f, 0.25f, 0}, {0, 0, -1, 0}, {0.25f, 0.25f, 0.25f, -0.5f}, {1, 0, 0, 0}};
    bank.count = 3;
    return bank;
}

constexpr u16 kAmbient = 0x8000 | 5 | 6 << 5 | 7 << 10;  // (40, 48, 56), alpha set

void tie_identity_instance() {
    // Slot 0 faces up, slot 1 faces -X, slot 2 faces +X.
    const TieClass c = class_with_normals({{0, 0, 32767, 0}, {-32767, 0, 0, 0}, {32767, 0, 0, 0}});
    const auto lit = light_tie_instance(c, tie_instance(kAtFiveSixSeven, 2, kAmbient), sun_bank());
    CHECK((lit[0] == Rgba{40 + 127, 48 + 63, 56 + 31, 0x80}));
    CHECK((lit[1] == Rgba{40 + 31, 48 + 31, 56 + 31, 0x80}));
    CHECK((lit[2] == Rgba{40 + 15, 48 + 15, 56 + 15, 0x80}));
    CHECK((lit[3] == Rgba{40, 48, 56, 0x80}));
    const auto no_alpha =
        light_tie_instance(c, tie_instance(kAtFiveSixSeven, 2, 5 | 6 << 5 | 7 << 10), sun_bank());
    CHECK((no_alpha[3] == Rgba{40, 48, 56, 0}));
}

void tie_clamp_is_243() {
    const TieClass c = class_with_normals({{0, 0, 32767, 0}});
    const auto lit = light_tie_instance(
        c, tie_instance(kAtFiveSixSeven, 2, 0x8000 | 31 | 31 << 5 | 31 << 10), sun_bank()
    );
    CHECK((lit[0] == Rgba{243, 243, 243, 0x80}));
}

void tie_rotation_not_scale() {
    // Class +X rotated onto world +Z with scale 3.
    const TieClass c = class_with_normals({{32767, 0, 0, 0}});
    const std::array<std::array<f32, 4>, 4> m = {
        {{0, 0, 3, 0}, {0, 3, 0, 0}, {-3, 0, 0, 0}, {0, 0, 0, 0.01f}}
    };
    const TieInstance inst = tie_instance(m, 2, 0x8000);
    CHECK((light_tie_instance(c, inst, sun_bank())[0] == Rgba{127, 63, 31, 0x80}));
    // Through the PS2's unit column: 3 * trunc(1/3) is one ULP below 1.0.
    const InstanceLightRegs regs = tie_light_regs(c, inst, sun_bank());
    CHECK(regs.rows[0][0] == 0x3f7f'ffff);
    CHECK((regs.back == std::array<u32, 3>{0, fb(-0.5f), 0}));
}

void tie_blended_and_unused_sets() {
    const TieClass c = class_with_normals({{0, 0, 32767, 0}});
    const auto blended = light_tie_instance(
        c, tie_instance(kAtFiveSixSeven, 0x80 << 8 | 3 << 4 | 2, kAmbient), sun_bank()
    );
    CHECK((blended[0] == Rgba{40 + 63, 48 + 31, 56 + 15, 0x80}));
    const auto unused =
        light_tie_instance(c, tie_instance(kAtFiveSixSeven, 15, kAmbient), sun_bank());
    CHECK((unused[0] == Rgba{40, 48, 56, 0x80}));
}

void tie_point_lights() {
    TieClass c = class_with_normals({{0, 0, 32767, 0}});
    c.header.bsphere = {0, 0, 0, 1};
    const TieInstance inst = tie_instance(kAtFiveSixSeven, 15, 0x8000);
    CHECK((tie_instance_centre(c, inst) == std::array<f32, 4>{5, 6, 7, 1}));
    PointLightBank points{};
    points[3] = {{1, 1, 1, 0}, {5, 6, 15, 16}};
    points[4] = {{1, 0, 0, 0}, {5, 6, 40, 16}};  // out of range
    const TiePointLights list{&points, 0xff43};
    const InstanceLightRegs regs = tie_light_regs(c, inst, LightBank{}, list);
    CHECK((regs.colors[2] == ps2::V4{fb(0.5f), fb(0.5f), fb(0.5f), 0}));
    CHECK(regs.rows[0][2] == 0 && regs.rows[1][2] == 0 && regs.rows[2][2] == fb(1.0f));
    CHECK((light_tie_instance(c, inst, LightBank{}, list)[0] == Rgba{63, 63, 63, 0x80}));
    const TiePointLights none{&points, 0xffff};
    CHECK((light_tie_instance(c, inst, LightBank{}, none)[0] == Rgba{0, 0, 0, 0x80}));
}

// ---- the sky ----

template <typename T>
void put(std::vector<u8>& b, std::size_t at, const T& v) {
    std::memcpy(b.data() + at, &v, sizeof(T));
}

// One 2x2 texture, a textured shell and a gouraud shell of one cluster each.
std::vector<u8> sky_block(s32 textured_flags) {
    std::vector<u8> b(0x900, 0);
    SkyHeader h;
    h.shell_count = 2;
    h.texture_count = 1;
    h.texture_defs = 0x40;
    h.texture_data = 0x400;
    h.shells[0] = 0x100;
    h.shells[1] = 0x200;
    put(b, 0, h);
    put(b, 0x40, SkyTextureDef{0, 0x400, 2, 2});
    // CLUT at 0x400: entry j = (j, 0, 0, 0x80). Pixels at 0x800.
    for (std::size_t j = 0; j < 256; ++j) {
        b[0x400 + 4 * j] = static_cast<u8>(j);
        b[0x400 + 4 * j + 3] = 0x80;
    }
    b[0x800] = 0;
    b[0x801] = 1;
    b[0x802] = 16;
    b[0x803] = 8;
    const std::array<SkyVertex, 3> verts =
        {SkyVertex{100, 0, 0, 0x80}, SkyVertex{0, 100, 0, 0x40}, SkyVertex{0, 0, 100, 0}};

    struct ShellSpec {
        std::size_t at;
        s32 flags;
        std::size_t data;
        std::array<u32, 3> attrs;
        std::array<std::array<u8, 4>, 2> faces;
    };

    const std::array<ShellSpec, 2> shells = {
        ShellSpec{
            0x100,
            textured_flags,
            0x300,
            {0x1000u << 16, 0x0800, 0xf000'0000},
            {{{0, 1, 2, 0}, {2, 1, 0, 0}}}
        },
        ShellSpec{
            0x200,
            1,
            0x340,
            {0x8011'2233, 0x8044'5566, 0x4000'0000},
            {{{0, 2, 1, 0xff}, {1, 2, 0, 0xff}}}
        },
    };
    for (const ShellSpec& s : shells) {
        put<s32>(b, s.at, 1);
        put<s32>(b, s.at + 4, s.flags);
        SkyClusterHeader ch;
        ch.bsphere = {0, 0, 0, 100};
        ch.data = static_cast<s32>(s.data);
        ch.vertex_count = 3;
        ch.tri_count = 2;
        ch.st_offset = 0x18;
        ch.tri_offset = 0x24;
        ch.data_size = 0x30;
        put(b, s.at + 0x10, ch);
        for (std::size_t i = 0; i < 3; ++i) {
            put(b, s.data + 8 * i, verts[i]);
            put(b, s.data + 0x18 + 4 * i, s.attrs[i]);
        }
        for (std::size_t i = 0; i < 2; ++i) {
            put(b, s.data + 0x24 + 4 * i, s.faces[i]);
        }
    }
    return b;
}

void sky_shells() {
    const auto b = sky_block(0);
    const Sky sky = parse_sky(b);
    CHECK(sky.shells.size() == 2);
    const SkyShell& textured = sky.shells[0];
    CHECK(textured.textured());
    const auto g = sky_gs_vertices(textured, textured.clusters[0]);
    CHECK(g.size() == 6);
    // ST zero-extended: 0xf000 is 15.0, not -1.0.
    CHECK((g[0] == SkyGsVertex{{100, 0, 0}, 0, {0.0f, 1.0f}, {0x80, 0x80, 0x80, 0x80}}));
    CHECK((g[1].st == std::array<f32, 2>{0.5f, 0.0f}) && g[1].rgba[3] == 0x40);
    CHECK((g[2].st == std::array<f32, 2>{0.0f, 15.0f}));
    CHECK((g[3].position == std::array<s16, 3>{0, 0, 100}));
    // Gouraud: the attribute word is the colour; the face's texture byte is ignored.
    const SkyShell& gouraud = sky.shells[1];
    CHECK(!gouraud.textured());
    const auto h = sky_gs_vertices(gouraud, gouraud.clusters[0]);
    CHECK((h[0].rgba == Rgba{0x33, 0x22, 0x11, 0x80}));
    CHECK((h[1].rgba == Rgba{0, 0, 0, 0x40}));
    CHECK(h[1].texture == 0xff && (h[1].st == std::array<f32, 2>{0, 0}));
    // Any non-zero flags word means gouraud.
    const Sky two = parse_sky(sky_block(2));
    CHECK(!two.shells[0].textured());
    CHECK(sky_gs_vertices(two.shells[0], two.shells[0].clusters[0])[0].texture == 0xff);
}

void sky_textures() {
    const auto b = sky_block(0);
    const auto t = decode_sky_textures(b, parse_sky(b));
    CHECK(t.size() == 1);
    if (t.size() == 1) {
        CHECK(t[0].key() == "sky/00_2x2");
        CHECK(
            (t[0].image.rgba
             == std::vector<u8>{0, 0, 0, 0xff, 1, 0, 0, 0xff, 8, 0, 0, 0xff, 16, 0, 0, 0xff})
        );
    }
}

void sky_rejects() {
    auto b = sky_block(0);
    b[0x300 + 0x24] = 3;  // a face index == vertex_count
    CHECK(throws([&] { parse_sky(b); }));
    b = sky_block(0);
    b[0x300 + 0x27] = 1;  // texture 1 of 1
    CHECK(throws([&] { parse_sky(b); }));
    b = sky_block(0);
    b[0x100 + 0x10 + 0x1e] = 0x28;  // data_size cuts the face array
    CHECK(throws([&] { parse_sky(b); }));
    b = sky_block(0);
    b[6] = 9;  // shell_count
    CHECK(throws([&] { parse_sky(b); }));
}

// ---- moby classes ----

bool close3(const std::array<f32, 3>& a, const std::array<f32, 3>& b) {
    return std::abs(a[0] - b[0]) < 1e-6f && std::abs(a[1] - b[1]) < 1e-6f
           && std::abs(a[2] - b[2]) < 1e-6f;
}

void moby_normals() {
    CHECK(close3(spherical_normal(0, 0), {1, 0, 0}));
    CHECK(close3(spherical_normal(64, 0), {0, 1, 0}));
    CHECK(close3(spherical_normal(0, 64), {0, 0, 1}));
    CHECK(close3(spherical_normal(128, 0), {-1, 0, 0}));
    CHECK(close3(spherical_normal(192, 0), {0, -1, 0}));
    const f32 h = static_cast<f32>(std::sqrt(0.5));
    CHECK(close3(spherical_normal(32, 32), {0.5f, 0.5f, h}));
    const auto n = spherical_normal(17, 200);
    CHECK(std::abs(n[0] * n[0] + n[1] * n[1] + n[2] * n[2] - 1.0f) < 1e-6f);
}

void moby_vertex_cache() {
    // Ten vertices in the file and one epilogue record: records 7..10 carry
    // the ids of vertices 0..3, the epilogue's bytes 4.. the other six.
    std::vector<std::array<u8, 16>> recs(11);
    for (std::size_t i = 7; i < 11; ++i) {
        const auto id = static_cast<u16>((0x100 + i) | 0xfe00);
        std::memcpy(recs[i].data(), &id, 2);
    }
    for (std::size_t k = 0; k < 6; ++k) {
        const auto id = static_cast<u16>((0x1f0 + k) | 0x200);
        std::memcpy(recs[10].data() + 4 + 2 * k, &id, 2);
    }
    CHECK(
        (vertex_cache_ids(recs, 10)
         == std::vector<u16>{0x107, 0x108, 0x109, 0x10a, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5})
    );
    CHECK(throws([&] { vertex_cache_ids(std::span(recs).first(8), 8); }));
    CHECK(duplicate_cache_id(0x1ab << 7) == 0x1ab);
    CHECK(duplicate_cache_id(0xffff) == 0x1ff);
}

void moby_slot_machine() {
    Vu0Slots s;
    s.store(0, MobySkin::joint(5));
    s.store(4, MobySkin::joint(9));
    const MobySkin two = s.vertex(MobyVertexKind::TwoWay, {0, 2 << 1, 0, 4, 100, 156, 8, 0x40});
    CHECK((two == MobySkin{2, {5, 9, 0}, {100, 156, 0}}));
    const MobySkin three =
        s.vertex(MobyVertexKind::ThreeWay, {0x77, 8 | 1, 0, 4, 50, 60, 146, 0x44});
    CHECK((three == MobySkin{3, {5, 9, 2}, {50, 60, 146}}));
    CHECK(
        s.vertex(MobyVertexKind::Single, {0, 7 << 1, 0xc, 0xc, 0, 0, 0, 0}) == MobySkin::joint(7)
    );
    CHECK(s.vertex(MobyVertexKind::Single, {0, 0, 0x40, 0xf4, 0, 0, 0, 0}) == two);
    CHECK(throws([&] { s.vertex(MobyVertexKind::TwoWay, {0, 0, 0x40, 0, 128, 128, 0xf4, 0xf4}); }));
    CHECK(throws([&] { s.vertex(MobyVertexKind::Single, {0, 0, 0x80, 0xf4, 0, 0, 0, 0}); }));
    CHECK(throws([&] { s.vertex(MobyVertexKind::Single, {0, 0, 2, 0xf4, 0, 0, 0, 0}); }));
    CHECK(throws([&] { s.vertex(MobyVertexKind::TwoWay, {0, 0, 0, 4, 100, 100, 0xf4, 0xf4}); }));
    CHECK(throws([&] { s.vertex(MobyVertexKind::Single, {0, 0, 0xf4, 0xf4, 0, 0, 0, 0}); }));
    CHECK(metal_skin({3, 4, 5, 1, 0, 0, 0, 0}) == MobySkin::joint(3));
    CHECK((metal_skin({3, 4, 5, 3, 16, 32, 208, 0}) == MobySkin{3, {3, 4, 5}, {16, 32, 208}}));
    CHECK(throws([&] { metal_skin({3, 4, 5, 2, 16, 32, 208, 0}); }));
}

MobyPacket walk_packet(
    std::vector<u8> indices,
    std::vector<u8> secrets,
    std::vector<s32> textures,
    std::size_t vertices
) {
    MobyPacket p;
    p.vertices.resize(vertices);
    p.index_bytes = std::move(indices);
    p.secret_indices = std::move(secrets);
    p.texture_indices = std::move(textures);
    p.initial_texture = 7;
    return p;
}

void moby_index_stream() {
    // 0: texture 3 and secret vertex 2 (no kick); 0x83 no kick; 4 and 5 kick;
    // 0: texture 1 and secret 6; 0x81 no kick; 2 kicks; the trailer 1, 1, 1; 0 ends.
    const auto t = moby_triangles(
        walk_packet({0, 0x83, 4, 5, 0, 0x81, 2, 1, 1, 1, 0, 0}, {0x82, 0x86, 0}, {3, 1}, 6)
    );
    std::vector<std::array<s64, 4>> abc;
    for (const MobyTriangle& tri : t) {
        abc.push_back({tri.a, tri.b, tri.c, tri.texture});
    }
    CHECK((abc == std::vector<std::array<s64, 4>>{{1, 2, 3, 3}, {2, 3, 4, 3}, {5, 0, 1, 1}}));
    // Without a switch the packet keeps the previous packet's texture.
    const auto kept = moby_triangles(walk_packet({0x81, 0x82, 3, 1, 1, 1, 0}, {0}, {}, 3));
    CHECK(
        kept.size() == 1 && kept[0].a == 0 && kept[0].b == 1 && kept[0].c == 2
        && kept[0].texture == 7
    );
    CHECK(throws([&] { moby_triangles(walk_packet({0x81, 0x82, 3, 1, 1, 1}, {0}, {}, 3)); }));
    CHECK(throws([&] { moby_triangles(walk_packet({0x81, 0x82, 4, 1, 1, 1, 0}, {0}, {}, 3)); }));
}

}  // namespace

int main() {
    tie_identity_instance();
    tie_clamp_is_243();
    tie_rotation_not_scale();
    tie_blended_and_unused_sets();
    tie_point_lights();
    sky_shells();
    sky_textures();
    sky_rejects();
    moby_normals();
    moby_vertex_cache();
    moby_slot_machine();
    moby_index_stream();
    return openrac::test::result();
}
