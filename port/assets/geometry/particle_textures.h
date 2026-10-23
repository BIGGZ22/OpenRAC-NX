// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/particle_tex.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// A RAC1 level's particle textures, particle frame lists (part_defs) and FX
// textures (ReRAC docs/formats/textures_rac1.md 8, docs/plan/particles.md 6).
// At level load the game (ParseParticleTexs, NTSC-U boot 0x2026c8) reads three
// core index tables:
//
//   part_textures  core header +0x50 count / +0x54 index offset, 0x10-byte
//                  PartTextureEntry records;
//   part_defs      core header +0x6c, one frame list per particle type;
//   the part bank  core header +0x64 (core data offset): every palette and
//                  pixel block the entries point at.
//
// FX textures come from +0x58 / +0x5c and the FX bank (+0x68). Every texture
// is 8-bit indexed with its own 256-entry palette (CSM1, alpha 0x80 = 1.0) and
// decodes like a level texture.

#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/texture.h"

namespace openrac::assets::rac1 {

// Particle types: entries of the update table and of part_defs.
constexpr std::size_t kParticleTypes = 81;

// One part_textures entry (0x10 bytes).
struct PartTextureEntry {
    s32 palette = 0;  // 0x0: palette byte offset in the part bank (1024 bytes, CSM1)
    // 0x4: the palette slot, TEX0.CSA; the upload sends 0x400 - csa * 0x100
    // palette bytes (0 on every RAC1 level).
    s32 csa = 0;
    s32 texture = 0;  // 0x8: pixel byte offset in the part bank (side * side bytes)
    s32 side = 0;     // 0xc: edge length of the square texture (32 on every RAC1 level)

    // The two words ParseParticleTexs stores per entry for a part bank at EE
    // address `bank`: lo = (bank + palette) * 16 + csa, hi = (bank + texture)
    // * 16 + log2(side). The particle code unpacks the palette address (lo >>
    // 4), the CSA (lo & 0xf) and the image (hi >> 4).
    std::pair<u32, u32> runtime_words(u32 bank) const;
};

static_assert(sizeof(PartTextureEntry) == 0x10);

// One fx_textures entry (0x10 bytes); all -1 when absent (every RAC1 entry).
struct FxTextureEntry {
    s32 palette = 0;  // palette byte offset in the FX bank
    s32 texture = 0;  // pixel byte offset in the FX bank
    s32 width = 0;
    s32 height = 0;

    bool present() const { return width > 0 && height > 0 && palette >= 0 && texture >= 0; }
};

static_assert(sizeof(FxTextureEntry) == 0x10);

// part_defs: a header {count (81), texture count, data offset, data size},
// `count` offsets relative to part_defs, and at the data offset a blob of
// part_textures indices. The game keeps no frame counts: each particle update
// function knows how many frames its type animates through, and a spawner
// takes the first.
struct PartDefs {
    std::array<s32, 4> header{};
    std::vector<s32> offsets;  // per type, relative to part_defs; 0 = null
    std::vector<u8> blob;      // the index blob the game copies whole

    // The index in `blob` of a type's frame list, by the game's rule (a null
    // offset points at the blob's start).
    std::optional<std::size_t> start(std::size_t type) const;

    // The first frame, what every spawner reads.
    std::optional<u8> first_frame(std::size_t type) const;

    // A type's frames up to the next distinct list start (Wrench's reading of
    // the table; the game only indexes from start()). Null types share the
    // run at the blob's start.
    std::vector<u8> frames(std::size_t type) const;
};

// Everything parse_particle_textures reads for a level: the entries, their decoded images, the
// frame lists and the FX textures.
struct ParticleTextures {
    std::vector<PartTextureEntry> entries;
    std::vector<RgbaImage> textures;  // entries[i] decoded, side x side
    PartDefs defs;
    std::vector<FxTextureEntry> fx_entries;
    std::vector<std::optional<RgbaImage>> fx_textures;  // empty for an absent entry
};

// part_defs at `offset` in the core index.
PartDefs parse_part_defs(ByteView index, s32 offset);

// The indices and palette of one bank image.
IndexedImage bank_texture_image(ByteView bank, s32 palette, s32 texture, s32 width, s32 height);
RgbaImage decode_bank_texture(ByteView bank, s32 palette, s32 texture, s32 width, s32 height);

// Where the particle tables of a level are: the core header's {count, offset}
// pairs (index-relative) and the two banks sliced from the core data.
struct ParticleTableRefs {
    s32 part_texture_count = 0;
    s32 part_texture_offset = 0;
    s32 fx_texture_count = 0;
    s32 fx_texture_offset = 0;
    s32 part_defs_offset = 0;
};

// Reads the tables named by `refs` from the core index and decodes each texture from its bank.
ParticleTextures parse_particle_textures(
    ByteView index, const ParticleTableRefs& refs, ByteView part_bank, ByteView fx_bank
);

}  // namespace openrac::assets::rac1
