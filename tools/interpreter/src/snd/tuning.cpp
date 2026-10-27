// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Notes to pitch, volume and pan to levels (see tuning.h).
 */

#include "snd/tuning.h"

#include <algorithm>
#include <cmath>

#include "snd/voice.h"

namespace snd {
namespace {

/** Fine steps in a semitone, and semitones in an octave. */
constexpr s32 kFineSteps = 128;
constexpr s32 kSemitones = 12;

/** The top of a voice's level range, and the top of the library's volume range. */
constexpr s32 kFullLevel = 0x7FFF;
constexpr s32 kFullVolume = 127;

/** A full turn of pan, in degrees. */
constexpr s32 kTurn = 360;

/** Pi, for the sines. */
constexpr double kPi = 3.14159265358979323846;

/**
 * Brings an angle into one turn.
 *
 * @param degrees Any angle.
 * @return The same direction, 0 to 359.
 */
s32 wrap_degrees(s32 degrees) {
    s32 wrapped = degrees % kTurn;

    // The remainder of a negative angle is negative.
    if (wrapped < 0) {
        wrapped += kTurn;
    }

    return wrapped;
}

}  // namespace

u32 note_pitch(const Tone& tone, Note note) {
    // A centre note below zero is stored negated and marks a sample at the processor's own rate.
    bool native_rate = tone.center_note < 0;
    s32 center = native_rate ? -tone.center_note : tone.center_note;

    // Distance from the centre in fine steps; an octave up doubles the pitch.
    s32 distance = (note.note - center) * kFineSteps + note.fine + tone.center_fine;
    double octaves = static_cast<double>(distance) / (kFineSteps * kSemitones);
    double pitch = static_cast<double>(Voice::kUnitPitch) * std::pow(2.0, octaves);

    // A sample recorded at 44,100 a second plays slower on a processor that runs at 48,000.
    if (!native_rate) {
        pitch = pitch * 44100.0 / 48000.0;
    }

    return static_cast<u32>(std::clamp(pitch + 0.5, 0.0, 16383.0));
}

Note bent_note(const Tone& tone, s32 pitch_bend, s32 pitch_modifier, Note start) {
    s32 fine = start.note * kFineSteps + start.fine + pitch_modifier;

    // A full bend moves the tone its own number of semitones, up or down.
    if (pitch_bend >= 0) {
        fine += tone.bend_high * (pitch_bend * kFineSteps) / 0x7FFF;
    } else {
        fine += tone.bend_low * (pitch_bend * kFineSteps) / 0x8000;
    }

    return Note{fine / kFineSteps, fine % kFineSteps};
}

Levels pan_levels(s32 sound_volume, s32 sound_pan, s32 tone_volume, s32 tone_pan, bool mono) {
    // 127 * 258 is as near the top of the level range as whole volumes get.
    s32 volume = kFullVolume * 258;

    volume = volume * sound_volume / kFullVolume;
    volume = volume * tone_volume / kFullVolume;

    // Silent on both sides.
    if (volume == 0) {
        return Levels{};
    }

    // One loudspeaker: no pan.
    if (mono) {
        return Levels{volume, volume};
    }

    // Pan 0 is straight ahead; turn it so that 0 is hard left and 180 hard right.
    s32 pan = wrap_degrees(sound_pan + tone_pan);

    pan = pan >= 270 ? pan - 270 : pan + 90;

    // Behind the listener the sides swap.
    bool behind = pan >= 180;
    double angle = static_cast<double>(behind ? pan - 180 : pan) * kPi / 360.0;
    s32 near = static_cast<s32>(std::cos(angle) * volume);
    s32 far = static_cast<s32>(std::sin(angle) * volume);

    return behind ? Levels{far, near} : Levels{near, far};
}

s32 group_level(s32 level, s32 group_volume) {
    s32 scaled = std::min(level, kFullLevel - 1) * group_volume / 1024;
    s32 squared = scaled * scaled / (kFullLevel - 1);

    return scaled < 0 ? -squared : squared;
}

s32 sine_step(s32 step) {
    double angle = static_cast<double>(step & 2047) * 2.0 * kPi / 2048.0;

    return static_cast<s32>(std::lround(std::cos(angle) * 32767.0));
}

}  // namespace snd
