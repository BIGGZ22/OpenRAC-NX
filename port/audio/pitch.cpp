// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/vag.rs and
// crates/rc-game/src/audio/grain_vm.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Notes to pitch words (pitch.h).

#include "audio/pitch.h"

#include <algorithm>
#include <cmath>

namespace openrac::audio {

const std::array<std::uint16_t, 140>& note_pitch_table() {
    static const std::array<std::uint16_t, 140> table = [] {
        std::array<std::uint16_t, 140> t{};

        // The table has the twelve semitone ratios first, then 128 steps of 1/128 semitone.
        for (int k = 0; k < 140; ++k) {
            const double e = k < 12 ? k / 12.0 : (k - 12) / 1536.0;
            t[k] = static_cast<std::uint16_t>(32768.0 * std::pow(2.0, e));
        }

        return t;
    }();
    return table;
}

std::uint16_t note_to_pitch(
    std::uint16_t center_note, std::uint16_t center_fine, std::uint16_t note, std::int16_t fine
) {
    const auto& table = note_pitch_table();
    const int fine_total = fine + center_fine;
    const int fine_carry = (fine_total < 0 ? fine_total + 127 : fine_total) / 128;
    const int n = note + fine_carry - center_note;
    int oct = n / 6;

    // C++ division truncates toward zero, but the octave has to round down for negative notes.
    if (n < 0) {
        --oct;
    }

    int fine_index = fine_total - fine_carry * 128;
    const int negative = n < 0 ? -1 : 0;

    // Same rounding again: a negative octave count is moved one lower so the halving below floors.
    if (oct < 0) {
        --oct;
    }

    const int octave = oct / 2 - negative;
    int shift = octave - 2;
    int semitone = n - octave * 12;

    // A semitone below zero (or at zero with a negative fine step) borrows an octave from the
    // shift.
    if (semitone < 0 || (semitone == 0 && fine_index < 0)) {
        semitone += 12;
        shift = octave - 3;
    }

    // A negative fine step borrows a semitone, so the fine index lands in the table's 0..127 range.
    if (fine_index < 0) {
        semitone = semitone - 1 + fine_carry;
        fine_index += (fine_carry + 1) * 128;
    }

    int ret = (table[static_cast<std::size_t>(semitone)]
               * table[static_cast<std::size_t>(fine_index + 12)])
              / 0x10000;

    // A negative shift lowers the pitch: shift right, rounding to nearest rather than truncating.
    if (shift < 0) {
        ret = (ret + (1 << (-shift - 1))) >> -shift;
    }

    return static_cast<std::uint16_t>(ret);
}

std::uint16_t ps1_note_to_pitch(
    std::int8_t center_note, std::int8_t center_fine, std::int16_t note, std::int16_t fine
) {
    const bool ps1 = center_note >= 0;
    const int centre = ps1 ? center_note : -center_note;
    const std::uint16_t p = note_to_pitch(
        static_cast<std::uint16_t>(centre),
        static_cast<std::uint8_t>(center_fine),
        static_cast<std::uint16_t>(note),
        fine
    );
    return ps1 ? static_cast<std::uint16_t>(44100u * p / 48000u) : p;
}

std::uint16_t rate_to_pitch(std::uint32_t rate) {
    return static_cast<std::uint16_t>(
        std::min<std::uint64_t>(std::uint64_t{rate} * 0x1000 / 48000, 0x3fff)
    );
}

std::pair<std::int16_t, std::int16_t> pitch_bend(
    const assets::Tone& tone, int bend, int modulation, int note, int fine
) {
    const int base = (note << 7) + fine + modulation;
    const int v = bend >= 0 ? tone.bend_up * (bend << 7) / 0x7fff + base
                            : tone.bend_down * (bend << 7) / 0x8000 + base;
    return {static_cast<std::int16_t>(v / 128), static_cast<std::int16_t>(v % 128)};
}

std::uint16_t tone_pitch(const assets::Tone& tone, int bend, int modulation) {
    const auto [n, f] = pitch_bend(tone, bend, modulation, 60, 0);
    return ps1_note_to_pitch(tone.center_note, tone.center_fine, n, f);
}

}  // namespace openrac::audio
