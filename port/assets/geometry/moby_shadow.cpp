// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/geometry/moby_shadow.h"

namespace openrac::assets::rac1 {

namespace {

std::array<f32, 4> vec4_at(ByteView b, std::size_t o) {
    return {b.f32_at(o), b.f32_at(o + 4), b.f32_at(o + 8), b.f32_at(o + 12)};
}

}  // namespace

ShadowBlock parse_shadow_block(ByteView block) {
    ShadowBlock s;
    std::size_t at = 0;

    while (true) {
        const u8 type = block.u8_at(at);
        const u8 last = block.u8_at(at + 1);
        const u16 size = block.u16_at(at + 2);
        std::size_t want = 0;

        if (type == 0) {
            s.primitives.push_back(
                ShadowSphere{block.s32_at(at + 4), block.s32_at(at + 8), vec4_at(block, at + 0x10)}
            );
            want = 0x20;
        } else if (type == 1) {
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
            fail("moby shadow: record {} at {:#x}: type {}", s.primitives.size(), at, type);
        }

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

        if (last != 0) {
            break;
        }
    }

    if (at != block.size()) {
        fail("moby shadow: last record ends at {:#x} in a {:#x}-byte block", at, block.size());
    }

    return s;
}

std::optional<ShadowBlock> moby_class_shadow(ByteView class_blob) {
    const std::size_t qw = class_blob.u8_at(0xf);

    if (qw == 0) {
        return std::nullopt;
    }

    const s32 skeleton = class_blob.s32_at(0x14);

    if (skeleton < 0 || static_cast<std::size_t>(skeleton) < 16 * qw) {
        fail("moby shadow: {} quadwords before the skeleton at {:#x}", qw, skeleton);
    }

    const std::size_t start = static_cast<std::size_t>(skeleton) - 16 * qw;
    return parse_shadow_block(class_blob.sub(start, 16 * qw, "moby shadow block"));
}

std::vector<LevelMobyShadow> parse_level_moby_shadows(
    std::span<const CoreClassEntry> classes, ByteView core_data
) {
    std::vector<LevelMobyShadow> out;

    for (const CoreClassEntry& e : classes) {
        if (e.offset <= 0) {
            continue;
        }

        try {
            const ByteView blob = core_data.tail(static_cast<std::size_t>(e.offset), "moby class");

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
