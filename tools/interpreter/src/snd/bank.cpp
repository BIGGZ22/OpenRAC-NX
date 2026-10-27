// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Reading a bank of sound effects (see bank.h).
 *
 * Every offset in the file is checked: a read outside the block gives zero, so a damaged bank
 * yields silent sounds and never a read out of range.
 */

#include "snd/bank.h"

#include <algorithm>
#include <cstring>

namespace snd {
namespace {

/** "SBlk", the four characters a sound effect block starts with, as a little-endian word. */
constexpr u32 kBlockId = 0x6B6C4253;

/** The size of one step in a version 1 block: its type, its delay and 32 bytes of payload. */
constexpr u32 kGrainBytesV1 = 0x28;

/** The size of one step in a version 2 block: an opcode word and its delay. */
constexpr u32 kGrainBytesV2 = 8;

/** The size of one sound's record in the block. */
constexpr u32 kSoundBytes = 12;

/** The longest a bank file is taken to be: 64 MB, far more than the console's sound memory. */
constexpr std::size_t kMostFileBytes = std::size_t{64} << 20;

/** The most sounds and steps a bank is taken to have; more means the file is not a bank. */
constexpr u32 kMostSounds = 4096;

/**
 * A stretch of bytes that can be read at any offset.
 *
 * A read that does not fit gives zero.
 */
struct Bytes {
    const u8* data = nullptr;  // First byte; not owned.
    std::size_t size = 0;      // How many bytes there are.

    /**
     * Reads a little-endian value.
     *
     * @tparam T An integer type of 1, 2 or 4 bytes.
     * @param at Offset of its first byte.
     * @return The value, or zero when it does not lie inside the stretch.
     */
    template <typename T>
    T read(std::size_t at) const {
        // Outside the stretch.
        if (at > size || size - at < sizeof(T)) {
            return 0;
        }

        T value;
        std::memcpy(&value, data + at, sizeof(T));

        return value;
    }
};

/**
 * Reads a tone.
 *
 * @param from The bytes the tone lies in.
 * @param at Offset of the tone's first byte.
 * @return The tone.
 */
Tone read_tone(const Bytes& from, std::size_t at) {
    Tone tone;

    // The tone record: four bytes, a pan, four bytes, three registers, the sample (documented).
    tone.priority = from.read<s8>(at);
    tone.volume = from.read<s8>(at + 1);
    tone.center_note = from.read<s8>(at + 2);
    tone.center_fine = from.read<s8>(at + 3);
    tone.pan = from.read<s16>(at + 4);
    tone.bend_low = from.read<s8>(at + 8);
    tone.bend_high = from.read<s8>(at + 9);
    tone.adsr1 = from.read<u16>(at + 10);
    tone.adsr2 = from.read<u16>(at + 12);
    tone.flags = from.read<u16>(at + 14);
    tone.sample = from.read<u32>(at + 16);

    return tone;
}

/**
 * Reads the settings of a modulator.
 *
 * @param from The bytes they lie in.
 * @param at Offset of their first byte.
 * @return The settings.
 */
LfoSettings read_lfo(const Bytes& from, std::size_t at) {
    LfoSettings lfo;

    // Which, target, a spare byte, shape, then four half-words and the step size (documented).
    lfo.which = from.read<u8>(at);
    lfo.target = from.read<u8>(at + 1);
    lfo.shape = from.read<u8>(at + 3);
    lfo.duty_cycle = from.read<u16>(at + 4);
    lfo.depth = from.read<u16>(at + 6);
    lfo.flags = from.read<u16>(at + 8);
    lfo.start_offset = from.read<u16>(at + 10);
    lfo.step_size = from.read<u32>(at + 12);

    return lfo;
}

/**
 * Reads the reference to another sound.
 *
 * @param from The bytes it lies in.
 * @param at Offset of its first byte.
 * @return The reference.
 */
ChildSound read_child(const Bytes& from, std::size_t at) {
    ChildSound child;

    // Volume, pan, four register settings (not used here), the sound's index (documented).
    child.volume = from.read<s32>(at);
    child.pan = from.read<s32>(at + 4);
    child.sound = from.read<s32>(at + 12);

    return child;
}

/**
 * Fills in a step's payload from where its kind keeps it.
 *
 * @param grain The step, with its type set.
 * @param from The bytes the payload lies in.
 * @param at Offset of the payload.
 */
void read_payload(Grain& grain, const Bytes& from, std::size_t at) {
    switch (grain.type) {
        // A tone record.
        case GrainType::Tone:
        case GrainType::Tone2:
            grain.tone = read_tone(from, at);
            break;

        // A modulator's settings.
        case GrainType::LfoSettings:
            grain.lfo = read_lfo(from, at);
            break;

        // A reference to another sound of the bank.
        case GrainType::StartChildSound:
        case GrainType::StopChildSound:
        case GrainType::Branch:
            grain.child = read_child(from, at);
            break;

        default:
            // Every other kind has its payload in the step itself, read by the caller.
            break;
    }
}

/**
 * Reads one step of a version 1 block.
 *
 * @param block The block.
 * @param at Offset of the step.
 * @return The step.
 */
Grain read_grain_v1(const Bytes& block, std::size_t at) {
    Grain grain;

    // A type word, a delay word, then the payload (documented).
    grain.type = static_cast<GrainType>(block.read<u32>(at));
    grain.delay = block.read<s32>(at + 4);
    grain.amount = block.read<s32>(at + 8);

    for (unsigned n = 0; n < 4; n++) {
        grain.parameters[n] = block.read<s16>(at + 8 + n * 2);
    }

    read_payload(grain, block, at + 8);

    return grain;
}

/**
 * Reads one step of a version 2 block.
 *
 * @param block The block.
 * @param at Offset of the step.
 * @param payloads Offset of the area the larger payloads are kept in.
 * @return The step.
 */
Grain read_grain_v2(const Bytes& block, std::size_t at, u32 payloads) {
    Grain grain;

    // The opcode: the type in the top byte, three argument bytes or an offset below (documented).
    u32 opcode = block.read<u32>(at);
    u32 value = opcode & 0xFFFFFF;

    grain.type = static_cast<GrainType>(opcode >> 24);
    grain.delay = block.read<s32>(at + 4);

    // The random delay stores one less than the amount this model wants.
    grain.amount = static_cast<s32>(value) + 1;

    // The control steps take their parameters from the three argument bytes, as signed bytes.
    for (unsigned n = 0; n < 3; n++) {
        grain.parameters[n] = static_cast<s8>((opcode >> (n * 8)) & 0xFF);
    }

    read_payload(grain, block, std::size_t{payloads} + value);

    return grain;
}

/**
 * Reads the sounds of a block.
 *
 * @param block The block.
 * @param version Its format version.
 * @return The sounds, or none when the block's counts make no sense.
 */
std::vector<Sfx> read_sounds(const Bytes& block, u32 version) {
    // The block's header after its four identifying words (documented).
    u32 count = static_cast<u32>(block.read<s16>(0x16));
    u32 first_sound = block.read<u32>(0x1C);
    u32 first_grain = block.read<u32>(0x20);
    u32 payloads = version >= 2 ? block.read<u32>(0x34) : 0;
    u32 grain_bytes = version >= 2 ? kGrainBytesV2 : kGrainBytesV1;
    std::vector<Sfx> sounds;

    // Not a count a bank has.
    if (count > kMostSounds) {
        return sounds;
    }

    sounds.resize(count);

    // One record a sound: volume, group, pan, step count, limit, flags, where its steps start.
    for (u32 n = 0; n < count; n++) {
        std::size_t at = std::size_t{first_sound} + n * kSoundBytes;
        Sfx& sound = sounds[n];

        sound.volume = block.read<s8>(at);
        sound.group = block.read<s8>(at + 1);
        sound.pan = block.read<s16>(at + 2);
        sound.instance_limit = block.read<s8>(at + 5);
        sound.flags = block.read<u16>(at + 6);

        u32 grains = block.read<u8>(at + 4);
        std::size_t first = std::size_t{first_grain} + block.read<u32>(at + 8);

        for (u32 g = 0; g < grains; g++) {
            std::size_t grain_at = first + g * grain_bytes;

            // Version 2 keeps the larger payloads apart from the steps.
            if (version >= 2) {
                sound.grains.push_back(read_grain_v2(block, grain_at, payloads));
            } else {
                sound.grains.push_back(read_grain_v1(block, grain_at));
            }
        }
    }

    return sounds;
}

}  // namespace

std::size_t Bank::file_bytes(const u8* table) {
    Bytes whole{table, kTableBytes};

    // A type, the number of parts, then each part's offset and size (documented).
    u32 parts = whole.read<u32>(4);
    std::size_t block_end = std::size_t{whole.read<u32>(8)} + whole.read<u32>(12);
    std::size_t samples_end = std::size_t{whole.read<u32>(16)} + whole.read<u32>(20);
    std::size_t bytes = std::max(block_end, samples_end);

    // Not two parts, parts that lie inside the table, or more than the sound memory could hold.
    if (parts != 2 || bytes <= kTableBytes || bytes > kMostFileBytes) {
        return 0;
    }

    return bytes;
}

std::unique_ptr<Bank> Bank::parse(const u8* file, std::size_t bytes) {
    Bytes whole{file, bytes};

    // The file's table: a type, the number of parts, then each part's offset and size (documented).
    u32 parts = whole.read<u32>(4);
    u32 block_at = whole.read<u32>(8);
    u32 block_bytes = whole.read<u32>(12);
    u32 samples_at = whole.read<u32>(16);
    u32 samples_bytes = whole.read<u32>(20);

    // A sound effect bank has two parts, and its block lies inside the file.
    if (parts < 2 || std::size_t{block_at} + block_bytes > bytes) {
        return nullptr;
    }

    // The sample data runs past the end of the file.
    if (std::size_t{samples_at} + samples_bytes > bytes) {
        return nullptr;
    }

    Bytes block{file + block_at, block_bytes};

    // Some other kind of block (a music bank).
    if (block.read<u32>(0) != kBlockId) {
        return nullptr;
    }

    auto bank = std::make_unique<Bank>();

    bank->version = block.read<u32>(4);
    bank->sounds = read_sounds(block, bank->version);
    bank->samples.assign(file + samples_at, file + samples_at + samples_bytes);

    return bank;
}

}  // namespace snd
