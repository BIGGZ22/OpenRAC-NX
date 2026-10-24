// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The moby shadow readers (moby_shadow.h). A class's shadow is a block of records (spheres and
// capsules) in the quadwords just before its skeleton, the last record carrying a flag. The block
// is parsed record by record, and each record's size and the block's total are checked so that a
// corrupt class throws an AssetError instead of misreading its neighbours.

#include "assets/geometry/moby_shadow.h"

namespace openrac::assets::rac1 {

namespace {

// Four consecutive floats at a byte offset.
std::array<f32, 4> vec4_at(ByteView b, std::size_t o) {
    return {b.f32_at(o), b.f32_at(o + 4), b.f32_at(o + 8), b.f32_at(o + 12)};
}

}  // namespace

// Parses a whole shadow block, which must be exactly filled by its records.
ShadowBlock parse_shadow_block(ByteView block) {
    ShadowBlock s;
    std::size_t at = 0;

    // Read records until one carries the last flag.
    while (true) {
        const u8 type = block.u8_at(at);
        const u8 last = block.u8_at(at + 1);
        const u16 size = block.u16_at(at + 2);
        std::size_t want = 0;

        // Type 0 is a sphere, 0x20 bytes.
        if (type == 0) {
            s.primitives.push_back(
                ShadowSphere{block.s32_at(at + 4), block.s32_at(at + 8), vec4_at(block, at + 0x10)}
            );
            want = 0x20;
        } else if (type == 1) {
            // Type 1 is a capsule, 0x30 bytes.
            s.primitives.push_back(
                ShadowCapsule{
                    {block.u16_at(at + 4), block.u16_at(at + 6)},
                    {block.s32_at(at + 8), block.s32_at(at + 0xc)},
                    vec4_at(block, at + 0x10),
                    vec4_at(block, at + 0x20)
                }
            );
            want = 0x30;
        } else {
            // Any other type is not a shadow primitive, so the block is corrupt.
            fail("moby shadow: record {} at {:#x}: type {}", s.primitives.size(), at, type);
        }

        // The record's own size field must agree with its type, or the next record would be
        // misplaced.
        if (size != want) {
            fail(
                "moby shadow: record {} at {:#x}: size {:#x} for type {}",
                s.primitives.size() - 1,
                at,
                size,
                type
            );
        }

        block.check(at, want, "moby shadow record");
        at += want;

        // The last flag ends the list.
        if (last != 0) {
            break;
        }
    }

    // The records must fill the block exactly, or something was left unread.
    if (at != block.size()) {
        fail("moby shadow: last record ends at {:#x} in a {:#x}-byte block", at, block.size());
    }

    return s;
}

// The shadow block of a class blob, or nothing when the class has none. The class header stores
// the block's length in quadwords; the block ends where the skeleton starts.
std::optional<ShadowBlock> moby_class_shadow(ByteView class_blob) {
    const std::size_t qw = class_blob.u8_at(0xf);

    // A class with no shadow quadwords has no shadow block.
    if (qw == 0) {
        return std::nullopt;
    }

    const s32 skeleton = class_blob.s32_at(0x14);

    // The block sits just before the skeleton, so the skeleton must be far enough in to hold it.
    if (skeleton < 0 || static_cast<std::size_t>(skeleton) < 16 * qw) {
        fail("moby shadow: {} quadwords before the skeleton at {:#x}", qw, skeleton);
    }

    const std::size_t start = static_cast<std::size_t>(skeleton) - 16 * qw;
    return parse_shadow_block(class_blob.sub(start, 16 * qw, "moby shadow block"));
}

// The shadow block of every class of a level that has one, tagged with its class number.
std::vector<LevelMobyShadow> parse_level_moby_shadows(
    std::span<const CoreClassEntry> classes, ByteView core_data
) {
    std::vector<LevelMobyShadow> out;

    // Classes without a blob offset (gadgets, empty slots) have no shadow data to read.
    for (const CoreClassEntry& e : classes) {
        // An offset of 0 or less means the class has no blob in the core data.
        if (e.offset <= 0) {
            continue;
        }

        // A malformed class must be reported by class number, so wrap its errors.
        try {
            const ByteView blob = core_data.tail(static_cast<std::size_t>(e.offset), "moby class");

            // A class without a shadow block is not an error, it simply adds nothing.
            if (auto s = moby_class_shadow(blob)) {
                out.push_back({e.o_class, blob.u8_at(8), std::move(*s)});
            }
        } catch (const AssetError& error) {
            fail("moby class {} shadow: {}", e.o_class, error.what());
        }
    }

    return out;
}

}  // namespace openrac::assets::rac1
