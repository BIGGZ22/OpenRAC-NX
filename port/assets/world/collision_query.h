// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-game/src/collision_query.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Queries against a level's collision mesh: a segment, a sphere, a vertical
// capsule (the hero's body) and a sphere against the hero-only groups. They
// follow the game's hand-written EE and VU0 kernels operation for operation
// (ReRAC's docs/plan/collision_queries.md derives them), so a tool or the
// renderer gets the face, point and push-out the game would:
//
//   kernel                          NTSC-U level01   here
//   CollLine_Fix (segment)          0x211870         collide_line
//   sphere                          0x212960         collide_sphere
//   vertical capsule                0x2135a0         collide_capsule
//   sphere vs hero groups           0x214d70         collide_sphere_hero_groups
//   cell lookup                     0x2117d0         CollisionMesh::find_cell
//
// The kernels work in "x1024 space" (world * 1024) relative to the centre of
// the cell under test. With FloatModel::Console every step is the console's
// truncating arithmetic (common/ps2_float.h) on the same operands in the same
// order, so the results match the game's bit for bit as far as that model
// goes; with FloatModel::Native the same steps run in IEEE floats, as the
// port's own code computes. Integer steps (cell mapping, the cell walk's list
// merge, the 16-bit vertex outcodes) are exact in both.
//
// Only the world pass is here. The game's kernels go on to test mobys through
// the 16-unit moby grid and each class's own collision; those belong to the
// running game, which the decompilation supplies.

#pragma once

#include <array>
#include <optional>
#include <vector>

#include "assets/bytes.h"
#include "assets/world/collision.h"

namespace openrac::assets {

enum class FloatModel : u8 {
    Native,   // IEEE single precision
    Console,  // the console's truncating arithmetic (common/ps2_float.h)
};

// The kernels' flags argument (the same bits in the line, sphere and capsule).
namespace collision_flags {

inline constexpr u32 kNone = 0;
// Skip the world mesh (the game then tests mobys only, so here nothing hits).
inline constexpr u32 kSkipWorld = 0x1;
// 0x2, 0x4 and 0x8 select what the moby pass tests; the world pass ignores them.
inline constexpr u32 kSkipMobyPrimitives = 0x2;
inline constexpr u32 kMobySubmask = 0x4;
inline constexpr u32 kMobyFlaggedOnly = 0x8;
// Faces are two-sided: the front-side test is dropped.
inline constexpr u32 kTwoSided = 0x10;
// Exclude faces whose surface id equals (flags >> 8) & 0x1f; see exclude_surface.
inline constexpr u32 kExcludeSurface = 0x20;
// Exclude faces with type bit 7.
inline constexpr u32 kExcludeHighBit = 0x80;

// 0x20 | id << 8. The hero's capsule passes exclude_surface(0) | kMobySubmask
// (0x24) and 0xd24.
constexpr u32 exclude_surface(u8 id) {
    return kExcludeSurface | (u32{id} & 0x1f) << 8;
}

}  // namespace collision_flags

// What the kernels write to their output record (NTSC-U level01 0x1742c0) on
// a hit, in world units unless noted.
struct CollisionHit {
    // +0x1c: 0x1000 | the face's type byte.
    s32 kind = 0;
    // +0x20: the hit point (line) or the closest point on the face (volumes).
    std::array<float, 3> point{};
    // +0x30, volumes only: the centre (the capsule's base) pushed out to touch.
    std::optional<std::array<float, 3>> pushed_centre;
    // +0x40: the raw face normal (v2 - v0) x (v1 - v0) in x1024 units, not
    // normalised (hero groups: x64 units).
    std::array<float, 3> normal{};
    // +0x50, +0x60, +0x70: the hit triangle after the quad split.
    std::array<std::array<float, 3>, 3> triangle{};
    // Not stored by the game: the kernel's running best at return. Line: the
    // hit parameter t along p0 -> p1. Volumes: 0.99951 * d^2 in x1024^2 units
    // (x64^2 for hero groups).
    float best = 0;

    // The game's getters (NTSC-U level01 0x2151d8, 0x215208) on the kind.
    int surface_id() const;
    int sound_class() const;

    std::optional<u8> face_type() const;
};

// One cell of a segment's walk and the parameter at which it is entered.
struct CollisionLineCell {
    float t_enter = 0;
    std::array<s32, 3> cell{};
};

// The segment query p0 -> p1 (CollLine_Fix): the hit nearest p0, or nothing.
//
// Per triangle, N = (v2 - v0) x (v1 - v0), s0 = (v0 - a).N, s1 = (v0 - b).N and
// t = s0 / (d.N). One-sided unless kTwoSided: s0 must be negative and s1
// positive (a on the +N side); the signs must differ in any case. The point
// must pass three inclusive edge tests and be strictly nearer than the best so
// far (1.0 at first). Cells are walked in t order, and once a hit exists the
// walk stops at the first cell entered at or after it.
std::optional<CollisionHit> collide_line(
    const CollisionMesh& mesh,
    const std::array<float, 3>& p0,
    const std::array<float, 3>& p1,
    u32 flags,
    FloatModel model = FloatModel::Native
);

// The sphere query: the face nearest the centre within the radius, and the
// centre pushed out to touch it. One-sided unless kTwoSided. The plane point
// moves to the nearest point of the first failing edge; a face is taken when
// d^2 <= best (best starts at 0.99951 * r^2 and becomes 0.99951 * d^2), so
// every gathered cell is tested.
std::optional<CollisionHit> collide_sphere(
    const CollisionMesh& mesh,
    const std::array<float, 3>& centre,
    float radius,
    u32 flags,
    FloatModel model = FloatModel::Native
);

// The vertical capsule base .. base + (0, 0, height) swept by radius (the
// hero's body). A face is rejected one-sided only when both the base and the
// top are behind it. The axis point tested is the height of v0 for a vertical
// face (|N.z| < 1), the base when it is above all three vertices, the top when
// it is below them all, else where the axis meets the plane, moved to the
// first failing edge and clamped to the axis. Cells are gathered around the
// base sphere only.
std::optional<CollisionHit> collide_capsule(
    const CollisionMesh& mesh,
    const std::array<float, 3>& base,
    float height,
    float radius,
    u32 flags,
    FloatModel model = FloatModel::Native
);

// A sphere against the hero-only groups, which hero movement runs after each
// capsule pass. In x64 units, no world-box or radius checks: groups culled by
// their bounding spheres, then the sphere test, always one-sided with no type
// filters, best starting at (r * 64)^2 unshrunk. The kind is 0x1000.
std::optional<CollisionHit> collide_sphere_hero_groups(
    const CollisionMesh& mesh,
    const std::array<float, 3>& centre,
    float radius,
    FloatModel model = FloatModel::Native
);

// The cells collide_line visits, in order, with their entry parameters; the
// walk ends before the first entry at exactly t = 1. Nothing when the segment
// leaves [0, 1024)^3 or has zero length (both ends truncate to the same x1024
// point), where the query never hits.
std::optional<std::vector<CollisionLineCell>> collision_line_cells(
    const std::array<float, 3>& p0,
    const std::array<float, 3>& p1,
    FloatModel model = FloatModel::Native
);

// The cells collide_sphere tests, Z outer, then Y, then X (before empty ones
// drop out). Nothing outside [0, 1024)^3 or for radius <= 0.
std::optional<std::vector<std::array<s32, 3>>> collision_sphere_cells(
    const std::array<float, 3>& centre, float radius, FloatModel model = FloatModel::Native
);

// The cells collide_capsule tests: those within radius of the base (the height
// widens the enumerated box, not the distance test).
std::optional<std::vector<std::array<s32, 3>>> collision_capsule_cells(
    const std::array<float, 3>& base,
    float height,
    float radius,
    FloatModel model = FloatModel::Native
);

// 0x3f7fdf3b = 0.99951: the volumes' best-distance shrink.
inline constexpr u32 kCollisionBestShrinkBits = 0x3f7f'df3b;

}  // namespace openrac::assets
