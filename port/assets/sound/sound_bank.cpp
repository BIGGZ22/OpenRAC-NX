// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/sound_bank.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The 989snd bank reader (sound_bank.h).

#include "assets/sound/sound_bank.h"

#include <algorithm>
#include <cstring>

namespace openrac::assets {

namespace {

constexpr std::size_t kSoundRecordSize = 12;
constexpr std::size_t kGrainRecordSize = 0x28;

template <typename T>
T grain_field(const std::array<u8, 32>& data, std::size_t offset) {
    T value;
    std::memcpy(&value, data.data() + offset, sizeof(T));
    return value;
}

}  // namespace

Tone Grain::tone() const {
    Tone t;
    t.priority = static_cast<s8>(data[0]);
    t.volume = static_cast<s8>(data[1]);
    t.center_note = static_cast<s8>(data[2]);
    t.center_fine = static_cast<s8>(data[3]);
    t.pan = grain_field<s16>(data, 4);
    t.map_low = static_cast<s8>(data[6]);
    t.map_high = static_cast<s8>(data[7]);
    t.bend_down = static_cast<s8>(data[8]);
    t.bend_up = static_cast<s8>(data[9]);
    t.adsr1 = grain_field<u16>(data, 10);
    t.adsr2 = grain_field<u16>(data, 12);
    t.flags = grain_field<u16>(data, 14);
    t.sample_offset = grain_field<u32>(data, 16);
    t.reserved = grain_field<u32>(data, 20);
    return t;
}

LfoSettings Grain::lfo() const {
    LfoSettings l;
    l.which = data[0];
    l.target = data[1];
    l.target_extra = data[2];
    l.shape = data[3];
    l.duty_cycle = grain_field<u16>(data, 4);
    l.depth = grain_field<u16>(data, 6);
    l.flags = grain_field<u16>(data, 8);
    l.start_offset = grain_field<u16>(data, 10);
    l.step_size = grain_field<u32>(data, 12);
    return l;
}

ChildSoundParams Grain::child_sound() const {
    ChildSoundParams p;
    p.volume = grain_field<s32>(data, 0);
    p.pan = grain_field<s32>(data, 4);

    // Copy the four signed register bytes that follow volume and pan.
    for (std::size_t i = 0; i < 4; ++i) {
        p.registers[i] = static_cast<s8>(data[8 + i]);
    }

    p.sound_id = grain_field<s32>(data, 12);
    return p;
}

std::array<s16, 4> Grain::params() const {
    return {
        grain_field<s16>(data, 0),
        grain_field<s16>(data, 2),
        grain_field<s16>(data, 4),
        grain_field<s16>(data, 6)
    };
}

s32 Grain::rand_delay_amount() const {
    return grain_field<s32>(data, 0);
}

std::optional<std::size_t> SoundBank::extent_at(u32 offset) const {
    const auto it = std::lower_bound(
        sample_extents.begin(), sample_extents.end(), offset, [](const SampleExtent& e, u32 o) {
            return e.offset < o;
        }
    );

    // The offset must be one of the extents found when the bank was read.
    if (it == sample_extents.end() || it->offset != offset) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(it - sample_extents.begin());
}

std::span<const Grain> SoundBank::sound_grains(std::size_t sound) const {
    const SfxSound& s = sounds.at(sound);
    return std::span<const Grain>(grains).subspan(s.first_grain, s.grain_count);
}

SoundBank parse_sound_bank(ByteView bytes) {
    SoundBank bank;
    bank.file_type = bytes.u32_at(0);
    const u32 chunk_count = bytes.u32_at(4);

    // A 989snd bank has type 3 and exactly two chunks: the block and the samples.
    if (bank.file_type != 3 || chunk_count != 2) {
        fail(
            "sound bank: type {}, {} chunks (a 989snd bank is type 3 with 2)",
            bank.file_type,
            chunk_count
        );
    }

    bank.chunks = {
        std::pair{bytes.u32_at(8), bytes.u32_at(12)}, std::pair{bytes.u32_at(16), bytes.u32_at(20)}
    };
    const ByteView block = bytes.sub(bank.chunks[0].first, bank.chunks[0].second, "SFX block");
    bank.samples =
        bytes.sub(bank.chunks[1].first, bank.chunks[1].second, "sample chunk").to_vector();

    // Chunk 0 is the sound block and starts with "SBlk".
    if (block.size() < 4 || std::memcmp(block.data(), "SBlk", 4) != 0) {
        fail("sound bank: chunk 0 is not an SBlk block");
    }

    SfxBlockHeader& h = bank.header;
    h.version = block.u32_at(0x04);
    h.flags = block.u32_at(0x08);
    h.bank_id = block.u32_at(0x0c);
    h.bank_number = block.s8_at(0x10);
    h.sound_count = block.s16_at(0x16);
    h.grain_count = block.s16_at(0x18);
    h.vag_count = block.s16_at(0x1a);
    h.first_sound = block.u32_at(0x1c);
    h.first_grain = block.u32_at(0x20);
    h.vags_in_sound_ram = block.u32_at(0x24);
    h.vag_data_size = block.u32_at(0x28);
    h.sound_ram_size = block.u32_at(0x2c);
    h.next_block = block.u32_at(0x30);

    // Only version 1 is known; it has 0x28-byte grains.
    if (h.version != 1) {
        fail("SBlk version {}: only version 1 (0x28-byte grains) is known", h.version);
    }

    // A negative count would turn into a huge unsigned one in the loops below.
    if (h.sound_count < 0) {
        fail("SBlk: negative sound count");
    }

    bank.sounds.reserve(static_cast<std::size_t>(h.sound_count));

    // Read each sound record, then its grains.
    for (std::size_t i = 0; i < static_cast<std::size_t>(h.sound_count); ++i) {
        const std::size_t o = h.first_sound + kSoundRecordSize * i;
        SfxSound s;
        s.volume = block.s8_at(o);
        s.volume_group = block.s8_at(o + 1);
        s.pan = block.s16_at(o + 2);
        s.grain_count = block.u8_at(o + 4);
        s.instance_limit = block.s8_at(o + 5);
        s.flags = block.u16_at(o + 6);
        s.first_grain_offset = block.u32_at(o + 8);
        s.first_grain = bank.grains.size();

        // Each sound's grains are stored in the grain table at its own offset.
        for (std::size_t k = 0; k < s.grain_count; ++k) {
            const std::size_t g = h.first_grain + s.first_grain_offset + kGrainRecordSize * k;
            Grain grain;
            grain.type = static_cast<GrainType>(block.u32_at(g));
            grain.delay = block.s32_at(g + 4);
            const ByteView data = block.sub(g + 8, grain.data.size(), "grain data");
            std::memcpy(grain.data.data(), data.data(), grain.data.size());
            bank.grains.push_back(grain);
        }

        bank.sounds.push_back(s);
    }

    std::vector<u32> offsets;

    // Collect the sample offsets of the tone grains; only those point at sample data.
    for (const Grain& g : bank.grains) {
        // Other grains do not name a sample.
        if (g.is_tone()) {
            offsets.push_back(g.tone().sample_offset);
        }
    }

    std::sort(offsets.begin(), offsets.end());
    offsets.erase(std::unique(offsets.begin(), offsets.end()), offsets.end());
    const ByteView samples(bank.samples);

    // Walk each distinct sample once, to find where it ends.
    for (const u32 offset : offsets) {
        bank.sample_extents.push_back(sample_extent(samples, offset));
    }

    return bank;
}

}  // namespace openrac::assets
