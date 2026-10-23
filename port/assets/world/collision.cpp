// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/collision.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The collision block's tree walk, leaf decoding and hero groups.

#include "assets/world/collision.h"

#include <algorithm>
#include <tuple>

#include "assets/world/known_games.h"

namespace openrac::assets {

std::array<s32, 3> PackedCollisionVertex::fields() const {
    // Shift pairs on the signed word, as the game and Wrench extract them.
    return {
        static_cast<s32>(word << 22) >> 22,
        static_cast<s32>(word << 12) >> 22,
        static_cast<s32>(word) >> 20,
    };
}

std::array<float, 3> PackedCollisionVertex::offset() const {
    const auto f = fields();
    return {
        static_cast<float>(f[0]) / 16.0f,
        static_cast<float>(f[1]) / 16.0f,
        static_cast<float>(f[2]) / 64.0f,
    };
}

std::array<float, 3> PackedCollisionVertex::world(const std::array<float, 3>& centre) const {
    const auto o = offset();
    return {centre[0] + o[0], centre[1] + o[1], centre[2] + o[2]};
}

PackedCollisionVertex PackedCollisionVertex::pack(s32 x, s32 y, s32 z) {
    return {
        (static_cast<u32>(x) & 0x3ff) | (static_cast<u32>(y) & 0x3ff) << 10
        | (static_cast<u32>(z) & 0xfff) << 20
    };
}

int CollisionFace::surface_id() const {
    return (type & 0x1f) == 0x1f ? -1 : type & 0x1f;
}

int CollisionFace::sound_class() const {
    return (type & 0x60) == 0x60 ? 0 : (type & 0x60) >> 5;
}

std::size_t CollisionLeafHeader::payload_size() const {
    return 4 + 4 * std::size_t{vertex_count} + 4 * std::size_t{face_count} + quad_count;
}

std::array<float, 3> CollisionCell::centre() const {
    return {
        static_cast<float>(x) * 4.0f + 2.0f,
        static_cast<float>(y) * 4.0f + 2.0f,
        static_cast<float>(z) * 4.0f + 2.0f,
    };
}

std::array<float, 3> CollisionCell::low_corner() const {
    return {
        static_cast<float>(x) * kCollisionCellSize,
        static_cast<float>(y) * kCollisionCellSize,
        static_cast<float>(z) * kCollisionCellSize,
    };
}

std::array<u8, 4> CollisionCell::face_indices(std::size_t face) const {
    const CollisionFace& f = faces.at(face);
    return {f.v[0], f.v[1], f.v[2], is_quad(face) ? quad_v3.at(face) : f.v[0]};
}

std::array<float, 4> HeroCollisionGroup::sphere_world() const {
    return {
        static_cast<float>(sphere[0]) / 64.0f,
        static_cast<float>(sphere[1]) / 64.0f,
        static_cast<float>(sphere[2]) / 64.0f,
        static_cast<float>(sphere[3]) / 64.0f,
    };
}

std::array<float, 3> HeroCollisionGroup::vertex_world(std::size_t i) const {
    const auto& v = vertices.at(i);
    return {
        static_cast<float>(v[0]) / 64.0f,
        static_cast<float>(v[1]) / 64.0f,
        static_cast<float>(v[2]) / 64.0f,
    };
}

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

    if (it == cells.end() || std::make_tuple(s32{it->z}, s32{it->y}, s32{it->x}) != key) {
        return nullptr;
    }

    return &*it;
}

std::map<u8, std::size_t> CollisionMesh::type_counts() const {
    std::map<u8, std::size_t> counts;

    for (const CollisionCell& c : cells) {
        for (const CollisionFace& f : c.faces) {
            ++counts[f.type];
        }
    }

    return counts;
}

namespace {

CollisionNode read_node(ByteView mesh, std::size_t at) {
    return {mesh.s16_at(at), mesh.u16_at(at + 2)};
}

// A node and the bytes of its entries.
CollisionNode checked_node(
    ByteView mesh, std::size_t at, std::size_t entry_size, const char* what
) {
    const CollisionNode node = read_node(mesh, at);

    if (node.count > 4096) {
        fail("collision: implausible {} count {}", what, node.count);
    }

    mesh.check(at + 4, node.count * entry_size, what);
    return node;
}

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

    if (quads > faces) {
        fail("collision: cell ({}, {}, {}) has more quads than faces", x, y, z);
    }

    if (cell.header.size_qwords() != (word & 0xff)) {
        fail(
            "collision: cell ({}, {}, {}): the leaf size byte does not match its contents", x, y, z
        );
    }

    mesh.check(leaf, cell.header.size_qwords() * 16, "collision leaf");
    const std::array<float, 3> centre = cell.centre();

    for (std::size_t i = 0; i < vertices; ++i) {
        const PackedCollisionVertex p{mesh.u32_at(leaf + 4 + 4 * i)};
        cell.packed.push_back(p);
        cell.vertices.push_back(p.world(centre));
    }

    const std::size_t face_at = leaf + 4 + 4 * vertices;

    for (std::size_t i = 0; i < faces; ++i) {
        const std::size_t at = face_at + 4 * i;
        cell.faces.push_back(
            {{mesh.u8_at(at), mesh.u8_at(at + 1), mesh.u8_at(at + 2)}, mesh.u8_at(at + 3)}
        );
    }

    const ByteView fourth = mesh.sub(face_at + 4 * faces, quads, "collision quad indices");
    cell.quad_v3 = fourth.to_vector();

    for (std::size_t i = 0; i < faces; ++i) {
        const std::size_t corners = i < quads ? 4 : 3;
        const auto idx = cell.face_indices(i);

        for (std::size_t k = 0; k < corners; ++k) {
            if (idx[k] >= vertices) {
                fail("collision: cell ({}, {}, {}): face index out of range", x, y, z);
            }
        }
    }

    return cell;
}

void read_hero_groups(ByteView hero, CollisionMesh& mesh) {
    mesh.hero_group_count = hero.s32_at(0);

    if (mesh.hero_group_count < 0 || mesh.hero_group_count > 100'000) {
        fail("collision: implausible hero group count {}", mesh.hero_group_count);
    }

    for (s32 g = 0; g < mesh.hero_group_count; ++g) {
        const std::size_t at = 0x10 + 0x10 * static_cast<std::size_t>(g);
        HeroCollisionGroup group;

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

        for (std::size_t i = 0; i < nv; ++i) {
            const std::size_t v = data + 8 * i;

            if (hero.u16_at(v + 6) != 0) {
                fail("collision: unknown hero vertex variant (non-zero pad)");
            }

            group.vertices.push_back({hero.u16_at(v), hero.u16_at(v + 2), hero.u16_at(v + 4)});
        }

        for (std::size_t i = 0; i < nt; ++i) {
            const std::size_t t = data + 8 * nv + 4 * i;
            const std::array<u8, 3> v{hero.u8_at(t), hero.u8_at(t + 1), hero.u8_at(t + 2)};

            for (u8 index : v) {
                if (index >= nv) {
                    fail("collision: hero group index out of range");
                }
            }

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

    if (i < node.count) {
        return i;
    }

    return std::nullopt;
}

}  // namespace

CollisionMesh read_collision(Game game, ByteView block) {
    require_world_layout(game, "collision");
    CollisionMesh mesh;
    mesh.mesh_offset = block.s32_at(0);
    mesh.hero_offset = block.s32_at(4);

    if (mesh.mesh_offset <= 0) {
        fail("collision: no mesh offset");
    }

    if (mesh.hero_offset != 0 && mesh.hero_offset < mesh.mesh_offset) {
        fail("collision: the hero section comes before the mesh");
    }

    // The mesh runs to the hero section, or to the end of the block.
    const std::size_t start = static_cast<std::size_t>(mesh.mesh_offset);
    const std::size_t end =
        mesh.hero_offset > 0 ? static_cast<std::size_t>(mesh.hero_offset) : block.size();
    const ByteView m = block.sub(start, end > start ? end - start : 0, "collision mesh");

    mesh.root = checked_node(m, 0, 2, "collision z");

    for (std::size_t zi = 0; zi < mesh.root.count; ++zi) {
        mesh.root_entries.push_back(m.u16_at(4 + 2 * zi));
    }

    for (std::size_t zi = 0; zi < mesh.root_entries.size(); ++zi) {
        if (mesh.root_entries[zi] == 0) {
            continue;
        }

        const auto z = static_cast<s16>(mesh.root.base + static_cast<s16>(zi));
        CollisionSlab slab;
        slab.offset = u32{mesh.root_entries[zi]} * 4;
        slab.z = z;
        slab.node = checked_node(m, slab.offset, 4, "collision y");

        for (std::size_t yi = 0; yi < slab.node.count; ++yi) {
            slab.rows.push_back(m.u32_at(slab.offset + 4 + 4 * yi));
        }

        for (std::size_t yi = 0; yi < slab.rows.size(); ++yi) {
            if (slab.rows[yi] == 0) {
                continue;
            }

            CollisionRow row;
            row.offset = slab.rows[yi];
            row.z = z;
            row.y = static_cast<s16>(slab.node.base + static_cast<s16>(yi));
            row.node = checked_node(m, row.offset, 4, "collision x");

            for (std::size_t xi = 0; xi < row.node.count; ++xi) {
                row.leaves.push_back(m.u32_at(row.offset + 4 + 4 * xi));
            }

            for (std::size_t xi = 0; xi < row.leaves.size(); ++xi) {
                if (row.leaves[xi] != 0) {
                    const auto x = static_cast<s16>(row.node.base + static_cast<s16>(xi));
                    mesh.cells.push_back(read_leaf(m, x, row.y, z, row.leaves[xi]));
                }
            }

            mesh.rows.push_back(std::move(row));
        }

        mesh.slabs.push_back(std::move(slab));
    }

    if (mesh.hero_offset > 0) {
        read_hero_groups(
            block.tail(static_cast<std::size_t>(mesh.hero_offset), "hero collision"), mesh
        );
    }

    return mesh;
}

std::optional<u32> lookup_collision_leaf(ByteView mesh, s32 x, s32 y, s32 z) {
    const auto zi = entry_index(read_node(mesh, 0), z);

    if (!zi) {
        return std::nullopt;
    }

    const std::size_t slab = std::size_t{mesh.u16_at(4 + 2 * *zi)} * 4;

    if (slab == 0) {
        return std::nullopt;
    }

    const auto yi = entry_index(read_node(mesh, slab), y);

    if (!yi) {
        return std::nullopt;
    }

    const std::size_t row = mesh.u32_at(slab + 4 + 4 * *yi);

    if (row == 0) {
        return std::nullopt;
    }

    const auto xi = entry_index(read_node(mesh, row), x);

    if (!xi) {
        return std::nullopt;
    }

    const u32 word = mesh.u32_at(row + 4 + 4 * *xi);

    if (word == 0) {
        return std::nullopt;
    }

    return word;
}

std::optional<std::array<s32, 3>> collision_cell_of(const std::array<float, 3>& p) {
    std::array<s32, 3> cell{};

    for (std::size_t k = 0; k < 3; ++k) {
        if (!(p[k] >= 0.0f && p[k] < kCollisionWorldSize)) {
            return std::nullopt;
        }

        cell[k] = static_cast<s32>(p[k] * 1024.0f) >> 12;
    }

    return cell;
}

std::vector<CollisionTriangle> collision_triangles(const CollisionMesh& mesh) {
    std::vector<CollisionTriangle> out;

    for (std::size_t ci = 0; ci < mesh.cells.size(); ++ci) {
        const CollisionCell& cell = mesh.cells[ci];

        for (std::size_t fi = 0; fi < cell.faces.size(); ++fi) {
            const auto v = cell.face_indices(fi);
            const bool quad = cell.is_quad(fi);

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
