// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/geometry/moby_collision.h"

#include <bit>
#include <format>

namespace openrac::assets::rac1 {

f32 MobyCollisionPrimitive::f(std::size_t i) const {
    return std::bit_cast<f32>(raw.at(i));
}

std::size_t MobyCollision::size() const {
    return 0x10 + 0x20 * primitives.size() + 8 * vertices.size() + 4 * faces.size();
}

MobyCollision parse_moby_collision(ByteView blob) {
    MobyCollision c;
    c.joint_counts = {blob.u16_at(0), blob.u16_at(2)};
    const s32 prim_bytes = blob.s32_at(4);
    const s32 face_bytes = blob.s32_at(8);
    const s32 vertex_bytes = blob.s32_at(0xc);

    if (prim_bytes < 0 || face_bytes < 0 || vertex_bytes < 0 || prim_bytes % 0x20 != 0
        || face_bytes % 4 != 0 || vertex_bytes % 8 != 0) {
        fail(
            "moby collision: section sizes {:#x} / {:#x} / {:#x}",
            prim_bytes,
            face_bytes,
            vertex_bytes
        );
    }

    const auto pb = static_cast<std::size_t>(prim_bytes);
    const auto fb = static_cast<std::size_t>(face_bytes);
    const auto vb = static_cast<std::size_t>(vertex_bytes);

    for (const auto& raw :
         blob.read_array<std::array<u32, 8>>(0x10, pb / 0x20, "moby collision primitives")) {
        c.primitives.push_back({raw});
    }

    for (std::size_t i = 0; i < c.primitives.size(); ++i) {
        if (c.primitives[i].mask() < 0 && i + 1 != c.primitives.size()) {
            fail("moby collision: primitive {} ends the list early", i);
        }
    }

    if (!c.primitives.empty() && c.primitives.back().mask() >= 0) {
        fail("moby collision: primitive list without its end bit");
    }

    c.vertices = blob.read_array<std::array<s16, 4>>(0x10 + pb, vb / 8, "moby collision vertices");
    c.faces = blob.read_array<std::array<u8, 4>>(0x10 + pb + vb, fb / 4, "moby collision faces");

    for (const auto& face : c.faces) {
        for (std::size_t k = 0; k < 3; ++k) {
            if (face[k] >= c.vertices.size()) {
                fail(
                    "moby collision: face {} {} {} past {} vertices",
                    face[0],
                    face[1],
                    face[2],
                    c.vertices.size()
                );
            }
        }
    }

    return c;
}

std::optional<MobyCollision> moby_class_collision(ByteView class_blob) {
    const s32 offset = class_blob.s32_at(0x10);

    if (offset == 0) {
        return std::nullopt;
    }

    if (offset < 0) {
        fail("moby collision offset {:#x}", offset);
    }

    return parse_moby_collision(
        class_blob.tail(static_cast<std::size_t>(offset), "moby collision blob")
    );
}

std::vector<LevelMobyCollision> parse_level_moby_collisions(
    std::span<const CoreClassEntry> classes, ByteView core_data
) {
    std::vector<LevelMobyCollision> out;

    for (const CoreClassEntry& e : classes) {
        if (e.offset <= 0) {
            continue;
        }

        try {
            if (auto c = moby_class_collision(
                    core_data.tail(static_cast<std::size_t>(e.offset), "moby class")
                )) {
                out.push_back({e.o_class, std::move(*c)});
            }
        } catch (const AssetError& error) {
            fail("moby class {} collision: {}", e.o_class, error.what());
        }
    }

    return out;
}

}  // namespace openrac::assets::rac1
