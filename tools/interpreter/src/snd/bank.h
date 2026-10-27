// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * A bank of sound effects as the games' sound library stores them: the "SBlk" block.
 *
 * A bank file has two parts: the block, which lists sounds, and the sample data the sounds play.
 * A sound is a short script of steps ("grains"). Most steps play a sample ("tone") or wait;
 * the rest loop, branch, pick at random and keep small counters, which is how one sound varies
 * from one playing to the next.
 *
 * Sources: the layout of the block and the meaning of each step are adapted from the 989snd
 * reimplementation in jak-project (ISC, see THIRD_PARTY_NOTICES.md), checked against the banks on
 * the games' own discs.
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "ps2/types.h"

namespace snd {

using ps2::s16;
using ps2::s32;
using ps2::s8;
using ps2::u16;
using ps2::u32;
using ps2::u8;

/**
 * The kinds of step a sound's script has, by the number the bank stores.
 */
enum class GrainType : u32 {
    Null = 0,
    Tone = 1,
    XrefId = 2,
    XrefNum = 3,
    LfoSettings = 4,
    StartChildSound = 5,
    StopChildSound = 6,
    PluginMessage = 7,
    Branch = 8,
    Tone2 = 9,
    ControlNull = 20,
    LoopStart = 21,
    LoopEnd = 22,
    LoopContinue = 23,
    Stop = 24,
    RandPlay = 25,
    RandDelay = 26,
    RandPitchBend = 27,
    PitchBend = 28,
    AddPitchBend = 29,
    SetRegister = 30,
    SetRegisterRand = 31,
    IncRegister = 32,
    DecRegister = 33,
    TestRegister = 34,
    Marker = 35,
    GotoMarker = 36,
    GotoRandomMarker = 37,
    WaitForAllVoices = 38,
    PlayCycle = 39,
    AddRegister = 40,
    KeyOffVoices = 41,
    KillVoices = 42,
    OnStopMarker = 43,
    CopyRegister = 44,
};

/**
 * A sample and how to play it: the payload of a tone step.
 */
struct Tone {
    s8 priority = 0;     // Which voice gives way when none is free.
    s8 volume = 0;       // 0 to 127; negative picks a register or a random value.
    s8 center_note = 0;  // The note at which the sample plays at its own rate.
    s8 center_fine = 0;  // Fine tuning of that note, in 128ths of a semitone.
    s16 pan = 0;         // Degrees; negative picks a register or a random value.
    s8 bend_low = 0;     // Semitones a full downward pitch bend moves.
    s8 bend_high = 0;    // Semitones a full upward pitch bend moves.
    u16 adsr1 = 0;       // First envelope register.
    u16 adsr2 = 0;       // Second envelope register.
    u16 flags = 0;       // Bit 3: noise instead of a sample.
    u32 sample = 0;      // Offset of the sample's first block in the bank's sample data.
};

/**
 * The settings of one low-frequency modulator: the payload of an LFO step.
 */
struct LfoSettings {
    u8 which = 0;          // Which of a sound's four modulators.
    u8 target = 0;         // What it moves: 0 nothing, 1 volume, 2 pan, 3 pitch, 4 pitch bend.
    u8 shape = 0;          // 0 off, 1 sine, 2 square, 3 triangle, 4 saw, 5 random.
    u16 duty_cycle = 0;    // For a square: the step at which it flips, of 2,048.
    u16 depth = 0;         // How far it moves its target, in 1,024ths of the target's range.
    u16 flags = 0;         // Bit 0 inverts it; bit 1 starts it at a random point.
    u16 start_offset = 0;  // The step it starts at, of 2,048.
    u32 step_size = 0;     // Steps per tick, in 65,536ths.
};

/**
 * Another sound of the same bank to start, stop or continue as: the payload of the child and
 * branch steps.
 */
struct ChildSound {
    s32 volume = 0;  // 0 to 127; negative picks a register or a random value.
    s32 pan = 0;     // Degrees; negative picks a register or a random value.
    s32 sound = 0;   // Index of the sound in the bank; negative means "by name" (not supported).
};

/**
 * One step of a sound's script.
 */
struct Grain {
    GrainType type = GrainType::Null;  // What the step does.
    s32 delay = 0;                     // Ticks to wait before the step runs.
    Tone tone;                         // For the tone steps.
    LfoSettings lfo;                   // For the LFO step.
    ChildSound child;                  // For the child and branch steps.
    s32 amount = 0;                    // For the random delay: one more than the longest wait.
    s16 parameters[4] = {0, 0, 0, 0};  // For the control steps; some steps keep state in them.
};

/**
 * One sound of a bank.
 */
struct Sfx {
    /** Bits of `flags`: what limits how many of the sound play at once. */
    static constexpr u16 kLimitInstances = 0x08;
    static constexpr u16 kLimitByVolume = 0x10;
    static constexpr u16 kLimitByAge = 0x20;

    s8 volume = 0;              // 0 to 127.
    s8 group = 0;               // The volume group the sound belongs to.
    s16 pan = 0;                // Degrees.
    s8 instance_limit = 0;      // How many may play at once, when `flags` asks for a limit.
    u16 flags = 0;              // See the constants above.
    std::vector<Grain> grains;  // The script.
};

/**
 * A loaded bank: its sounds and the sample data their tones play.
 *
 * The sounds are not constant: two kinds of step remember what they picked last time in their own
 * parameters, as the library's do.
 */
class Bank {
public:
    /** How many bytes of a file's start `file_bytes` needs: the table of parts. */
    static constexpr std::size_t kTableBytes = 24;

    /**
     * Works out how long a bank file is from the table at its start.
     *
     * A program names a bank on the disc by its first sector alone, so the length has to come
     * from the file.
     *
     * @param table The first `kTableBytes` bytes of the file.
     * @return The file's length in bytes, or 0 when the table is not that of a two-part bank.
     */
    static std::size_t file_bytes(const u8* table);

    /**
     * Reads a bank file.
     *
     * @param file The file: a table of parts, then the "SBlk" block and the sample data.
     * @param bytes Size of `file`.
     * @return The bank, or null when the file is not a bank this model reads.
     */
    static std::unique_ptr<Bank> parse(const u8* file, std::size_t bytes);

    /** The sounds, by their index in the bank. */
    std::vector<Sfx> sounds;

    /** The sample data: ADPCM blocks, which tones point into. */
    std::vector<u8> samples;

    /** The block's format version: 1 keeps a step's payload in the step, 2 in a separate area. */
    u32 version = 0;
};

}  // namespace snd
