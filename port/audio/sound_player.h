// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Behaviour after OpenGOAL's reimplementation of the same library
// (https://github.com/open-goal/jak-project, game/sound/989snd: the block
// sound handler, its grains, the voice manager and the LFOs; ISC License,
// Copyright (c) 2020-2026 OpenGOAL Team), adapted to the banks of RAC1 with
// ReRAC's notes on them (https://github.com/re-rac/rerac; ISC License,
// Copyright (c) 2026 ReRAC contributors).
//
// The sound player the games' sound library (989snd) drives: what the
// library's IOP side did, on the PC. A bank is loaded once; playing one of
// its sounds starts a script of grains that runs at 240 Hz and keys tones
// onto the mixer's voices (spu.h) at the pitch, volume and pan the script
// and the caller ask for. The game's commands (port/game/common/lib/snd.c)
// map one for one onto the calls below.
//
// Not played yet, as in RAC1's banks they never occur: cross-bank references
// (grain types 2, 3), child sounds and branches (5, 6, 8), plugin messages
// (7), noise and reverb-only tones. They are counted (unsupported()).

#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "assets/sound/sound_bank.h"
#include "audio/spu.h"

namespace openrac::audio {

// Handles are never 0: 0 is what play() and the lookups return for "none".
using BankHandle = std::uint32_t;
using SoundHandle = std::uint32_t;

// The library's sentinels: keep the volume, reset or keep the pan.
inline constexpr std::int32_t kVolumeKeep = 0x7fffffff;
inline constexpr std::int32_t kPanReset = -1;
inline constexpr std::int32_t kPanKeep = -2;

// Script ticks: 240 a second, 200 output frames each.
inline constexpr int kTicksPerSecond = 240;
inline constexpr int kFramesPerTick = kOutputRate / kTicksPerSecond;

// Master volume groups, 0x400 = full.
inline constexpr std::size_t kVolumeGroups = 16;
inline constexpr std::int32_t kFullVolume = 0x400;

// How a play starts: the caller's volume, pan, pitch and the sound's four registers.
struct PlayRequest {
    std::int32_t volume = kVolumeKeep;  // 1024 = the sound's own volume
    std::int32_t pan = kPanReset;       // degrees, or a sentinel
    std::int32_t pitch_mod = 0;         // 1/128 semitones
    std::int32_t pitch_bend = 0;        // -0x8000..0x7fff over the tone's bend range
    std::array<std::int8_t, 4> registers{};
};

// The left and right volume a tone starts at, before its group: 989snd's
// MakeVolume (three volumes 0..127 multiplied, three pans in degrees added,
// the sum placed on a constant-power curve).
std::array<std::int16_t, 2> make_volume(int vol1, int pan1, int vol2, int pan2, int vol3, int pan3);

// A volume scaled by a group's master volume (square law, as the library).
std::int16_t group_volume(std::int16_t volume, std::int32_t master);

// The player: owns the loaded banks, the playing sounds and the mixer their tones go to. It is not
// thread-safe: the caller serialises the calls and render().
class SoundPlayer {
public:
    SoundPlayer();
    ~SoundPlayer();
    SoundPlayer(const SoundPlayer&) = delete;
    SoundPlayer& operator=(const SoundPlayer&) = delete;

    // A bank file (assets/sound/sound_bank.h); throws assets::AssetError.
    BankHandle load_bank(std::span<const std::uint8_t> file);
    // Stops the bank's sounds first.
    void unload_bank(BankHandle bank);
    const assets::SoundBank* bank(BankHandle bank) const;

    // Starts sound `sound` of `bank`; 0 when there is no such sound, it has
    // no grains, or its instance limit refuses it.
    SoundHandle play(BankHandle bank, std::uint32_t sound, const PlayRequest& request = {});
    bool playing(SoundHandle sound) const;
    // Keys the sound's voices off; it ends when they have released.
    void stop(SoundHandle sound);
    void stop_all();

    void set_volume_pan(SoundHandle sound, std::int32_t volume, std::int32_t pan);
    void set_pitch_mod(SoundHandle sound, std::int32_t pitch_mod);
    void set_pitch_bend(SoundHandle sound, std::int32_t pitch_bend);
    void set_register(SoundHandle sound, std::size_t which, std::int8_t value);

    void set_master_volume(std::size_t group, std::int32_t volume);
    // Groups by bit (bit g = group g).
    void pause_groups(std::uint32_t mask);
    void continue_groups(std::uint32_t mask);

    // Renders interleaved stereo at 48 kHz, running the scripts every 200
    // frames.
    void render(std::span<std::int16_t> interleaved);
    // One script tick, without rendering (tests, and callers that mix).
    void tick();

    VoiceMixer& mixer() { return m_mixer; }

    std::size_t sounds_playing() const;

    // Grain types and tone kinds met but not played, with counts.
    const std::map<std::string, std::uint32_t>& unsupported() const { return m_unsupported; }

private:
    struct Bank;
    struct Script;
    struct Slot;

    void run_grain(Script& s);
    void start_tone(Script& s, const assets::Tone& tone);
    void apply_volume(Script& s, std::int32_t volume, std::int32_t pan);
    void apply_pitch(Script& s);
    void tick_lfos(Script& s);
    std::optional<std::size_t> allocate_voice(int priority);
    bool owns(const Script& s, std::size_t voice) const;
    std::vector<std::size_t> voices_of(const Script& s) const;
    std::int32_t register_value(const Script& s, std::int32_t selector, int scale);
    std::int32_t random();
    std::int8_t& reg(Script& s, std::int32_t which);
    void unsupported(const char* what);

    // The 48 voices and the reverb that every sound's tones are keyed onto.
    VoiceMixer m_mixer;
    std::map<BankHandle, std::unique_ptr<Bank>> m_banks;
    std::map<SoundHandle, std::unique_ptr<Script>> m_scripts;
    // Per voice: which sound owns it and the tone's settings, to recompute volume and pitch.
    std::vector<Slot> m_slots;
    // The master volume of each group, kFullVolume when never set.
    std::array<std::int32_t, kVolumeGroups> m_master{};
    // The registers every sound shares (negative selectors of the register grains).
    std::array<std::int8_t, 32> m_global_registers{};
    std::map<std::string, std::uint32_t> m_unsupported;
    // The next handles to give out, and the script ticks since the start (the age of a sound).
    BankHandle m_next_bank = 1;
    SoundHandle m_next_sound = 1;
    std::uint64_t m_ticks = 0;
    // State of the deterministic random() below.
    std::uint32_t m_random = 1;
    // Frames rendered since the last script tick.
    int m_frame_in_tick = 0;
};

}  // namespace openrac::audio
