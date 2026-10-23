// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/vag.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The SPU ADPCM decoder and the sample walk (adpcm.h).

#include "assets/sound/adpcm.h"

#include <algorithm>

namespace openrac::assets {

namespace {

// The prediction coefficients (in 1/64), indexed by the filter nibble: the
// hardware's, as psx-spx lists them.
constexpr int kPositive[5] = {0, 60, 115, 98, 122};
constexpr int kNegative[5] = {0, 0, -52, -55, -60};

}  // namespace

std::array<s16, kAdpcmFrameSamples> decode_adpcm_frame(
    const u8* frame, AdpcmHistory& history, AdpcmRounding rounding
) {
    const int shift = frame[0] & 0x0f;
    const int filter = std::min(frame[0] >> 4, 4);
    const int k0 = kPositive[filter];
    const int k1 = kNegative[filter];
    std::array<s16, kAdpcmFrameSamples> out{};

    // A frame holds two samples per byte after its 2-byte header, low nibble first.
    for (std::size_t i = 0; i < kAdpcmFrameSamples; ++i) {
        const int nibble = (frame[2 + i / 2] >> ((i & 1) * 4)) & 0x0f;
        // The nibble is the top four bits of a signed 16-bit value.
        int s = static_cast<s16>(static_cast<u16>(nibble << 12)) >> shift;

        // The rounded form adds half a step before the shift; the other form truncates.
        if (rounding == AdpcmRounding::Rounded) {
            s += (k0 * history.h1 + k1 * history.h2 + 32) >> 6;
        } else {
            s += (k0 * history.h1) >> 6;
            s += (k1 * history.h2) >> 6;
        }

        s = std::clamp(s, -32768, 32767);
        history.h2 = history.h1;
        history.h1 = s;
        out[i] = static_cast<s16>(s);
    }

    return out;
}

std::vector<s16> decode_adpcm(ByteView adpcm, AdpcmRounding rounding) {
    const std::size_t frames = adpcm.size() / kAdpcmFrameBytes;
    std::vector<s16> out;
    out.reserve(frames * kAdpcmFrameSamples);
    AdpcmHistory history;

    // Decode frame after frame; the history carries from one frame to the next.
    for (std::size_t f = 0; f < frames; ++f) {
        const auto pcm = decode_adpcm_frame(adpcm.data() + f * kAdpcmFrameBytes, history, rounding);
        out.insert(out.end(), pcm.begin(), pcm.end());
    }

    return out;
}

std::optional<std::pair<std::size_t, std::size_t>> SampleExtent::loop_points() const {
    // A sample that does not loop has no loop points.
    if (!looped) {
        return std::nullopt;
    }

    return std::pair{loop_start.value_or(0) * kAdpcmFrameSamples, samples()};
}

SampleExtent sample_extent(ByteView adpcm, std::size_t offset) {
    // Samples start on frame boundaries; a misaligned offset is not a sample.
    if (offset % kAdpcmFrameBytes != 0) {
        fail("sample offset {:#x} is not frame aligned", offset);
    }

    SampleExtent extent;
    extent.offset = offset;

    // Walk frame by frame until the end flag; the loop exits from inside.
    for (std::size_t f = 0;; ++f) {
        const std::size_t at = offset + f * kAdpcmFrameBytes;

        // A sample must end with an end flag before the data does.
        if (at + kAdpcmFrameBytes > adpcm.size()) {
            fail("sample at {:#x} runs past the data without an end flag", offset);
        }

        const u8 flags = adpcm.data()[at + 1];

        // The loop start flag marks the frame the loop jumps back to.
        if (flags & kAdpcmFlagLoopStart) {
            extent.loop_start = f;
        }

        // The end flag closes the sample, and the repeat flag says whether it loops.
        if (flags & kAdpcmFlagEnd) {
            extent.frames = f + 1;
            extent.looped = (flags & kAdpcmFlagRepeat) != 0;
            const std::size_t next = at + kAdpcmFrameBytes;

            // The flags of the frame after the end say how the sound processor continues, if there
            // is one.
            if (next + kAdpcmFrameBytes <= adpcm.size()) {
                extent.next_flags = adpcm.data()[next + 1];
            }

            return extent;
        }
    }
}

std::vector<s16> decode_extent(ByteView adpcm, const SampleExtent& extent) {
    return decode_adpcm(adpcm.sub(extent.offset, extent.bytes(), "sample"));
}

}  // namespace openrac::assets
