// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * One voice of the console's sound processor: a sample played at a pitch under an envelope.
 *
 * Samples are stored as ADPCM: blocks of 16 bytes holding 28 samples each, with a header that
 * also marks where a sample loops and ends. A voice decodes its sample block by block, steps
 * through it at its pitch, shapes it with its envelope and scales it to the left and the right.
 *
 * Sources: the public documentation of the PlayStation sound processor (the ADPCM block format,
 * its predictor weights, the loop flags, the pitch register). The hardware's four-point
 * interpolation is replaced by a straight line between two samples.
 */

#pragma once

#include <cstddef>

#include "snd/envelope.h"

namespace snd {

/**
 * A sample being played.
 *
 * Only the thread that mixes sound uses it. A voice that has ended is free to be keyed on again.
 */
class Voice {
public:
    /** The pitch at which one stored sample is one output sample (documented). */
    static constexpr u32 kUnitPitch = 0x1000;

    /**
     * Starts playing a sample from its first block: key on.
     *
     * @param sample First block of the sample. The memory must outlive the voice's playing.
     * @param bytes How many bytes may be read from `sample`. Playing past them ends the voice.
     * @param adsr1 First envelope register.
     * @param adsr2 Second envelope register.
     */
    void key_on(const u8* sample, std::size_t bytes, u16 adsr1, u16 adsr2);

    /** Lets the voice die away along its release curve: key off. */
    void key_off() { envelope_.release(); }

    /** Silences the voice at once and frees it. */
    void stop() { envelope_.stop(); }

    /** True when the voice plays nothing and can be given a new sample. */
    bool is_free() const { return envelope_.phase() == Envelope::Phase::Stopped; }

    /**
     * Sets how fast the sample is stepped through.
     *
     * @param pitch Stored samples per output sample, in 4096ths; at most 0x3FFF (documented).
     */
    void set_pitch(u32 pitch);

    /**
     * Sets the voice's loudness on each side.
     *
     * @param left Left level, 0 to 0x7FFF; negative inverts the phase.
     * @param right Right level, likewise.
     */
    void set_volume(s32 left, s32 right);

    /**
     * Produces the voice's next output sample and adds it to a mix.
     *
     * @param[out] left Sum the left side is added to.
     * @param[out] right Sum the right side is added to.
     */
    void run(s32& left, s32& right);

private:
    /** Decodes the block at `block_` into `decoded_` and reads its loop flags. */
    void decode_block();

    /** Moves one stored sample on, decoding the next block or following a loop when needed. */
    void next_sample();

    /** The samples of one ADPCM block (documented). */
    static constexpr unsigned kBlockSamples = 28;

    /** The bytes of one ADPCM block (documented). */
    static constexpr std::size_t kBlockBytes = 16;

    Envelope envelope_;                // The loudness curve.
    const u8* sample_ = nullptr;       // First block of the sample; not owned.
    std::size_t bytes_ = 0;            // Bytes readable from `sample_`.
    std::size_t block_ = 0;            // Byte offset of the block after the one decoded.
    std::size_t loop_ = 0;             // Byte offset of the block a loop returns to.
    s16 decoded_[kBlockSamples] = {};  // The block being played.
    unsigned at_ = kBlockSamples;      // The next sample of `decoded_` to take.
    bool ends_ = false;                // The block being played is the sample's last.
    bool repeats_ = false;             // After the last block the sample goes back to `loop_`.
    s32 before_[2] = {0, 0};           // The two samples decoded last, for the predictor.
    s32 previous_ = 0;                 // The stored sample before the output's position.
    s32 current_ = 0;                  // The stored sample after it.
    u32 phase_ = 0;                    // How far between the two the output is, in 4096ths.
    u32 pitch_ = kUnitPitch;           // Stored samples per output sample, in 4096ths.
    s32 left_ = 0;                     // Left level.
    s32 right_ = 0;                    // Right level.
};

}  // namespace snd
