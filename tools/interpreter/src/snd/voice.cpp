// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * One voice of the console's sound processor (see voice.h).
 */

#include "snd/voice.h"

#include <algorithm>

namespace snd {
namespace {

/**
 * The five predictors of the ADPCM format: weights, in 64ths, of the two samples before
 * (documented).
 */
constexpr s32 kWeights[5][2] = {{0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}};

/** The highest pitch the register holds (documented). */
constexpr u32 kHighestPitch = 0x3FFF;

/** Flag bits of a block's second byte: the last block, loop after it, a loop starts here. */
constexpr u8 kLastBlock = 1;
constexpr u8 kRepeat = 2;
constexpr u8 kLoopStart = 4;

}  // namespace

void Voice::key_on(const u8* sample, std::size_t bytes, u16 adsr1, u16 adsr2) {
    sample_ = sample;
    bytes_ = bytes;
    block_ = 0;
    loop_ = 0;
    at_ = kBlockSamples;
    ends_ = false;
    repeats_ = false;
    before_[0] = 0;
    before_[1] = 0;
    previous_ = 0;
    current_ = 0;
    phase_ = 0;

    envelope_.set_registers(adsr1, adsr2);
    envelope_.attack();
}

void Voice::set_pitch(u32 pitch) {
    pitch_ = std::min(pitch, kHighestPitch);
}

void Voice::set_volume(s32 left, s32 right) {
    left_ = left;
    right_ = right;
}

void Voice::run(s32& left, s32& right) {
    // A free voice adds nothing.
    if (is_free()) {
        return;
    }

    phase_ += pitch_;

    // Step over as many stored samples as the pitch has covered.
    while (phase_ >= kUnitPitch && !is_free()) {
        phase_ -= kUnitPitch;
        next_sample();
    }

    // The sample ended while stepping.
    if (is_free()) {
        return;
    }

    // A straight line between the two stored samples around the output's position.
    s32 fraction = static_cast<s32>(phase_);
    s32 value = previous_ + (((current_ - previous_) * fraction) >> 12);

    // The envelope, then each side's level; both are 15-bit fractions (documented).
    value = (value * envelope_.level()) >> 15;
    left += (value * left_) >> 15;
    right += (value * right_) >> 15;

    envelope_.run();
}

void Voice::next_sample() {
    // The block is used up: follow its flags, then decode the next one.
    if (at_ == kBlockSamples) {
        // After the last block the sample loops, or the voice ends (documented).
        if (ends_ && !repeats_) {
            envelope_.stop();
            return;
        }

        // A repeating sample goes back to its loop start.
        if (ends_) {
            block_ = loop_;
        }

        // Nothing left to read: a sample with no end flag, or a bad loop address.
        if (block_ + kBlockBytes > bytes_) {
            envelope_.stop();
            return;
        }

        decode_block();
    }

    previous_ = current_;
    current_ = decoded_[at_];
    at_++;
}

void Voice::decode_block() {
    const u8* block = sample_ + block_;

    // First byte: the shift in bits 0-3, the predictor in bits 4-6 (documented).
    unsigned shift = block[0] & 0xF;
    unsigned predictor = std::min<unsigned>((block[0] >> 4) & 7, 4);
    u8 flags = block[1];

    // A loop returns to the latest block that carries the start flag.
    if (flags & kLoopStart) {
        loop_ = block_;
    }

    ends_ = (flags & kLastBlock) != 0;
    repeats_ = (flags & kRepeat) != 0;

    // Fourteen bytes of two samples each, the low four bits first (documented).
    for (unsigned n = 0; n < kBlockSamples; n++) {
        s32 nibble = (block[2 + n / 2] >> ((n & 1) * 4)) & 0xF;
        s32 value = static_cast<s16>(nibble << 12) >> shift;

        value += (before_[0] * kWeights[predictor][0] + before_[1] * kWeights[predictor][1]) >> 6;
        value = std::clamp(value, -32768, 32767);

        decoded_[n] = static_cast<s16>(value);
        before_[1] = before_[0];
        before_[0] = value;
    }

    at_ = 0;
    block_ += kBlockBytes;
}

}  // namespace snd
