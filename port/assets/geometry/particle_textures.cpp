// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/particle_tex.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 particle and FX textures.

#include "assets/geometry/particle_textures.h"

#include <algorithm>
#include <bit>

namespace openrac::assets::rac1 {

std::pair<u32, u32> PartTextureEntry::runtime_words(u32 bank) const {
    // The exponent of a power of two (the TW / TH field), as the boot helper
    // at NTSC-U 0x1f97a0 computes it.
    const u32 log2_side =
        side > 0 ? 31u - static_cast<u32>(std::countl_zero(static_cast<u32>(side))) : 0u;
    const u32 lo = (bank + static_cast<u32>(palette)) * 16u + static_cast<u32>(csa);
    const u32 hi = (bank + static_cast<u32>(texture)) * 16u + log2_side;
    return {lo, hi};
}

std::optional<std::size_t> PartDefs::start(std::size_t type) const {
    if (type >= offsets.size()) {
        return std::nullopt;
    }

    const s32 offset = offsets[type];
    s64 s = 0;

    if (offset != 0) {
        s = s64{offset} - header[2];

        if (s < 0) {
            return std::nullopt;
        }
    }

    if (static_cast<std::size_t>(s) >= blob.size()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(s);
}

std::optional<u8> PartDefs::first_frame(std::size_t type) const {
    const auto s = start(type);

    if (!s) {
        return std::nullopt;
    }

    return blob[*s];
}

std::vector<u8> PartDefs::frames(std::size_t type) const {
    const auto s = start(type);

    if (!s) {
        return {};
    }

    std::size_t end = blob.size();

    for (std::size_t t = 0; t < offsets.size(); ++t) {
        const auto other = start(t);

        if (other && *other > *s) {
            end = std::min(end, *other);
        }
    }

    return {
        blob.begin() + static_cast<std::ptrdiff_t>(*s),
        blob.begin() + static_cast<std::ptrdiff_t>(end)
    };
}

PartDefs parse_part_defs(ByteView index, s32 offset) {
    if (offset <= 0) {
        fail("level has no part_defs");
    }

    const auto base = static_cast<std::size_t>(offset);
    PartDefs defs;
    defs.header = index.read<std::array<s32, 4>>(base, "part_defs header");
    const auto [count, texture_count, data_offset, data_size] = defs.header;
    (void)texture_count;

    if (count < 0 || count > 0x100) {
        fail("part_defs: implausible count {}", count);
    }

    if (data_offset < 0 || data_size < 0) {
        fail("part_defs: negative blob range");
    }

    defs.offsets =
        index.read_array<s32>(base + 0x10, static_cast<std::size_t>(count), "part_defs offsets");
    defs.blob = index
                    .sub(
                        base + static_cast<std::size_t>(data_offset),
                        static_cast<std::size_t>(data_size),
                        "part_defs blob"
                    )
                    .to_vector();
    return defs;
}

IndexedImage bank_texture_image(ByteView bank, s32 palette, s32 texture, s32 width, s32 height) {
    if (palette < 0 || texture < 0 || width <= 0 || height <= 0) {
        fail("bank texture with a negative offset or no size");
    }

    const auto w = static_cast<u32>(width);
    const auto h = static_cast<u32>(height);
    return {
        w,
        h,
        bank.sub(static_cast<std::size_t>(texture), std::size_t{w} * h, "bank texture pixels"),
        bank.sub(static_cast<std::size_t>(palette), 1024, "bank texture palette"),
    };
}

RgbaImage decode_bank_texture(ByteView bank, s32 palette, s32 texture, s32 width, s32 height) {
    return bank_texture_image(bank, palette, texture, width, height).decode();
}

ParticleTextures parse_particle_textures(
    ByteView index, const ParticleTableRefs& refs, ByteView part_bank, ByteView fx_bank
) {
    ParticleTextures out;
    out.entries = read_core_table<PartTextureEntry>(
        index, refs.part_texture_count, refs.part_texture_offset, "part_textures"
    );
    out.fx_entries = read_core_table<FxTextureEntry>(
        index, refs.fx_texture_count, refs.fx_texture_offset, "fx_textures"
    );
    out.defs = parse_part_defs(index, refs.part_defs_offset);

    for (const PartTextureEntry& e : out.entries) {
        out.textures.push_back(
            decode_bank_texture(part_bank, e.palette, e.texture, e.side, e.side)
        );
    }

    for (const FxTextureEntry& e : out.fx_entries) {
        if (e.present()) {
            out.fx_textures.emplace_back(
                decode_bank_texture(fx_bank, e.palette, e.texture, e.width, e.height)
            );
        } else {
            out.fx_textures.emplace_back(std::nullopt);
        }
    }

    return out;
}

}  // namespace openrac::assets::rac1
