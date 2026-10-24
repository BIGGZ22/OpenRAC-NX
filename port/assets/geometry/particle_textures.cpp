// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/particle_tex.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The particle readers (particle_textures.h). The core index holds three tables, particle textures,
// FX textures and the particle frame lists, and the pixel data lives in two separate banks. This
// file reads the tables, bounds-checks every offset into them, and decodes each texture from its
// bank. The frame lists have no stored length, so the end of one is found from the next one.

#include "assets/geometry/particle_textures.h"

#include <algorithm>
#include <bit>

namespace openrac::assets::rac1 {

// The two words the game builds for the GS from an entry: the palette word in `lo` and the
// texture word in `hi`, each an address in `bank` times 16 plus a small field.
std::pair<u32, u32> PartTextureEntry::runtime_words(u32 bank) const {
    // The exponent of a power of two (the TW / TH field), as the boot helper
    // at NTSC-U 0x1f97a0 computes it.
    const u32 log2_side =
        side > 0 ? 31u - static_cast<u32>(std::countl_zero(static_cast<u32>(side))) : 0u;
    const u32 lo = (bank + static_cast<u32>(palette)) * 16u + static_cast<u32>(csa);
    const u32 hi = (bank + static_cast<u32>(texture)) * 16u + log2_side;
    return {lo, hi};
}

// The byte offset of a type's frame list in the blob, or nothing when the type, or the offset
// stored for it, is out of range.
std::optional<std::size_t> PartDefs::start(std::size_t type) const {
    // An unknown particle type has no frame list.
    if (type >= offsets.size()) {
        return std::nullopt;
    }

    const s32 offset = offsets[type];
    s64 s = 0;

    // An offset of 0 means the list starts at the head of the blob; otherwise it is stored relative
    // to the blob's base address.
    if (offset != 0) {
        s = s64{offset} - header[2];

        // An offset before the blob's base cannot point inside it.
        if (s < 0) {
            return std::nullopt;
        }
    }

    // An offset past the end of the blob points at nothing.
    if (static_cast<std::size_t>(s) >= blob.size()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(s);
}

// The first frame of a type's list, which names the particle's initial texture.
std::optional<u8> PartDefs::first_frame(std::size_t type) const {
    const auto s = start(type);

    // A type without a frame list has no first frame.
    if (!s) {
        return std::nullopt;
    }

    return blob[*s];
}

// A type's whole frame list, up to the start of the next list in the blob (or its end).
std::vector<u8> PartDefs::frames(std::size_t type) const {
    const auto s = start(type);

    // A type without a frame list has no frames.
    if (!s) {
        return {};
    }

    std::size_t end = blob.size();

    // The lists are packed one after another, so a list ends where the next one that follows it
    // starts.
    for (std::size_t t = 0; t < offsets.size(); ++t) {
        const auto other = start(t);

        // Only lists that start after this one can bound it.
        if (other && *other > *s) {
            end = std::min(end, *other);
        }
    }

    return {
        blob.begin() + static_cast<std::ptrdiff_t>(*s),
        blob.begin() + static_cast<std::ptrdiff_t>(end)
    };
}

// Reads the part_defs block at `offset` in the index: its header, one offset per type, and the
// blob of frame lists. Throws on an implausible count or range.
PartDefs parse_part_defs(ByteView index, s32 offset) {
    // A level with no part_defs offset has no particle data to read.
    if (offset <= 0) {
        fail("level has no part_defs");
    }

    const auto base = static_cast<std::size_t>(offset);
    PartDefs defs;
    defs.header = index.read<std::array<s32, 4>>(base, "part_defs header");
    const auto [count, texture_count, data_offset, data_size] = defs.header;
    (void)texture_count;

    // Reject a count that cannot be real before it sizes the offset table read.
    if (count < 0 || count > 0x100) {
        fail("part_defs: implausible count {}", count);
    }

    // The blob range comes from disc data, so a negative start or size is corruption.
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

// A view of one 8-bit texture of a bank with its 256-entry palette (1024 bytes), undecoded.
IndexedImage bank_texture_image(ByteView bank, s32 palette, s32 texture, s32 width, s32 height) {
    // A bank texture needs pixels, a palette and a size to be decodable.
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

// The same texture decoded to RGBA.
RgbaImage decode_bank_texture(ByteView bank, s32 palette, s32 texture, s32 width, s32 height) {
    return bank_texture_image(bank, palette, texture, width, height).decode();
}

// Reads all the particle tables of a level and decodes every texture from the two banks.
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

    // Decode each particle texture, which is square (`side` wide and high).
    for (const PartTextureEntry& e : out.entries) {
        out.textures.push_back(
            decode_bank_texture(part_bank, e.palette, e.texture, e.side, e.side)
        );
    }

    // Keep the FX table's positions: an absent entry becomes an empty slot, not a skipped one.
    for (const FxTextureEntry& e : out.fx_entries) {
        // An FX entry that is not present has no pixels to decode.
        if (e.present()) {
            out.fx_textures.emplace_back(
                decode_bank_texture(fx_bank, e.palette, e.texture, e.width, e.height)
            );
        } else {
            // An absent FX texture still takes its slot in the list.
            out.fx_textures.emplace_back(std::nullopt);
        }
    }

    return out;
}

}  // namespace openrac::assets::rac1
