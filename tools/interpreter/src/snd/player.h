// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The sound effect side of the games' sound library: banks, playing sounds and their voices.
 *
 * On the console the library runs on the second processor. A program loads banks, asks for
 * sounds by bank and index, and changes or stops them by the handle it was given; the library
 * advances every sound 240 times a second and the sound processor mixes the voices.
 *
 * Sources: adapted from the 989snd reimplementation in jak-project (ISC, see
 * THIRD_PARTY_NOTICES.md): its player, voice manager and instance limits.
 */

#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <map>
#include <memory>

#include "snd/bank.h"
#include "snd/sfx.h"
#include "snd/tuning.h"
#include "snd/voice.h"

namespace snd {

/**
 * What a sound gives the player to start a voice with.
 */
struct VoiceStart {
    const Tone* tone = nullptr;  // The tone; it lives in the bank.
    const Bank* bank = nullptr;  // The bank, for its sample data.
    Levels levels;               // Left and right before the group's volume.
    s32 tone_volume = 0;         // The tone's volume as resolved, 0 to 127.
    s32 tone_pan = 0;            // The tone's pan as resolved, in degrees.
    u32 group = 0;               // The sound's volume group.
    Note note;                   // The note to sound, after bend and modifier.
};

/**
 * Plays sound effects from banks and mixes their voices.
 *
 * One thread uses it: the one that runs the machine, which gives the commands and asks for the
 * mix after each field.
 */
class Player {
public:
    /** Samples a second of the mix (documented: the sound processor's rate). */
    static constexpr int kRate = 48'000;

    /** Samples between two ticks of the sounds: 240 ticks a second. */
    static constexpr int kTickFrames = 200;

    /** How many volume groups there are; group 16 is the volume of everything. */
    static constexpr unsigned kGroups = 32;

    /** How many voices can sound at once (documented: the sound processor has 48). */
    static constexpr unsigned kVoices = 48;

    /** One voice and what the sound that started it needs to change it later. */
    struct Slot {
        Voice voice;                 // The voice.
        u32 serial = 0;              // The count of voice starts when this use began.
        const Tone* tone = nullptr;  // The tone it plays; not owned.
        const Bank* bank = nullptr;  // The bank its sample is in; not owned.
        s8 priority = 0;             // The tone's priority, for when no voice is free.
        Levels levels;               // Left and right before the group's volume.
        s32 tone_volume = 0;         // The tone's volume as resolved.
        s32 tone_pan = 0;            // The tone's pan as resolved.
        u32 group = 0;               // Its volume group.
        u32 pitch = 0;               // The pitch it had before a pause.
        bool paused = false;         // Silent and still until continued.
    };

    Player();

    // --- Banks ---

    /**
     * Loads a bank.
     *
     * @param handle The number the program will name the bank by.
     * @param file The bank file.
     * @param bytes Its size.
     * @return True if it is a sound effect bank and is now loaded.
     */
    bool load_bank(u32 handle, const u8* file, std::size_t bytes);

    /**
     * Unloads a bank and ends every sound and voice that plays from it.
     *
     * @param handle The bank's number.
     */
    void unload_bank(u32 handle);

    // --- Sounds ---

    /**
     * Starts a sound.
     *
     * @param handle The number the program will name the sound by.
     * @param bank The bank's number.
     * @param start How to play it; its `bank` and `start_tick` are filled in here.
     * @return True if it started. False: no such bank or sound, or the sound's limit on how many
     *     may play at once kept it out.
     */
    bool play(u32 handle, u32 bank, SoundStart start);

    /**
     * Ends a sound's script and lets its voices die away.
     *
     * @param handle The sound's number.
     */
    void stop(u32 handle);

    /** Ends every sound at once. */
    void stop_all();

    /**
     * Says whether a sound is still playing.
     *
     * @param handle The sound's number.
     * @return True until its script has ended and its voices have died away.
     */
    bool is_playing(u32 handle) const { return sounds_.count(handle) != 0; }

    /**
     * Finds a playing sound.
     *
     * @param handle The sound's number.
     * @return The sound, or null when it is not playing.
     */
    PlayingSound* sound(u32 handle);

    /**
     * Pauses or continues every sound of some groups.
     *
     * @param groups One bit a group.
     * @param paused True to pause.
     */
    void set_groups_paused(u32 groups, bool paused);

    /**
     * Sets the volume of a group.
     *
     * @param group The group; 16 is the volume of everything, 15 is not a group.
     * @param volume 0 to 1,024.
     */
    void set_group_volume(u32 group, s32 volume);

    /**
     * Chooses between two loudspeakers and one.
     *
     * @param mono True for one: both sides carry the same.
     */
    void set_mono(bool mono) { mono_ = mono; }

    /**
     * Mixes the next stretch of sound, advancing the sounds as time passes.
     *
     * @param frames How many samples to produce.
     * @param[out] sums Left and right sums, two for each sample, which the mix is added to.
     */
    void mix(std::size_t frames, s32* sums);

    // --- For the sounds it plays ---

    /**
     * Starts a voice.
     *
     * @param start The tone and how to play it.
     * @return Where the voice is.
     */
    VoiceUse start_voice(const VoiceStart& start);

    /**
     * Finds a voice a sound started.
     *
     * @param use What `start_voice` gave.
     * @return The voice's slot, or null when the voice has ended or the slot was used again.
     */
    Slot* slot(VoiceUse use);

    /**
     * Gives a slot's voice the levels its group's volume makes of its own.
     *
     * @param target The slot.
     */
    void apply_levels(Slot& target);

    /** A random number, 0 to 0x7FFF. The sequence is the same on every run. */
    s32 random();

    /**
     * Gives one of the registers all sounds share.
     *
     * @param which Its number, 0 to 31; anything else gives a spare one.
     * @return The register.
     */
    s8& shared_register(s32 which);

    /** True when both sides carry the same. */
    bool mono() const { return mono_; }

    /** Ticks since the player was made. */
    u32 ticks() const { return ticks_; }

private:
    /** Advances every sound by one tick and forgets those that are over. */
    void tick();

    /**
     * Applies a sound's limit on how many of it may play at once.
     *
     * @param sfx The sound about to start.
     * @param volume The volume it is to start with.
     * @return True if it may start. A playing one may have been stopped to make room.
     */
    bool make_room(const Sfx* sfx, s32 volume);

    std::map<u32, std::unique_ptr<Bank>> banks_;           // Loaded banks, by their numbers.
    std::map<u32, std::unique_ptr<PlayingSound>> sounds_;  // Playing sounds, by their numbers.
    std::deque<Slot> slots_;                               // Voices; a free one is used again.
    std::array<s32, kGroups> group_volume_;                // Each group's volume, 1,024 for full.
    std::array<s8, 32> shared_registers_ = {};             // The registers all sounds share.
    s8 spare_register_ = 0;                                // Stands in for one out of range.
    u32 voice_serial_ = 0;                                 // Voice starts so far.
    u32 random_state_ = 1;                                 // The random sequence's state.
    u32 ticks_ = 0;                                        // Ticks so far.
    int frames_to_tick_ = 0;                               // Samples until the next tick.
    bool mono_ = false;                                    // One loudspeaker.
};

}  // namespace snd
