// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The moby collision readers (moby_collision.h). A class's collision blob is a 0x10-byte header,
// then three packed arrays: primitives (0x20 bytes each), vertices (four s16 each) and faces
// (four u8 each, of which three are vertex indices). The header gives the byte size of each array,
// and every size and index is checked so that a corrupt blob throws an AssetError rather than
// reading outside the class.

#include "assets/geometry/moby_collision.h"

#include <bit>
#include <format>

namespace openrac::assets::rac1 {

// The i-th 32-bit word of the primitive, read as a float.
f32 MobyCollisionPrimitive::f(std::size_t i) const {
    return std::bit_cast<f32>(raw.at(i));
}

// The size in bytes the blob occupies: the header plus the three arrays.
std::size_t MobyCollision::size() const {
    return 0x10 + 0x20 * primitives.size() + 8 * vertices.size() + 4 * faces.size();
}

// Parses a collision blob (starting at its header) and validates the sections and indices.
MobyCollision parse_moby_collision(ByteView blob) {
    MobyCollision c;
    c.joint_counts = {blob.u16_at(0), blob.u16_at(2)};
    const s32 prim_bytes = blob.s32_at(4);
    const s32 face_bytes = blob.s32_at(8);
    const s32 vertex_bytes = blob.s32_at(0xc);

    // The section sizes must be whole numbers of records, or the sections below would be
    // misaligned.
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

    // Wrap each raw primitive record as it is read.
    for (const auto& raw :
         blob.read_array<std::array<u32, 8>>(0x10, pb / 0x20, "moby collision primitives")) {
        c.primitives.push_back({raw});
    }

    // Only the last primitive may carry the end bit (a negative mask).
    for (std::size_t i = 0; i < c.primitives.size(); ++i) {
        // An end bit before the last record would cut the list short.
        if (c.primitives[i].mask() < 0 && i + 1 != c.primitives.size()) {
            fail("moby collision: primitive {} ends the list early", i);
        }
    }

    // A non-empty list must end with a primitive that has the end bit.
    if (!c.primitives.empty() && c.primitives.back().mask() >= 0) {
        fail("moby collision: primitive list without its end bit");
    }

    c.vertices = blob.read_array<std::array<s16, 4>>(0x10 + pb, vb / 8, "moby collision vertices");
    c.faces = blob.read_array<std::array<u8, 4>>(0x10 + pb + vb, fb / 4, "moby collision faces");

    // Every face must refer to vertices that exist.
    for (const auto& face : c.faces) {
        // A face has three vertex indices; the fourth byte is not an index.
        for (std::size_t k = 0; k < 3; ++k) {
            // An index past the vertex count would read outside the array.
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

// The collision data of a class blob, or nothing when the class has none.
std::optional<MobyCollision> moby_class_collision(ByteView class_blob) {
    const s32 offset = class_blob.s32_at(0x10);

    // An offset of 0 means the class has no collision data.
    if (offset == 0) {
        return std::nullopt;
    }

    // A negative offset cannot point into the class blob.
    if (offset < 0) {
        fail("moby collision offset {:#x}", offset);
    }

    return parse_moby_collision(
        class_blob.tail(static_cast<std::size_t>(offset), "moby collision blob")
    );
}

// The collision data of every class of a level that has some, tagged with its class number.
std::vector<LevelMobyCollision> parse_level_moby_collisions(
    std::span<const CoreClassEntry> classes, ByteView core_data
) {
    std::vector<LevelMobyCollision> out;

    // Classes without a blob offset (gadgets, empty slots) have no collision data to read.
    for (const CoreClassEntry& e : classes) {
        // An offset of 0 or less means the class has no blob in the core data.
        if (e.offset <= 0) {
            continue;
        }

        // A malformed class must be reported by class number, so wrap its errors.
        try {
            // A class without collision data is not an error, it simply adds nothing.
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
