// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The envelope generator of a sound processor voice: its ADSR curve.
 *
 * A voice's loudness over its life is drawn by one generator that steps a 15-bit level up or down
 * at a rate two registers (ADSR1, ADSR2) set for each of four phases.
 *
 * Sources: the public documentation of the PlayStation sound processor's ADSR registers. The
 * stepping rule is adapted from the 989snd reimplementation in jak-project (ISC, see
 * THIRD_PARTY_NOTICES.md), which approximates the hardware's counter.
 */

#pragma once

#include "ps2/types.h"

namespace snd {

using ps2::s16;
using ps2::s32;
using ps2::s8;
using ps2::u16;
using ps2::u32;
using ps2::u8;

/**
 * The loudness curve of one voice: attack, decay, sustain, release.
 *
 * Only the thread that mixes sound uses it.
 */
class Envelope {
public:
    /** Where in its curve a voice is. `Stopped` is silence: the voice is free. */
    enum class Phase {
        Attack,
        Decay,
        Sustain,
        Release,
        Stopped,
    };

    /**
     * Sets the two registers that shape the curve.
     *
     * @param adsr1 Attack mode, shift and step, decay shift, sustain level (documented).
     * @param adsr2 Sustain mode, direction, shift and step, release mode and shift (documented).
     */
    void set_registers(u16 adsr1, u16 adsr2);

    /** Starts the curve from silence: key on. */
    void attack();

    /** Moves to the release phase from wherever the curve is: key off. */
    void release();

    /** Silences the voice at once. */
    void stop();

    /** Moves the curve one output sample on. */
    void run();

    /** The phase the curve is in. */
    Phase phase() const { return phase_; }

    /** The current level, 0 to 0x7FFF. */
    s32 level() const { return level_; }

private:
    /** Loads the rate and target of the phase that was entered from the registers. */
    void enter_phase();

    /** Steps the level once, at the current phase's rate. */
    void step();

    u32 registers_ = 0;             // ADSR1 in the low half, ADSR2 in the high half.
    Phase phase_ = Phase::Stopped;  // Where the curve is.
    s32 level_ = 0;                 // The level now, 0 to 0x7FFF.
    s32 target_ = 0;                // The level that ends the phase.
    u32 counter_ = 0;               // Fraction of a step gathered so far.
    u32 shift_ = 0;                 // The phase's rate: a larger shift is slower.
    s32 rate_step_ = 0;             // The phase's step: +4 to +7 rising, -8 to -5 falling.
    bool exponential_ = false;      // The phase's curve bends instead of running straight.
    bool falling_ = false;          // The phase lowers the level.
};

}  // namespace snd
