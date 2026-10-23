// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/collision.rs
// and crates/rc-game/src/collision_query/tests.rs (their unit tests): ISC License,
// Copyright (c) 2026 ReRAC contributors.
//
// The collision block reader on a synthetic block, and the queries on hand-built
// meshes. A face is front-facing for a query from the side N = (v2 - v0) x
// (v1 - v0) points to: a floor seen from above runs v0, then +y, then +x.

#include "assets/world/collision.h"

#include <algorithm>
#include <bit>
#include <cmath>

#include "assets/bytes.h"
#include "assets/world/collision_query.h"
#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
namespace ps2 = openrac::ps2;

namespace {

using P3 = std::array<float, 3>;

void check_ps2_float() {
    namespace cf = ps2;
    auto f = [](float x) {
        return std::bit_cast<u32>(x);
    };
    CHECK(cf::add(f(65536.0f), f(0.5f / 128.0f * 1.999f)) == f(65536.0f));
    CHECK(cf::add(f(65536.0f), f(1.0f)) == f(65537.0f));
    CHECK(cf::mul(f(1.0f), f(0.3f)) == f(0.3f));
    CHECK(cf::mul(0x3f80'0003, 0x3fc0'0001) == 0x3fc0'0005);  // truncated; IEEE nearest is ..06
    CHECK(cf::add(f(1.0f), f(-1.0f)) == 0);
    CHECK(cf::add(f(1.0f), f(1e-10f)) == f(1.0f));  // an exponent gap of 25 or more
    // The adder drops the smaller operand's bits below half an ulp of the larger.
    CHECK(cf::add(f(1.0f), f(-1.5f * std::ldexp(1.0f, -24))) == 0x3f7f'ffff);
    CHECK(cf::sqrt(f(4.0f)) == f(2.0f));
    CHECK(cf::sqrt(f(2.0f)) == 0x3fb5'04f3);
    CHECK(cf::div(f(1.0f), f(4.0f)) == f(0.25f));
    CHECK(cf::div(f(1.0f), f(3.0f)) == 0x3eaa'aaaa);  // IEEE nearest is ..ab
    CHECK(cf::itof12(0x800) == f(0.5f));
    CHECK(cf::itof12(-4096 * 3) == f(-3.0f));
    CHECK(cf::mul(f(1e30f), f(1e30f)) == cf::kMax);
    CHECK(cf::ftoi(f(0.999f), 0) == 0);
    CHECK(cf::ftoi(f(-1.75f), 0) == -1);
    CHECK(cf::ftoi(f(4095.9f), 0) == 4095);
    CHECK(cf::ftoi(f(1.5e10f), 0) == 2147483647);
    CHECK(cf::ftoi(f(-1.5e10f), 0) == -2147483647 - 1);
    CHECK(cf::itof0(-12) == f(-12.0f));
    CHECK(cf::itof0((1 << 24) + 1) == f(16777216.0f));  // truncated above 2^24
}

void check_packed_vertices() {
    const std::array<std::pair<u32, std::array<s32, 3>>, 10> cases{{
        {0x0000'0000, {0, 0, 0}},
        {0x0000'01ff, {511, 0, 0}},
        {0x0000'0200, {-512, 0, 0}},
        {0x0000'03ff, {-1, 0, 0}},
        {0x0007'fc00, {0, 511, 0}},
        {0x0008'0000, {0, -512, 0}},
        {0x7ff0'0000, {0, 0, 2047}},
        {0x8000'0000, {0, 0, -2048}},
        {0xfff0'0000, {0, 0, -1}},
        {0xffff'ffff, {-1, -1, -1}},
    }};
    for (const auto& [word, fields] : cases) {
        const PackedCollisionVertex p{word};
        CHECK(p.fields() == fields);
        CHECK(PackedCollisionVertex::pack(fields[0], fields[1], fields[2]).word == word);
    }
    // x = -3 (1/16), y = 17 (1/16), z = -129 (1/64) around cell (1, -2, 3).
    const auto p = PackedCollisionVertex::pack(-3, 17, -129);
    CHECK((p.offset() == P3{-0.1875f, 1.0625f, -2.015625f}));
    CollisionCell cell;
    cell.x = 1;
    cell.y = -2;
    cell.z = 3;
    CHECK((cell.centre() == P3{6.0f, -6.0f, 14.0f}));
    CHECK((p.world(cell.centre()) == P3{5.8125f, -4.9375f, 11.984375f}));
    CHECK((cell.low_corner() == P3{4.0f, -8.0f, 12.0f}));
    CHECK((collision_cell_of({5.8125f, 4.9375f, 11.984375f}) == std::array<s32, 3>{1, 1, 2}));
    CHECK((collision_cell_of({0.0f, 3.999f, 1023.99f}) == std::array<s32, 3>{0, 0, 255}));
    CHECK(!collision_cell_of({5.8125f, -4.9375f, 11.984375f}));
    CHECK(!collision_cell_of({1024.0f, 0.0f, 0.0f}));
    const CollisionFace none{{}, 31};
    const CollisionFace mid{{}, 0x4c};
    const CollisionFace high{{}, 0x9f};
    CHECK(none.surface_id() == -1 && none.sound_class() == 0);
    CHECK(mid.surface_id() == 12 && mid.sound_class() == 2 && !mid.high_bit());
    CHECK(high.surface_id() == -1 && high.sound_class() == 0 && high.high_bit());
    CHECK((CollisionFace{{}, 0x7f}.sound_class() == 0));
}

// A block with the mesh at 0x40: root base 5 (z 5 empty, z 6), slab base 3 (y 3
// empty, y 4), row base 10 (x 10 a quad and a triangle, x 11 empty, x 12 a
// triangle), and a hero section at 0x180 with one group.
std::vector<u8> synthetic_block() {
    ByteWriter w;
    w.resize(0x200);
    const std::size_t m = 0x40;
    w.put_at<s32>(0, 0x40);
    w.put_at<s32>(4, 0x180);
    w.put_at<s16>(m, 5);
    w.put_at<u16>(m + 2, 2);
    w.put_at<u16>(m + 4, 0);
    w.put_at<u16>(m + 6, 0x10 / 4);
    w.put_at<s16>(m + 0x10, 3);
    w.put_at<u16>(m + 0x12, 2);
    w.put_at<u32>(m + 0x14, 0);
    w.put_at<u32>(m + 0x18, 0x20);
    w.put_at<s16>(m + 0x20, 10);
    w.put_at<u16>(m + 0x22, 3);
    w.put_at<u32>(m + 0x24, 0x40u << 8 | 2);
    w.put_at<u32>(m + 0x28, 0);
    w.put_at<u32>(m + 0x2c, 0x80u << 8 | 2);
    const std::array<PackedCollisionVertex, 4> v{
        PackedCollisionVertex::pack(-16, -16, 0),
        PackedCollisionVertex::pack(16, -16, 0),
        PackedCollisionVertex::pack(16, 16, 0),
        PackedCollisionVertex::pack(-16, 16, 64),
    };
    // Leaf 0: 2 faces (1 quad), 4 vertices: 4 + 16 + 8 + 1 = 29 bytes, 2 qw.
    const std::size_t l0 = m + 0x40;
    w.put_at<u16>(l0, 2);
    w.put_at<u8>(l0 + 2, 4);
    w.put_at<u8>(l0 + 3, 1);
    for (std::size_t i = 0; i < 4; ++i) {
        w.put_at<u32>(l0 + 4 + 4 * i, v[i].word);
    }
    w.put_at<std::array<u8, 4>>(l0 + 20, {0, 1, 2, 31});
    w.put_at<std::array<u8, 4>>(l0 + 24, {3, 2, 0, 8});
    w.put_at<u8>(l0 + 28, 3);
    // Leaf 1: 1 triangle, 3 vertices: 4 + 12 + 4 = 20 bytes, 2 qw.
    const std::size_t l1 = m + 0x80;
    w.put_at<u16>(l1, 1);
    w.put_at<u8>(l1 + 2, 3);
    w.put_at<u8>(l1 + 3, 0);
    for (std::size_t i = 0; i < 3; ++i) {
        w.put_at<u32>(l1 + 4 + 4 * i, v[i].word);
    }
    w.put_at<std::array<u8, 4>>(l1 + 16, {2, 1, 0, 95});
    // The hero section: 1 group, its data at +0x20: 3 vertices then 1 triangle.
    w.put_at<s32>(0x180, 1);
    w.put_at<std::array<u16, 4>>(0x190, {64, 128, 192, 32});
    w.put_at<u16>(0x198, 1);
    w.put_at<u16>(0x19a, 3);
    w.put_at<u32>(0x19c, 0x20);
    w.put_at<std::array<u16, 4>>(0x1a0, {64, 64, 64, 0});
    w.put_at<std::array<u16, 4>>(0x1a8, {128, 64, 64, 0});
    w.put_at<std::array<u16, 4>>(0x1b0, {64, 128, 64, 0});
    w.put_at<std::array<u8, 4>>(0x1b8, {0, 1, 2, 0});
    return w.bytes();
}

bool parses(const std::vector<u8>& bytes) {
    try {
        read_collision(Game::Rac1, bytes);
        return true;
    } catch (const AssetError&) {
        return false;
    }
}

void check_block() {
    const std::vector<u8> b = synthetic_block();
    const CollisionMesh c = read_collision(Game::Rac1, b);
    CHECK((c.root_entries == std::vector<u16>{0, 4}));
    CHECK(c.slabs.size() == 1 && c.slabs[0].offset == 0x10 && c.slabs[0].z == 6);
    CHECK(c.rows.size() == 1 && c.rows[0].offset == 0x20 && c.rows[0].z == 6 && c.rows[0].y == 4);
    CHECK(c.cells.size() == 2);
    CHECK(c.cells[0].x == 10 && c.cells[0].y == 4 && c.cells[0].z == 6);
    CHECK(c.cells[1].x == 12 && c.cells[1].y == 4 && c.cells[1].z == 6);
    const CollisionCell& c0 = c.cells[0];
    CHECK(c0.leaf_offset() == 0x40 && c0.leaf_qwords() == 2);
    CHECK((c0.face_indices(0) == std::array<u8, 4>{0, 1, 2, 3}));
    CHECK((c0.face_indices(1) == std::array<u8, 4>{3, 2, 0, 3}));
    CHECK((c0.vertices[3] == P3{41.0f, 19.0f, 27.0f}));
    // The raw tree walk agrees with the parsed cells and misses outside and on
    // empty entries.
    const ByteView mesh = ByteView(b).sub(0x40, 0x140);
    CHECK(lookup_collision_leaf(mesh, 10, 4, 6) == (0x40u << 8 | 2));
    CHECK(lookup_collision_leaf(mesh, 12, 4, 6) == (0x80u << 8 | 2));
    const std::array<std::array<s32, 3>, 9> misses{{
        {11, 4, 6},
        {13, 4, 6},
        {9, 4, 6},
        {10, 3, 6},
        {10, 5, 6},
        {10, 4, 5},
        {10, 4, 7},
        {10, 4, 4},
        {10, -4, 6},
    }};
    for (const auto& p : misses) {
        CHECK(!lookup_collision_leaf(mesh, p[0], p[1], p[2]));
        CHECK(c.find_cell(p[0], p[1], p[2]) == nullptr);
    }
    CHECK(c.find_cell(12, 4, 6) == &c.cells[1]);
    // The quad splits along v0-v2.
    const auto t = collision_triangles(c);
    CHECK(t.size() == 4);
    CHECK(t[0].part == 1 && t[1].part == 2 && t[2].part == 0 && t[3].part == 0);
    CHECK(t[0].a == c0.vertices[0] && t[0].b == c0.vertices[1] && t[0].c == c0.vertices[2]);
    CHECK(t[1].a == c0.vertices[0] && t[1].b == c0.vertices[2] && t[1].c == c0.vertices[3]);
    CHECK(t[3].cell == 1 && t[3].face == 0 && t[3].type == 95);
    CHECK((c.type_counts() == std::map<u8, std::size_t>{{8, 1}, {31, 1}, {95, 1}}));
    // The hero group.
    CHECK(c.hero_group_count == 1);
    CHECK((c.hero_groups[0].sphere_world() == std::array<float, 4>{1.0f, 2.0f, 3.0f, 0.5f}));
    CHECK((c.hero_groups[0].vertex_world(1) == P3{2.0f, 1.0f, 1.0f}));

    // Corrupt leaves are rejected: a wrong size byte, a face index past the
    // vertex count, more quads than faces, a non-zero hero pad.
    auto corrupt = [&](std::size_t at, u8 value) {
        std::vector<u8> bad = b;
        bad[at] = value;
        return !parses(bad);
    };
    CHECK(corrupt(0x40 + 0x24, 3));
    CHECK(corrupt(0x40 + 0x80 + 16, 3));
    CHECK(corrupt(0x40 + 0x80 + 3, 2));
    CHECK(corrupt(0x1bb, 1));
    // Other games are not known yet.
    bool refused = false;
    try {
        read_collision(Game::Rac2, b);
    } catch (const AssetError&) {
        refused = true;
    }
    CHECK(refused);
}

// A cell at `c` holding world-space vertices (representable at 1/16 in x, y
// and 1/64 in z from the centre), triangles and quads.
struct Face {
    std::array<u8, 4> v;
    u8 type;
};

CollisionCell make_cell(
    std::array<s16, 3> c,
    const std::vector<P3>& verts,
    const std::vector<Face>& tris,
    const std::vector<Face>& quads = {}
) {
    CollisionCell cell;
    cell.x = c[0];
    cell.y = c[1];
    cell.z = c[2];
    const P3 centre = cell.centre();
    for (const P3& v : verts) {
        const auto x = static_cast<s32>((v[0] - centre[0]) * 16.0f);
        const auto y = static_cast<s32>((v[1] - centre[1]) * 16.0f);
        const auto z = static_cast<s32>((v[2] - centre[2]) * 64.0f);
        const auto p = PackedCollisionVertex::pack(x, y, z);
        CHECK(p.world(centre) == v);
        cell.packed.push_back(p);
        cell.vertices.push_back(v);
    }
    for (const Face& q : quads) {
        cell.faces.push_back({{q.v[0], q.v[1], q.v[2]}, q.type});
        cell.quad_v3.push_back(q.v[3]);
    }
    for (const Face& t : tris) {
        cell.faces.push_back({{t.v[0], t.v[1], t.v[2]}, t.type});
    }
    cell.header.face_count = static_cast<u16>(cell.faces.size());
    cell.header.vertex_count = static_cast<u8>(verts.size());
    cell.header.quad_count = static_cast<u8>(quads.size());
    return cell;
}

CollisionMesh make_mesh(std::vector<CollisionCell> cells) {
    std::sort(cells.begin(), cells.end(), [](const CollisionCell& a, const CollisionCell& b) {
        return std::tie(a.z, a.y, a.x) < std::tie(b.z, b.y, b.x);
    });
    CollisionMesh mesh;
    mesh.cells = std::move(cells);
    return mesh;
}

// A floor triangle at height z over [8, 12]^2 in cell (2, 2, 2).
CollisionMesh floor_mesh(float z, u8 type) {
    return make_mesh(
        {make_cell({2, 2, 2}, {{8, 8, z}, {8, 12, z}, {12, 8, z}}, {{{0, 1, 2, 0}, type}})}
    );
}

bool close(const P3& a, const P3& b, float eps) {
    for (std::size_t k = 0; k < 3; ++k) {
        if (std::fabs(a[k] - b[k]) > eps) {
            return false;
        }
    }
    return true;
}

using namespace collision_flags;

void check_line(FloatModel model) {
    const CollisionMesh m = floor_mesh(10.0f, 0x21);
    const auto h = collide_line(m, {9, 9, 11}, {9, 9, 9}, kNone, model);
    CHECK(h.has_value());
    if (h) {
        CHECK((h->point == P3{9, 9, 10}));
        CHECK(h->kind == 0x1021);
        CHECK(h->surface_id() == 1 && h->sound_class() == 1);
        CHECK(h->best == 0.5f);
        CHECK(!h->pushed_centre);
        // (v2 - v0) x (v1 - v0) in x1024 units: (0, 0, 4096^2).
        CHECK((h->normal == P3{0, 0, 16'777'216.0f}));
        CHECK((h->triangle[0] == P3{8, 8, 10} && h->triangle[1] == P3{8, 12, 10}));
        CHECK((h->triangle[2] == P3{12, 8, 10}));
    }
    // Short of the floor, beside the triangle, leaving the world.
    CHECK(!collide_line(m, {9, 9, 11}, {9, 9, 10.5f}, kNone, model));
    CHECK(!collide_line(m, {11.5f, 11.5f, 11}, {11.5f, 11.5f, 9}, kNone, model));
    CHECK(!collide_line(m, {9, 9, 11}, {9, 9, -1}, kNone, model));
    CHECK(!collide_line(m, {9, 9, 11}, {9, 9, 1024}, kNone, model));
    CHECK(!collide_line(m, {9, 9, 11}, {9, 9, 9}, kSkipWorld, model));
    // Zero length (the same x1024 integer point) never hits.
    CHECK(!collide_line(m, {9, 9, 10}, {9, 9, 10}, kTwoSided, model));
    CHECK(!collision_line_cells({9, 9, 10}, {9, 9, 10.0002f}, model));

    // One-sided and two-sided.
    const CollisionMesh plain = floor_mesh(10.0f, 0x1f);
    CHECK(!collide_line(plain, {9, 9, 9}, {9, 9, 11}, kNone, model));
    const auto up = collide_line(plain, {9, 9, 9}, {9, 9, 11}, kTwoSided, model);
    CHECK(up && up->point == (P3{9, 9, 10}) && up->surface_id() == -1);
    CHECK(!collide_sphere(plain, {9, 9, 9.6f}, 0.5f, kNone, model));
    CHECK(collide_sphere(plain, {9, 9, 9.6f}, 0.5f, kTwoSided, model).has_value());

    // Exclusion flags.
    auto down = [&](const CollisionMesh& mesh, u32 flags) {
        return collide_line(mesh, {9, 9, 11}, {9, 9, 9}, flags, model).has_value();
    };
    const CollisionMesh typed = floor_mesh(10.0f, 0x4c);
    CHECK(!down(typed, exclude_surface(0xc)));
    CHECK(down(typed, exclude_surface(0xd)));
    CHECK(down(typed, 0xd24));
    CHECK(down(typed, kExcludeHighBit));
    const CollisionMesh high = floor_mesh(10.0f, 0x80);
    CHECK(!down(high, kExcludeHighBit));
    CHECK(!down(high, 0x24));
    CHECK(down(high, kNone));
    CHECK(!collide_sphere(high, {9, 9, 10.3f}, 0.5f, 0x24, model));
    CHECK(collide_sphere(high, {9, 9, 10.3f}, 0.5f, kNone, model).has_value());

    // A quad splits (v0, v1, v2) + (v0, v2, v3).
    const CollisionMesh quad = make_mesh({make_cell(
        {2, 2, 2}, {{8, 8, 10}, {8, 12, 10}, {12, 12, 10}, {12, 8, 10}}, {}, {{{0, 1, 2, 3}, 0x05}}
    )});
    const auto upper = collide_line(quad, {9, 11, 11}, {9, 11, 9}, kNone, model);
    CHECK(upper && upper->triangle[2] == (P3{12, 12, 10}) && upper->triangle[1] == (P3{8, 12, 10}));
    const auto lower = collide_line(quad, {11, 9, 11}, {11, 9, 9}, kNone, model);
    CHECK(lower && lower->triangle[1] == (P3{12, 12, 10}) && lower->triangle[2] == (P3{12, 8, 10}));
    CHECK(lower && lower->kind == 0x1005);

    // The edge tests are inclusive: the hypotenuse, the edge x = 8, vertex v0.
    const CollisionMesh one = floor_mesh(10.0f, 1);
    for (const auto& p : std::array<std::array<float, 2>, 3>{{{10, 10}, {8, 9}, {8, 8}}}) {
        const auto hit = collide_line(one, {p[0], p[1], 11}, {p[0], p[1], 9}, kNone, model);
        CHECK(hit && hit->point == (P3{p[0], p[1], 10}));
    }
    CHECK(!collide_line(one, {10, 10.001f, 11}, {10, 10.001f, 9}, kNone, model));
}

void check_line_walk() {
    const FloatModel model = FloatModel::Console;
    // The slope z = 15.25 - x / 2 over x 8..16 stored in cells (2,2,2) type 1
    // and (3,2,2) type 2. The line finds it in (2,2,2) at x 13 (t 0.75); the
    // copy in (3,2,2), entered at t 0.5, is at the same t and not strictly
    // nearer, so it loses.
    const std::vector<P3> tri{{8, 6, 11.25f}, {8, 14, 11.25f}, {16, 6, 7.25f}};
    const P3 a{10, 9, 11};
    const P3 b{14, 9, 8};
    const auto cells = collision_line_cells(a, b, model);
    CHECK(cells && cells->size() == 2);
    if (cells && cells->size() == 2) {
        CHECK(((*cells)[0].cell == std::array<s32, 3>{2, 2, 2}));
        CHECK(((*cells)[1].cell == std::array<s32, 3>{3, 2, 2}));
    }
    auto two_cells = [&](const std::vector<P3>& second) {
        return make_mesh(
            {make_cell({2, 2, 2}, tri, {{{0, 1, 2, 0}, 1}}),
             make_cell({3, 2, 2}, second, {{{0, 1, 2, 0}, 2}})}
        );
    };
    auto h = collide_line(two_cells(tri), a, b, kNone, model);
    CHECK(h && h->kind == 0x1001 && h->best == 0.75f);
    CHECK(h && close(h->point, {13, 9, 8.75f}, 1e-4f));
    // Raise the second copy by 1/64: strictly nearer, it wins.
    std::vector<P3> raised = tri;
    for (P3& v : raised) {
        v[2] += 1.0f / 64.0f;
    }
    h = collide_line(two_cells(raised), a, b, kNone, model);
    CHECK(h && h->kind == 0x1002 && h->best < 0.75f);

    // The walk stops once the next cell starts behind the hit: (2,2,2) has a
    // floor at z 10.75 hit at t 0.25; (3,2,2) a slope the line crosses earlier,
    // at t 1/6, never visited.
    const P3 a2{9, 9, 11.5f};
    const P3 b2{15, 9, 8.5f};
    const std::vector<P3> low{{8, 8, 10.75f}, {8, 12, 10.75f}, {12, 8, 10.75f}};
    const std::vector<P3> slope{{8, 6, 13}, {8, 14, 13}, {16, 6, 5}};
    const CollisionMesh both = make_mesh(
        {make_cell({2, 2, 2}, low, {{{0, 1, 2, 0}, 1}}),
         make_cell({3, 2, 2}, slope, {{{0, 1, 2, 0}, 2}})}
    );
    h = collide_line(both, a2, b2, kTwoSided, model);
    CHECK(h && h->kind == 0x1001 && h->best == 0.25f);
    const CollisionMesh only_slope = make_mesh({make_cell({3, 2, 2}, slope, {{{0, 1, 2, 0}, 2}})});
    h = collide_line(only_slope, a2, b2, kTwoSided, model);
    CHECK(h && h->kind == 0x1002 && std::fabs(h->best - 1.0f / 6.0f) < 1e-6f);
    CHECK(h && close(h->point, {10, 9, 11}, 1e-4f));
    // A face whose box misses the cell's stretch of segment is culled by the
    // outcodes, though the infinite segment would hit it.
    const std::vector<P3> flat{{8, 6, 11}, {8, 14, 11}, {16, 6, 11}};
    CHECK(!collide_line(
        make_mesh({make_cell({3, 2, 2}, flat, {{{0, 1, 2, 0}, 2}})}), a2, b2, kTwoSided, model
    ));
    CHECK(collide_line(
              make_mesh({make_cell({2, 2, 2}, flat, {{{0, 1, 2, 0}, 2}})}), a2, b2, kTwoSided, model
    )
              .has_value());

    // The walk order: ties go x, y, z.
    auto walk = [&](const P3& p, const P3& q) {
        std::vector<std::pair<float, std::array<s32, 3>>> out;
        for (const auto& c :
             collision_line_cells(p, q, model).value_or(std::vector<CollisionLineCell>{})) {
            out.push_back({c.t_enter, c.cell});
        }
        return out;
    };
    using Walk = std::vector<std::pair<float, std::array<s32, 3>>>;
    CHECK((
        walk({1, 1, 1}, {9, 9, 1})
        == Walk{{0.0f, {0, 0, 0}}, {0.375f, {1, 0, 0}}, {0.375f, {1, 1, 0}}, {0.875f, {2, 1, 0}}, {0.875f, {2, 2, 0}}}
    ));
    CHECK(
        (walk({3, 3, 3}, {5, 5, 5})
         == Walk{{0.0f, {0, 0, 0}}, {0.5f, {1, 0, 0}}, {0.5f, {1, 1, 0}}, {0.5f, {1, 1, 1}}})
    );
    CHECK(
        (walk({9, 9, 1}, {1, 7, 1})
         == Walk{{0.0f, {2, 2, 0}}, {0.125f, {1, 2, 0}}, {0.5f, {1, 1, 0}}, {0.625f, {0, 1, 0}}})
    );
    // Ending on a plane: that crossing is t = 1, which ends the walk...
    CHECK((walk({2, 1, 1}, {4, 1, 1}) == Walk{{0.0f, {0, 0, 0}}}));
    // ... unless the truncating reciprocal lands just below 1: 1/3072 rounds
    // toward zero, so (4096 - 1024) * (1/3072) = 0.99999994.
    CHECK((walk({1, 1, 1}, {4, 1, 1}) == Walk{{0.0f, {0, 0, 0}}, {0.99999994f, {1, 0, 0}}}));
    CHECK(!collision_line_cells({1, 1, 1}, {-1, 1, 1}, model));
}

void check_volumes(FloatModel model) {
    const float k = std::bit_cast<float>(kCollisionBestShrinkBits);
    const CollisionMesh m = floor_mesh(10.0f, 3);
    auto h = collide_sphere(m, {9, 9, 10.5f}, 1.0f, kNone, model);
    CHECK(h && h->point == (P3{9, 9, 10}) && h->kind == 0x1003);
    if (h) {
        // best = 0.99951 * (0.5 * 1024)^2; pushed = hit + (c - hit) * r / sqrt(best).
        CHECK(std::fabs(h->best - k * 512.0f * 512.0f) < 0.1f);
        CHECK(
            h->pushed_centre && close(*h->pushed_centre, {9, 9, 10.0f + 1.0f / std::sqrt(k)}, 1e-5f)
        );
        CHECK(h->pushed_centre && (*h->pushed_centre)[2] > 11.0f);
    }
    CHECK(!collide_sphere(m, {9, 9, 10.5f}, 0.49f, kNone, model));
    // Beside the hypotenuse: the nearest point is on the edge.
    h = collide_sphere(m, {10.5f, 10.5f, 10}, 1.0f, kTwoSided, model);
    CHECK(h && h->point == (P3{10, 10, 10}));
    if (h && h->pushed_centre) {
        const float s = 1.0f / std::sqrt(2.0f) / std::sqrt(k);
        CHECK(close(*h->pushed_centre, {10 + s, 10 + s, 10}, 1e-4f));
    }
    CHECK(!collide_sphere(m, {9, 9, 10.5f}, 0.0f, kNone, model));
    CHECK(!collide_sphere(m, {0.5f, 9, 10.5f}, 1.0f, kNone, model));

    // Two floors in one cell: the nearer wins.
    const CollisionMesh two = make_mesh({make_cell(
        {2, 2, 2},
        {{8, 8, 10}, {8, 12, 10}, {12, 8, 10}, {8, 8, 10.25f}, {8, 12, 10.25f}, {12, 8, 10.25f}},
        {{{0, 1, 2, 0}, 1}, {{3, 4, 5, 0}, 2}}
    )});
    h = collide_sphere(two, {9, 9, 10.5f}, 1.0f, kNone, model);
    CHECK(h && h->kind == 0x1002 && h->point == (P3{9, 9, 10.25f}));
    CHECK(
        (collision_sphere_cells({9, 9, 10.5f}, 1.0f, model)
         == std::vector<std::array<s32, 3>>{{2, 2, 2}})
    );
    const auto corner = collision_sphere_cells({8, 8, 8}, 0.5f, model);
    CHECK(corner && corner->size() == 8 && (*corner)[1] == (std::array<s32, 3>{2, 1, 1}));

    // The capsule: a floor 0.3 below the base.
    h = collide_capsule(m, {9, 9, 10.3f}, 1.5f, 0.5f, kNone, model);
    CHECK(h && h->point == (P3{9, 9, 10}));
    CHECK(
        h && h->pushed_centre
        && close(*h->pushed_centre, {9, 9, 10.0f + 0.5f / std::sqrt(k)}, 1e-5f)
    );
    // A wall x = 11 facing -x.
    const CollisionMesh wall =
        make_mesh({make_cell({2, 2, 2}, {{11, 8, 8}, {11, 12, 8}, {11, 8, 12}}, {{{0, 1, 2, 0}, 4}})
        });
    h = collide_capsule(wall, {10.6f, 9, 8.5f}, 2.0f, 0.5f, kNone, model);
    CHECK(h && close(h->point, {11, 9, 8.5f}, 1e-5f));
    CHECK(
        h && h->pushed_centre
        && close(*h->pushed_centre, {11.0f - 0.5f / std::sqrt(k), 9, 8.5f}, 1e-4f)
    );
    CHECK(!collide_capsule(wall, {11.4f, 9, 8.5f}, 2.0f, 0.5f, kNone, model));
    // A ceiling only in cell (2,2,3): the gathering tests the base sphere only.
    const CollisionMesh ceiling = make_mesh(
        {make_cell({2, 2, 3}, {{8, 8, 12.5f}, {12, 8, 12.5f}, {8, 12, 12.5f}}, {{{0, 1, 2, 0}, 1}})}
    );
    const auto gathered = collision_capsule_cells({9, 9, 9}, 3.3f, 0.5f, model);
    CHECK(
        gathered
        && std::find(gathered->begin(), gathered->end(), std::array<s32, 3>{2, 2, 3})
               == gathered->end()
    );
    CHECK(!collide_capsule(ceiling, {9, 9, 9}, 3.3f, 0.5f, kNone, model));
    h = collide_capsule(ceiling, {9, 9, 11.8f}, 0.5f, 0.5f, kNone, model);
    CHECK(h && close(h->point, {9, 9, 12.5f}, 1e-5f));
}

void check_hero_groups(FloatModel model) {
    const float k = std::bit_cast<float>(kCollisionBestShrinkBits);
    auto v = [](float x, float y, float z) {
        return std::array<u16, 3>{
            static_cast<u16>(x * 64.0f), static_cast<u16>(y * 64.0f), static_cast<u16>(z * 64.0f)
        };
    };
    HeroCollisionGroup g;
    g.sphere = {640, 640, 640, 192};
    g.triangle_count = 1;
    g.vertex_count = 3;
    g.vertices = {v(8, 8, 10), v(8, 12, 10), v(12, 8, 10)};
    g.triangles = {{0, 1, 2}};
    CollisionMesh m;
    m.hero_group_count = 1;
    m.hero_groups.push_back(g);
    const auto h = collide_sphere_hero_groups(m, {9, 9, 10.3f}, 0.5f, model);
    CHECK(h && h->kind == 0x1000);
    CHECK(h && close(h->point, {9, 9, 10}, 1e-5f));
    CHECK(
        h && h->pushed_centre
        && close(*h->pushed_centre, {9, 9, 10.0f + 0.5f / std::sqrt(k)}, 1e-4f)
    );
    CHECK(!collide_sphere_hero_groups(m, {9, 9, 10.6f}, 0.5f, model));
    CHECK(!collide_sphere_hero_groups(m, {9, 9, 9.7f}, 0.5f, model));
    CHECK(!collide_sphere_hero_groups(m, {30, 30, 10.3f}, 0.5f, model));
}

}  // namespace

int main() {
    check_ps2_float();
    check_packed_vertices();
    check_block();
    for (FloatModel model : {FloatModel::Console, FloatModel::Native}) {
        check_line(model);
        check_volumes(model);
        check_hero_groups(model);
    }
    check_line_walk();
    return openrac::test::result();
}
