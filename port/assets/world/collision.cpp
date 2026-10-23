// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/collision.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Implements collision.h: the small accessors of the collision types, the parser that walks the
// Z, Y, X tree of a level's collision block into cells, the hero-only groups, and the raw tree
// lookup the game's queries use. The parser checks every offset, count and index it follows, so a
// damaged block fails with a message naming the part instead of reading outside the block.

#include "assets/world/collision.h"

#include <algorithm>
#include <tuple>

#include "assets/world/known_games.h"

namespace openrac::assets {

// X is bits 0..9, Y bits 10..19 and Z bits 20..31 of the word (see collision.h).
std::array<s32, 3> PackedCollisionVertex::fields() const {
    // Shift pairs on the signed word, as the game and Wrench extract them.
    return {
        static_cast<s32>(word << 22) >> 22,
        static_cast<s32>(word << 12) >> 22,
        static_cast<s32>(word) >> 20,
    };
}

// X and Y are in 1/16 units, Z in 1/64 (the scales Wrench uses, see collision.h).
std::array<float, 3> PackedCollisionVertex::offset() const {
    const auto f = fields();
    return {
        static_cast<float>(f[0]) / 16.0f,
        static_cast<float>(f[1]) / 16.0f,
        static_cast<float>(f[2]) / 64.0f,
    };
}

// Float addition of the cell centre and the offset, in the game's order.
std::array<float, 3> PackedCollisionVertex::world(const std::array<float, 3>& centre) const {
    const auto o = offset();
    return {centre[0] + o[0], centre[1] + o[1], centre[2] + o[2]};
}

// Masks to the field widths: 10 bits for X and Y, 12 for Z.
PackedCollisionVertex PackedCollisionVertex::pack(s32 x, s32 y, s32 z) {
    return {
        (static_cast<u32>(x) & 0x3ff) | (static_cast<u32>(y) & 0x3ff) << 10
        | (static_cast<u32>(z) & 0xfff) << 20
    };
}

// Type bits 0..4; the value 0x1f means the face has no surface.
int CollisionFace::surface_id() const {
    return (type & 0x1f) == 0x1f ? -1 : type & 0x1f;
}

// Type bits 5..6; the value 3 reads as 0 (as the game's getter does).
int CollisionFace::sound_class() const {
    return (type & 0x60) == 0x60 ? 0 : (type & 0x60) >> 5;
}

// Header word, 4 bytes per vertex, 4 per face, 1 per quad's fourth index.
std::size_t CollisionLeafHeader::payload_size() const {
    return 4 + 4 * std::size_t{vertex_count} + 4 * std::size_t{face_count} + quad_count;
}

// Cells are 4 units wide, so the centre of cell c is 4c + 2 (kCollisionCellSize / 2).
std::array<float, 3> CollisionCell::centre() const {
    return {
        static_cast<float>(x) * 4.0f + 2.0f,
        static_cast<float>(y) * 4.0f + 2.0f,
        static_cast<float>(z) * 4.0f + 2.0f,
    };
}

// Cell c starts at 4c.
std::array<float, 3> CollisionCell::low_corner() const {
    return {
        static_cast<float>(x) * kCollisionCellSize,
        static_cast<float>(y) * kCollisionCellSize,
        static_cast<float>(z) * kCollisionCellSize,
    };
}

// Throws (via at()) for a face index past the cell's faces.
std::array<u8, 4> CollisionCell::face_indices(std::size_t face) const {
    const CollisionFace& f = faces.at(face);
    // Only quads have a fourth index stored; a triangle repeats its first.
    return {f.v[0], f.v[1], f.v[2], is_quad(face) ? quad_v3.at(face) : f.v[0]};
}

// The sphere's x, y, z and radius are unsigned at 1/64 (see collision.h).
std::array<float, 4> HeroCollisionGroup::sphere_world() const {
    return {
        static_cast<float>(sphere[0]) / 64.0f,
        static_cast<float>(sphere[1]) / 64.0f,
        static_cast<float>(sphere[2]) / 64.0f,
        static_cast<float>(sphere[3]) / 64.0f,
    };
}

// Hero vertices are absolute, unsigned at 1/64.
std::array<float, 3> HeroCollisionGroup::vertex_world(std::size_t i) const {
    const auto& v = vertices.at(i);
    return {
        static_cast<float>(v[0]) / 64.0f,
        static_cast<float>(v[1]) / 64.0f,
        static_cast<float>(v[2]) / 64.0f,
    };
}

// Binary search over the cells, which are sorted by (z, y, x) in tree order.
const CollisionCell* CollisionMesh::find_cell(s32 x, s32 y, s32 z) const {
    const auto key = std::make_tuple(z, y, x);
    auto it = std::lower_bound(
        cells.begin(),
        cells.end(),
        key,
        [](const CollisionCell& c, const std::tuple<s32, s32, s32>& k) {
            return std::make_tuple(s32{c.z}, s32{c.y}, s32{c.x}) < k;
        }
    );

    // lower_bound gives the first cell not below the key; it is a hit only if it equals the key.
    if (it == cells.end() || std::make_tuple(s32{it->z}, s32{it->y}, s32{it->x}) != key) {
        return nullptr;
    }

    return &*it;
}

std::map<u8, std::size_t> CollisionMesh::type_counts() const {
    std::map<u8, std::size_t> counts;

    // A face stored in several cells counts once per cell, as it is stored.
    for (const CollisionCell& c : cells) {
        // Tally every face of the cell by its type byte.
        for (const CollisionFace& f : c.faces) {
            ++counts[f.type];
        }
    }

    return counts;
}

namespace {

// The most entries a node may claim; the real levels stay well below it (assumed).
constexpr std::size_t kMaxNodeEntries = 4096;

// The most hero groups accepted before the count is called damaged (assumed).
constexpr s32 kMaxHeroGroups = 100'000;

// A leaf's size is counted in 16-byte units.
constexpr std::size_t kLeafUnitBytes = 16;

// A node is {s16 base, u16 count}, followed by its entries.
CollisionNode read_node(ByteView mesh, std::size_t at) {
    return {mesh.s16_at(at), mesh.u16_at(at + 2)};
}

// Reads a node and checks that its `count` entries of `entry_size` bytes lie inside the mesh.
// `what` names the tree level in the error.
CollisionNode checked_node(
    ByteView mesh, std::size_t at, std::size_t entry_size, const char* what
) {
    const CollisionNode node = read_node(mesh, at);

    // A huge count is a misread node; reject it before it sizes a loop.
    if (node.count > kMaxNodeEntries) {
        fail("collision: implausible {} count {}", what, node.count);
    }

    mesh.check(at + 4, node.count * entry_size, what);
    return node;
}

// Decodes one leaf: header, packed vertices, faces, then the quads' fourth indices. `word` is the
// tree entry (leaf offset << 8 | size in quadwords). Every face index is checked against the
// vertex count.
CollisionCell read_leaf(ByteView mesh, s16 x, s16 y, s16 z, u32 word) {
    const std::size_t leaf = word >> 8;
    CollisionCell cell;
    cell.x = x;
    cell.y = y;
    cell.z = z;
    cell.leaf_word = word;
    cell.header = {mesh.u16_at(leaf), mesh.u8_at(leaf + 2), mesh.u8_at(leaf + 3)};
    const std::size_t faces = cell.header.face_count;
    const std::size_t vertices = cell.header.vertex_count;
    const std::size_t quads = cell.header.quad_count;

    // The first `quads` faces are quads, so there cannot be more quads than faces.
    if (quads > faces) {
        fail("collision: cell ({}, {}, {}) has more quads than faces", x, y, z);
    }

    // The tree word carries the leaf's size in its low byte; a mismatch means a wrong leaf.
    if (cell.header.size_qwords() != (word & 0xff)) {
        fail(
            "collision: cell ({}, {}, {}): the leaf size byte does not match its contents", x, y, z
        );
    }

    mesh.check(leaf, cell.header.size_qwords() * kLeafUnitBytes, "collision leaf");
    const std::array<float, 3> centre = cell.centre();

    // Vertices come first, one packed word each, and are also kept as world positions.
    for (std::size_t i = 0; i < vertices; ++i) {
        const PackedCollisionVertex p{mesh.u32_at(leaf + 4 + 4 * i)};
        cell.packed.push_back(p);
        cell.vertices.push_back(p.world(centre));
    }

    const std::size_t face_at = leaf + 4 + 4 * vertices;

    // Faces follow: three vertex indices and the type byte each.
    for (std::size_t i = 0; i < faces; ++i) {
        const std::size_t at = face_at + 4 * i;
        cell.faces.push_back(
            {{mesh.u8_at(at), mesh.u8_at(at + 1), mesh.u8_at(at + 2)}, mesh.u8_at(at + 3)}
        );
    }

    const ByteView fourth = mesh.sub(face_at + 4 * faces, quads, "collision quad indices");
    cell.quad_v3 = fourth.to_vector();

    // Check every index a face uses, so later reads of `vertices` cannot go out of range.
    for (std::size_t i = 0; i < faces; ++i) {
        const std::size_t corners = i < quads ? 4 : 3;
        const auto idx = cell.face_indices(i);

        // A quad has four corners, a triangle three.
        for (std::size_t k = 0; k < corners; ++k) {
            // An index past the vertex list would read outside the leaf.
            if (idx[k] >= vertices) {
                fail("collision: cell ({}, {}, {}): face index out of range", x, y, z);
            }
        }
    }

    return cell;
}

// Reads the hero section: a count at +0, then 0x10-byte group records from +0x10, each pointing to
// its own vertices and triangles.
void read_hero_groups(ByteView hero, CollisionMesh& mesh) {
    mesh.hero_group_count = hero.s32_at(0);

    // A negative or huge count is a misread section.
    if (mesh.hero_group_count < 0 || mesh.hero_group_count > kMaxHeroGroups) {
        fail("collision: implausible hero group count {}", mesh.hero_group_count);
    }

    // One record per group: sphere, counts, and the offset of the group's data.
    for (s32 g = 0; g < mesh.hero_group_count; ++g) {
        const std::size_t at = 0x10 + 0x10 * static_cast<std::size_t>(g);
        HeroCollisionGroup group;

        // The bounding sphere is four u16: x, y, z, radius.
        for (std::size_t k = 0; k < 4; ++k) {
            group.sphere[k] = hero.u16_at(at + 2 * k);
        }

        group.triangle_count = hero.u16_at(at + 8);
        group.vertex_count = hero.u16_at(at + 10);
        group.data = hero.u32_at(at + 12);
        const std::size_t data = group.data;
        const std::size_t nv = group.vertex_count;
        const std::size_t nt = group.triangle_count;
        hero.check(data, 8 * nv + 4 * nt, "hero collision group");

        // Vertices are 8 bytes: three u16 and a pad.
        for (std::size_t i = 0; i < nv; ++i) {
            const std::size_t v = data + 8 * i;

            // A non-zero pad would be a variant of the format this reader does not know.
            if (hero.u16_at(v + 6) != 0) {
                fail("collision: unknown hero vertex variant (non-zero pad)");
            }

            group.vertices.push_back({hero.u16_at(v), hero.u16_at(v + 2), hero.u16_at(v + 4)});
        }

        // Triangles follow the vertices, 4 bytes each: three indices and a pad.
        for (std::size_t i = 0; i < nt; ++i) {
            const std::size_t t = data + 8 * nv + 4 * i;
            const std::array<u8, 3> v{hero.u8_at(t), hero.u8_at(t + 1), hero.u8_at(t + 2)};

            // Indices are group-local, so each must be below the group's vertex count.
            for (u8 index : v) {
                // Each corner index of the triangle is group-local, so it must be below the vertex
                // count.
                if (index >= nv) {
                    fail("collision: hero group index out of range");
                }
            }

            // As for vertices, a non-zero pad is an unknown variant.
            if (hero.u8_at(t + 3) != 0) {
                fail("collision: unknown hero triangle variant (non-zero pad)");
            }

            group.triangles.push_back(v);
        }

        mesh.hero_groups.push_back(std::move(group));
    }
}

// Entry `coord - base` when it lies in [0, count). The game reads the base
// unsigned (lhu) and misses on i < 0 or count - i <= 0; one unsigned compare
// is the same test.
std::optional<std::size_t> entry_index(CollisionNode node, s32 coord) {
    const auto i = static_cast<u32>(coord - static_cast<s32>(static_cast<u16>(node.base)));

    // An unsigned index below the count is inside the node.
    if (i < node.count) {
        return i;
    }

    return std::nullopt;
}

}  // namespace

// Walks the tree Z, then Y, then X, so slabs, rows and cells come out in tree order.
CollisionMesh read_collision(Game game, ByteView block) {
    require_world_layout(game, "collision");
    CollisionMesh mesh;
    mesh.mesh_offset = block.s32_at(0);
    mesh.hero_offset = block.s32_at(4);

    // The mesh follows the block's two offsets, so its offset must be positive.
    if (mesh.mesh_offset <= 0) {
        fail("collision: no mesh offset");
    }

    // The mesh is bounded by the hero section after it, so the hero section cannot come first.
    if (mesh.hero_offset != 0 && mesh.hero_offset < mesh.mesh_offset) {
        fail("collision: the hero section comes before the mesh");
    }

    // The mesh runs to the hero section, or to the end of the block.
    const std::size_t start = static_cast<std::size_t>(mesh.mesh_offset);
    const std::size_t end =
        mesh.hero_offset > 0 ? static_cast<std::size_t>(mesh.hero_offset) : block.size();
    const ByteView m = block.sub(start, end > start ? end - start : 0, "collision mesh");

    mesh.root = checked_node(m, 0, 2, "collision z");

    // The root entries are u16 slab offsets divided by 4.
    for (std::size_t zi = 0; zi < mesh.root.count; ++zi) {
        mesh.root_entries.push_back(m.u16_at(4 + 2 * zi));
    }

    // One slab per Z that has any geometry.
    for (std::size_t zi = 0; zi < mesh.root_entries.size(); ++zi) {
        // A zero entry is a Z with no geometry.
        if (mesh.root_entries[zi] == 0) {
            continue;
        }

        const auto z = static_cast<s16>(mesh.root.base + static_cast<s16>(zi));
        CollisionSlab slab;
        slab.offset = u32{mesh.root_entries[zi]} * 4;
        slab.z = z;
        slab.node = checked_node(m, slab.offset, 4, "collision y");

        // The slab's entries are u32 row offsets.
        for (std::size_t yi = 0; yi < slab.node.count; ++yi) {
            slab.rows.push_back(m.u32_at(slab.offset + 4 + 4 * yi));
        }

        // One row per Y that has any geometry.
        for (std::size_t yi = 0; yi < slab.rows.size(); ++yi) {
            // A zero entry is a Y with no geometry.
            if (slab.rows[yi] == 0) {
                continue;
            }

            CollisionRow row;
            row.offset = slab.rows[yi];
            row.z = z;
            row.y = static_cast<s16>(slab.node.base + static_cast<s16>(yi));
            row.node = checked_node(m, row.offset, 4, "collision x");

            // The row's entries are u32 leaf words.
            for (std::size_t xi = 0; xi < row.node.count; ++xi) {
                row.leaves.push_back(m.u32_at(row.offset + 4 + 4 * xi));
            }

            // One cell per X that has a leaf.
            for (std::size_t xi = 0; xi < row.leaves.size(); ++xi) {
                // A zero word is an empty cell.
                if (row.leaves[xi] != 0) {
                    const auto x = static_cast<s16>(row.node.base + static_cast<s16>(xi));
                    mesh.cells.push_back(read_leaf(m, x, row.y, z, row.leaves[xi]));
                }
            }

            mesh.rows.push_back(std::move(row));
        }

        mesh.slabs.push_back(std::move(slab));
    }

    // The hero section is optional; read it when the block has one.
    if (mesh.hero_offset > 0) {
        read_hero_groups(
            block.tail(static_cast<std::size_t>(mesh.hero_offset), "hero collision"), mesh
        );
    }

    return mesh;
}

// Mirrors the game's walk: each level returns nothing for an out-of-range or zero entry.
std::optional<u32> lookup_collision_leaf(ByteView mesh, s32 x, s32 y, s32 z) {
    const auto zi = entry_index(read_node(mesh, 0), z);

    // The Z index is outside the root's range, so the cell is empty.
    if (!zi) {
        return std::nullopt;
    }

    const std::size_t slab = std::size_t{mesh.u16_at(4 + 2 * *zi)} * 4;

    // A zero slab offset is a Z with no geometry.
    if (slab == 0) {
        return std::nullopt;
    }

    const auto yi = entry_index(read_node(mesh, slab), y);

    // The Y index is outside the slab's range.
    if (!yi) {
        return std::nullopt;
    }

    const std::size_t row = mesh.u32_at(slab + 4 + 4 * *yi);

    // A zero row offset is a Y with no geometry.
    if (row == 0) {
        return std::nullopt;
    }

    const auto xi = entry_index(read_node(mesh, row), x);

    // The X index is outside the row's range.
    if (!xi) {
        return std::nullopt;
    }

    const u32 word = mesh.u32_at(row + 4 + 4 * *xi);

    // A zero word is an empty cell.
    if (word == 0) {
        return std::nullopt;
    }

    return word;
}

// The game computes trunc(p * 1024) >> 12 for lines; the multiplication is exact in a float.
std::optional<std::array<s32, 3>> collision_cell_of(const std::array<float, 3>& p) {
    std::array<s32, 3> cell{};

    // Check and convert each axis in turn.
    for (std::size_t k = 0; k < 3; ++k) {
        // The game rejects any position outside [0, 1024) on an axis; NaN fails the test too.
        if (!(p[k] >= 0.0f && p[k] < kCollisionWorldSize)) {
            return std::nullopt;
        }

        // 1024 and 12 are the game's scale and shift for lines: floor(p / 4) in range.
        cell[k] = static_cast<s32>(p[k] * 1024.0f) >> 12;
    }

    return cell;
}

// A quad's second half is (v0, v2, v3), its first (v0, v1, v2); `part` records which.
std::vector<CollisionTriangle> collision_triangles(const CollisionMesh& mesh) {
    std::vector<CollisionTriangle> out;

    // Walk the cells in order, so the triangles come out in cell then face order.
    for (std::size_t ci = 0; ci < mesh.cells.size(); ++ci) {
        const CollisionCell& cell = mesh.cells[ci];

        // Every face of the cell is emitted.
        for (std::size_t fi = 0; fi < cell.faces.size(); ++fi) {
            const auto v = cell.face_indices(fi);
            const bool quad = cell.is_quad(fi);

            // A quad makes two triangles along its v0-v2 diagonal, a triangle makes one.
            for (std::size_t half = 0; half < (quad ? 2u : 1u); ++half) {
                out.push_back({
                    cell.vertices[v[0]],
                    cell.vertices[v[half + 1]],
                    cell.vertices[v[half + 2]],
                    static_cast<u32>(ci),
                    static_cast<u16>(fi),
                    cell.faces[fi].type,
                    static_cast<u8>(quad ? half + 1 : 0),
                });
            }
        }
    }

    return out;
}

}  // namespace openrac::assets
