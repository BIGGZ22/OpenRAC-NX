// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * A sound effect while it plays: the state of one run through a sound's script.
 *
 * The library advances every playing sound 240 times a second. On each tick a sound counts down
 * the delay of its next step and runs the steps that are due: starting voices, waiting, looping,
 * branching to another sound, moving its volume, pan and pitch with up to four modulators.
 *
 * Sources: adapted from the 989snd reimplementation in jak-project (ISC, see
 * THIRD_PARTY_NOTICES.md): its sound handler, its step functions and its modulators.
 */

#pragma once

#include <array>
#include <memory>
#include <vector>

#include "snd/bank.h"
#include "snd/tuning.h"

namespace snd {

class Player;

/** A voice a sound started: where it is in the player, and which use of that place it was. */
struct VoiceUse {
    unsigned slot = 0;  // Index of the voice in the player.
    u32 serial = 0;     // The player's count of voice starts when this one began.
};

/**
 * What a sound is started with.
 */
struct SoundStart {
    /** For `volume`: leave the volume as the caller last set it (full, for a new sound). */
    static constexpr s32 kKeepVolume = 0x7FFFFFFF;

    /** For `pan`: the sound's own pan from the bank. */
    static constexpr s32 kOwnPan = -1;

    /** For `pan`: leave the pan as it is (the sound's own, for a new sound). */
    static constexpr s32 kKeepPan = -2;

    Bank* bank = nullptr;              // The bank the sound is in; it outlives the sound.
    u32 index = 0;                     // The sound's index in the bank.
    s32 sound_volume = -1;             // Replaces the bank's volume for the sound; -1 keeps it.
    s32 sound_pan = -1;                // Replaces the bank's pan for the sound; -1 keeps it.
    s32 volume = 1024;                 // The caller's volume, 1,024 for full.
    s32 pan = kOwnPan;                 // The caller's pan in degrees, or one of the two above.
    s32 pitch_modifier = 0;            // An offset in 128ths of a semitone.
    s32 pitch_bend = 0;                // -0x8000 to 0x7FFF.
    std::array<s8, 4> registers = {};  // The sound's four counters to start with.
    u32 start_tick = 0;                // The player's tick when it started, for the age limit.
};

/**
 * One low-frequency modulator of a playing sound.
 */
struct Modulator {
    LfoSettings settings;    // As the script's step set them; shape 0 is off.
    s32 next_step = 0;       // Position in the wave: steps of 2,048 a turn, in 65,536ths.
    s32 range = 0;           // How far it moves its target at full height.
    s32 hold = 0;            // The square's flip step, or the random shape's current height.
    bool high_half = false;  // The random shape: the wave is in its second half.
    u32 ticks = 0;           // Ticks seen; it moves its target on every other one.
};

/**
 * A sound that is playing.
 *
 * Only the thread that mixes sound uses it. It lives in its player, which it calls for voices,
 * random numbers and the registers all sounds share.
 */
class PlayingSound {
public:
    /**
     * Starts a sound and runs the steps that are due at once.
     *
     * @param player The player it belongs to.
     * @param start What to play and how.
     */
    PlayingSound(Player& player, const SoundStart& start);

    /** Silences the voices the sound still has. */
    ~PlayingSound();

    PlayingSound(const PlayingSound&) = delete;
    PlayingSound& operator=(const PlayingSound&) = delete;

    /**
     * Advances the sound by one tick.
     *
     * @return True when the sound is over: its script has ended and its voices have died away.
     */
    bool tick();

    /** Ends the script and releases the voices, so that they die away. */
    void stop();

    /**
     * Pauses or continues the sound, its children and its voices.
     *
     * @param paused True to pause.
     */
    void set_paused(bool paused);

    /**
     * Changes the caller's volume and pan.
     *
     * @param volume The new volume, 1,024 for full; `SoundStart::kKeepVolume` leaves it; a
     *     negative value is a volume of 0 to 127 negated.
     * @param pan The new pan in degrees, or `SoundStart::kOwnPan` or `SoundStart::kKeepPan`.
     */
    void set_volume_pan(s32 volume, s32 pan);

    /**
     * Changes the caller's pitch modifier.
     *
     * @param modifier An offset in 128ths of a semitone.
     */
    void set_pitch_modifier(s32 modifier);

    /**
     * Changes the caller's pitch bend.
     *
     * @param bend -0x8000 to 0x7FFF.
     */
    void set_pitch_bend(s32 bend);

    /** The volume group of the sound it is playing now. */
    u32 group() const { return group_; }

    /** The bank the sound is in. */
    const Bank* bank() const { return bank_; }

    /** The sound of the bank it was started as, before any branch. */
    const Sfx* started_as() const { return started_as_; }

    /** The sound of the bank it is playing now. */
    const Sfx* playing_as() const { return sfx_; }

    /** The caller's volume, 1,024 for full. */
    s32 caller_volume() const { return caller_volume_; }

    /** The player's tick when it started. */
    u32 start_tick() const { return start_tick_; }

private:
    /** Runs the next step and sets the countdown to the one after it. */
    void run_next_grain();

    /**
     * Runs one step.
     *
     * @param grain The step. Two kinds keep what they picked in its parameters.
     * @return Extra ticks to wait before the next step.
     */
    s32 run_grain(Grain& grain);

    /** The steps, by family. Each returns extra ticks to wait. */
    s32 run_tone(const Grain& grain);
    s32 run_lfo_settings(const Grain& grain);
    s32 run_start_child(const Grain& grain);
    s32 run_stop_child(const Grain& grain);
    s32 run_branch(const Grain& grain);
    s32 run_loop(const Grain& grain);
    s32 run_pick(Grain& grain);
    s32 run_pitch_bend(const Grain& grain);
    s32 run_register(const Grain& grain);
    s32 run_marker(const Grain& grain);
    s32 run_voices(const Grain& grain);

    /**
     * Resolves a volume or pan a step names indirectly.
     *
     * @param value 0 or more: the value itself. -1 to -4: one of the sound's registers. -5: a
     *     random value. -6 and below: one of the registers all sounds share.
     * @param random_range For -5: one more than the largest random value.
     * @return The value; a register's content as it is.
     */
    s32 resolve(s32 value, s32 random_range);

    /**
     * Gives the register a step names.
     *
     * @param which 0 to 3: one of the sound's own. Negative: one all sounds share.
     * @return The register, or a spare one when the number is out of range.
     */
    s8& register_at(s32 which);

    /** Works out the volume and pan now and gives every voice its levels. */
    void update_levels();

    /** Works out the pitch now and gives it to every voice. */
    void update_pitch();

    /** Forgets voices that have ended or been taken for another sound. */
    void drop_ended_voices();

    /** Advances the modulators by one tick and applies what they moved. */
    void tick_modulators();

    /**
     * Gives a modulator's height now and moves it on.
     *
     * @param modulator The modulator.
     * @return The height, -32,767 to 32,767.
     */
    s32 modulator_height(Modulator& modulator);

    Player& player_;             // The player the sound belongs to.
    Bank* bank_ = nullptr;       // The bank the sound is in; not owned.
    Sfx* sfx_ = nullptr;         // The sound being played now.
    Sfx* started_as_ = nullptr;  // The sound it was started as.
    u32 start_tick_ = 0;         // The player's tick when it started.
    u32 group_ = 0;              // The volume group of `sfx_`.
    bool paused_ = false;        // The script and the voices stand still.
    bool done_ = false;          // The script has ended.

    s32 next_grain_ = 0;      // Index of the next step.
    s32 countdown_ = 0;       // Ticks until it runs.
    bool skipping_ = false;   // A pick step is playing one group of several.
    s32 grains_to_play_ = 0;  // Steps of that group still to run.
    s32 grains_to_skip_ = 0;  // Steps of the other groups to jump over afterwards.

    s32 sound_volume_ = 0;           // The sound's volume from the bank (or its parent), 0 to 127.
    s32 caller_volume_ = 1024;       // The caller's volume, 1,024 for full.
    s32 caller_pan_ = 0;             // The caller's pan in degrees.
    s32 caller_pitch_modifier_ = 0;  // The caller's pitch modifier.
    s32 caller_pitch_bend_ = 0;      // The caller's pitch bend.

    s32 volume_ = 0;          // The volume in force, 0 to 127.
    s32 pan_ = 0;             // The pan in force, 0 to 359.
    s32 pitch_modifier_ = 0;  // The pitch modifier in force.
    s32 pitch_bend_ = 0;      // The pitch bend in force.

    s32 lfo_volume_ = 0;          // What the modulators add to the volume.
    s32 lfo_pan_ = 0;             // What they add to the pan.
    s32 lfo_pitch_modifier_ = 0;  // What they add to the pitch modifier.
    s32 lfo_pitch_bend_ = 0;      // What they add to the pitch bend.

    Note note_{60, 0};                                     // The note tones are played at.
    std::array<s8, 4> registers_ = {};                     // The sound's own counters.
    s8 spare_register_ = 0;                                // Stands in for a register out of range.
    std::array<Modulator, 4> modulators_;                  // The four modulators.
    std::vector<VoiceUse> voices_;                         // The voices it started.
    std::vector<std::unique_ptr<PlayingSound>> children_;  // The sounds it started.
};

}  // namespace snd
