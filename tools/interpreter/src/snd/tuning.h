// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The arithmetic between a sound's musical terms and a voice's registers: notes to pitch, a
 * volume and a pan angle to left and right levels.
 *
 * Sources: the steps follow the 989snd reimplementation in jak-project (ISC, see
 * THIRD_PARTY_NOTICES.md). Its tables (a pitch for each note and fine step, a level pair for each
 * half degree) are not copied: they are powers of two and sines, computed here.
 */

#pragma once

#include "snd/bank.h"

namespace snd {

/** A note and its fine tuning in 128ths of a semitone. */
struct Note {
    s32 note = 0;  // Semitones; 60 is the note sounds play at unless told otherwise.
    s32 fine = 0;  // 128ths of a semitone, 0 to 127 (or down to -127 for a note below zero).
};

/** A loudness on each side, 0 to 0x7FFF. */
struct Levels {
    s32 left = 0;
    s32 right = 0;
};

/**
 * Works out the pitch register value that plays a tone's sample as a given note.
 *
 * @param tone The tone: its centre note says which note the sample is recorded at. A centre note
 *     that is not negative marks a sample recorded for 44,100 samples a second.
 * @param note The note to sound.
 * @return The pitch, with `Voice::kUnitPitch` for the sample's own rate.
 */
u32 note_pitch(const Tone& tone, Note note);

/**
 * Applies a pitch bend and a pitch modifier to a note.
 *
 * @param tone The tone, for how many semitones a full bend moves it.
 * @param pitch_bend The bend, -0x8000 to 0x7FFF.
 * @param pitch_modifier An offset in 128ths of a semitone.
 * @param start The note before either.
 * @return The note after both.
 */
Note bent_note(const Tone& tone, s32 pitch_bend, s32 pitch_modifier, Note start);

/**
 * Works out a voice's levels from a sound's and a tone's volume and pan.
 *
 * @param sound_volume The sound's volume, 0 to 127.
 * @param sound_pan The sound's pan in degrees.
 * @param tone_volume The tone's volume, 0 to 127.
 * @param tone_pan The tone's pan in degrees.
 * @param mono True when both sides are to carry the same level.
 * @return The levels, before the sound's group volume is applied.
 */
Levels pan_levels(s32 sound_volume, s32 sound_pan, s32 tone_volume, s32 tone_pan, bool mono);

/**
 * Scales a level by its group's volume.
 *
 * The library squares the result, so that a volume setting is heard as evenly spaced steps.
 *
 * @param level A level from `pan_levels`.
 * @param group_volume The group's volume, 1,024 for full.
 * @return The level a voice is given.
 */
s32 group_level(s32 level, s32 group_volume);

/**
 * Gives one step of a sine wave.
 *
 * @param step Position in the wave, 0 to 2,047 for one turn.
 * @return The wave's height, -32,767 to 32,767, starting at its top.
 */
s32 sine_step(s32 step);

}  // namespace snd
