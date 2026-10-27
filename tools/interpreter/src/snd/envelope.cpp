// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The envelope generator of a sound processor voice (see envelope.h).
 */

#include "snd/envelope.h"

#include <algorithm>

namespace snd {
namespace {

/** The highest level of the curve: 15 bits. */
constexpr s32 kFullLevel = 0x7FFF;

/** One whole step of the rate counter. A phase adds a part of this for each output sample. */
constexpr u32 kCounterStep = 0x800000;

/** The shift at which the counter starts to slow instead of the step growing (documented). */
constexpr s32 kShiftPivot = 11;

/** The level above which an exponential rise slows to a quarter (documented). */
constexpr s32 kSlowRiseLevel = 0x6000;

/**
 * Reads a field of the two ADSR registers.
 *
 * @param registers ADSR1 in the low 16 bits, ADSR2 in the high 16.
 * @param first The field's lowest bit.
 * @param count How many bits it has.
 * @return The field's value.
 */
u32 field(u32 registers, unsigned first, unsigned count) {
    return (registers >> first) & ((1u << count) - 1);
}

}  // namespace

void Envelope::set_registers(u16 adsr1, u16 adsr2) {
    registers_ = static_cast<u32>(adsr1) | (static_cast<u32>(adsr2) << 16);
}

void Envelope::attack() {
    phase_ = Phase::Attack;
    level_ = 0;
    counter_ = 0;
    enter_phase();
}

void Envelope::release() {
    phase_ = Phase::Release;
    counter_ = 0;
    enter_phase();
}

void Envelope::stop() {
    phase_ = Phase::Stopped;
    level_ = 0;
}

void Envelope::run() {
    // A stopped voice has no curve to move.
    if (phase_ == Phase::Stopped) {
        return;
    }

    step();

    // Sustain has no target: it runs until the key is released.
    if (phase_ == Phase::Sustain) {
        return;
    }

    bool reached = falling_ ? level_ <= target_ : level_ >= target_;

    // The phase runs on until its target is reached.
    if (!reached) {
        return;
    }

    // Attack leads to decay, decay to sustain, release to silence (documented).
    switch (phase_) {
        case Phase::Attack:
            phase_ = Phase::Decay;
            break;

        case Phase::Decay:
            phase_ = Phase::Sustain;
            break;

        case Phase::Release:
            phase_ = Phase::Stopped;
            break;

        case Phase::Sustain:
        case Phase::Stopped:
            break;
    }

    enter_phase();
}

void Envelope::enter_phase() {
    // Fields of ADSR1 and ADSR2 as one 32-bit word, ADSR2 above ADSR1 (documented).
    switch (phase_) {
        case Phase::Attack:
            // Attack: mode bit 15, shift bits 10-14, step bits 8-9; it always rises to the top.
            exponential_ = field(registers_, 15, 1) != 0;
            falling_ = false;
            shift_ = field(registers_, 10, 5);
            rate_step_ = 7 - static_cast<s32>(field(registers_, 8, 2));
            target_ = kFullLevel;
            break;

        case Phase::Decay:
            // Decay: shift bits 4-7; it falls exponentially to the sustain level, bits 0-3.
            exponential_ = true;
            falling_ = true;
            shift_ = field(registers_, 4, 4);
            rate_step_ = -8;
            target_ = static_cast<s32>(field(registers_, 0, 4) + 1) << 11;
            break;

        case Phase::Sustain:
            // Sustain: mode bit 31, direction bit 30, shift bits 24-28, step bits 22-23.
            exponential_ = field(registers_, 31, 1) != 0;
            falling_ = field(registers_, 30, 1) != 0;
            shift_ = field(registers_, 24, 5);
            rate_step_ = static_cast<s32>(field(registers_, 22, 2));
            rate_step_ = falling_ ? rate_step_ - 8 : 7 - rate_step_;
            target_ = 0;
            break;

        case Phase::Release:
            // Release: mode bit 21, shift bits 16-20; it always falls to silence.
            exponential_ = field(registers_, 21, 1) != 0;
            falling_ = true;
            shift_ = field(registers_, 16, 5);
            rate_step_ = -8;
            target_ = 0;
            break;

        case Phase::Stopped:
            break;
    }
}

void Envelope::step() {
    u32 counter_step = kCounterStep;
    s32 slowing = static_cast<s32>(shift_) - kShiftPivot;

    // Past the pivot a larger shift makes steps rarer; below it, it makes them smaller.
    if (slowing > 0) {
        counter_step >>= slowing;
    }

    s32 change = rate_step_ << std::max(0, kShiftPivot - static_cast<s32>(shift_));

    // An exponential rise slows near the top.
    if (exponential_ && !falling_ && level_ > kSlowRiseLevel) {
        counter_step >>= 2;
    }

    // An exponential fall scales with the level.
    if (exponential_ && falling_) {
        change = (change * level_) >> 15;
    }

    counter_ += counter_step;

    // Not a whole step yet.
    if (counter_ < kCounterStep) {
        return;
    }

    counter_ = 0;
    level_ = std::clamp(level_ + change, 0, kFullLevel);
}

}  // namespace snd
