// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-game/src/audio.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The voices and the mixer (spu.h).

#include "audio/spu.h"

#include <algorithm>

namespace openrac::audio {

// psx-spx lists it as 512 values; OpenGOAL's common/interp_table.inc holds the
// same numbers as 256 rows of {g[255 - i], g[511 - i], g[256 + i], g[i]}.
const std::array<std::int16_t, 512> kGaussianTable = {
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    0,     0,     0,     0,     0,     0,     0,     1,     1,     1,
    1,     2,     2,     2,     3,     3,     3,     4,     4,     5,     5,     6,     7,
    7,     8,     9,     9,     10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    21,    22,    24,    25,    27,    28,    30,    32,    33,    35,    37,    39,
    41,    44,    46,    48,    51,    53,    56,    58,    61,    64,    67,    70,    73,
    77,    80,    84,    87,    91,    95,    99,    103,   107,   111,   116,   120,   125,
    130,   135,   140,   145,   150,   156,   161,   167,   173,   179,   186,   192,   199,
    205,   212,   219,   227,   234,   242,   250,   257,   266,   274,   283,   291,   300,
    309,   319,   328,   338,   348,   358,   369,   379,   390,   401,   412,   424,   436,
    448,   460,   473,   485,   498,   512,   525,   539,   553,   567,   582,   597,   612,
    627,   643,   659,   675,   692,   708,   726,   743,   761,   779,   797,   816,   835,
    854,   874,   894,   914,   935,   956,   977,   999,   1020,  1043,  1066,  1089,  1112,
    1136,  1160,  1184,  1209,  1234,  1260,  1286,  1312,  1339,  1366,  1394,  1422,  1450,
    1479,  1508,  1537,  1567,  1598,  1628,  1660,  1691,  1723,  1756,  1789,  1822,  1856,
    1890,  1924,  1959,  1995,  2031,  2067,  2104,  2141,  2179,  2217,  2256,  2295,  2334,
    2374,  2415,  2456,  2497,  2539,  2582,  2624,  2668,  2712,  2756,  2801,  2846,  2892,
    2938,  2985,  3032,  3079,  3128,  3176,  3225,  3275,  3325,  3376,  3427,  3479,  3531,
    3584,  3637,  3691,  3745,  3799,  3855,  3910,  3967,  4023,  4081,  4138,  4197,  4255,
    4315,  4374,  4435,  4495,  4557,  4619,  4681,  4744,  4807,  4871,  4935,  5000,  5065,
    5131,  5197,  5264,  5332,  5399,  5468,  5536,  5606,  5676,  5746,  5817,  5888,  5959,
    6032,  6104,  6177,  6251,  6325,  6400,  6475,  6550,  6626,  6702,  6779,  6856,  6934,
    7012,  7091,  7170,  7249,  7329,  7409,  7490,  7571,  7653,  7735,  7817,  7900,  7983,
    8066,  8150,  8234,  8319,  8404,  8489,  8575,  8661,  8748,  8834,  8922,  9009,  9097,
    9185,  9273,  9362,  9451,  9541,  9630,  9720,  9811,  9901,  9992,  10083, 10174, 10266,
    10358, 10450, 10542, 10635, 10727, 10820, 10913, 11007, 11100, 11194, 11288, 11382, 11476,
    11571, 11665, 11760, 11855, 11950, 12045, 12140, 12236, 12331, 12427, 12522, 12618, 12714,
    12809, 12905, 13001, 13097, 13193, 13289, 13385, 13481, 13577, 13673, 13769, 13865, 13961,
    14056, 14152, 14248, 14343, 14439, 14534, 14630, 14725, 14820, 14915, 15010, 15104, 15199,
    15293, 15387, 15481, 15575, 15669, 15762, 15855, 15948, 16041, 16133, 16226, 16317, 16409,
    16500, 16592, 16682, 16773, 16863, 16953, 17042, 17131, 17220, 17308, 17396, 17484, 17571,
    17658, 17744, 17830, 17916, 18001, 18086, 18170, 18254, 18337, 18420, 18502, 18584, 18665,
    18746, 18826, 18905, 18985, 19063, 19141, 19219, 19295, 19372, 19447, 19522, 19597, 19671,
    19744, 19816, 19888, 19959, 20030, 20100, 20169, 20238, 20306, 20373, 20439, 20505, 20570,
    20634, 20698, 20760, 20822, 20884, 20944, 21004, 21063, 21121, 21178, 21235, 21290, 21345,
    21399, 21452, 21505, 21556, 21607, 21657, 21706, 21754, 21801, 21848, 21893, 21938, 21982,
    22025, 22066, 22107, 22148, 22187, 22225, 22262, 22299, 22334, 22369, 22402, 22435, 22467,
    22498, 22527, 22556, 22584, 22611, 22637, 22662, 22686, 22709, 22731, 22752, 22772, 22791,
    22809, 22826, 22842, 22857, 22872, 22885, 22897, 22908, 22918, 22927, 22935, 22942, 22948,
    22953, 22957, 22960, 22962, 22963,
};

int gaussian_interpolate(std::size_t i, const std::array<int, 4>& s) {
    const auto& g = kGaussianTable;
    return ((g[0xff - i] * s[0]) >> 15) + ((g[0x1ff - i] * s[1]) >> 15)
           + ((g[0x100 + i] * s[2]) >> 15) + ((g[i] * s[3]) >> 15);
}

void Voice::key_on(std::uint16_t pitch, std::uint16_t adsr1, std::uint16_t adsr2) {
    m_history = {};
    m_previous = {};
    m_counter = 0;
    m_part_frame = 0;
    m_pitch = pitch;
    m_envelope.set_registers(adsr1, adsr2);
    ++m_generation;
    m_reverb = false;
    m_held = false;
    load_block();
    m_envelope.attack();
}

void Voice::key_on_memory(
    SampleData memory,
    std::size_t start,
    std::uint16_t pitch,
    std::uint16_t adsr1,
    std::uint16_t adsr2
) {
    m_streaming = false;
    m_memory = std::move(memory);
    m_part = {};
    m_queue.clear();
    m_address = start;
    m_loop_address = start;
    key_on(pitch, adsr1, adsr2);
}

void Voice::key_on_stream(
    StreamPart first, std::uint16_t pitch, std::uint16_t adsr1, std::uint16_t adsr2
) {
    m_streaming = true;
    m_memory = nullptr;
    m_part = std::move(first);
    m_queue.clear();
    m_address = 0;
    m_loop_address = 0;
    key_on(pitch, adsr1, adsr2);
}

void Voice::queue_part(StreamPart part) {
    // Parts queued on a voice that is not a stream have nowhere to play, so they are ignored.
    if (m_streaming) {
        m_queue.push_back(std::move(part));
    }
}

const std::vector<std::uint8_t>* Voice::source() const {
    return m_streaming ? m_part.data.get() : m_memory.get();
}

std::size_t Voice::stream_remaining() const {
    // Nothing is left to play on a voice that is not streaming or has no part loaded.
    if (!m_streaming || !m_part.data) {
        return 0;
    }

    const std::size_t frames = m_part.data->size() / assets::kAdpcmFrameBytes;
    const std::size_t left = frames > m_part_frame ? frames - m_part_frame : 0;
    const std::size_t in_block = m_counter >> 12;
    const std::size_t total = left * assets::kAdpcmFrameSamples;
    return total > in_block ? total - in_block : 0;
}

void Voice::load_block() {
    const std::vector<std::uint8_t>* data = source();

    // Silence rather than an out-of-range read when the address is past the end of the data.
    if (data == nullptr || m_address + assets::kAdpcmFrameBytes > data->size()) {
        // Past the data (a malformed sample): silence.
        m_envelope.stop();
        return;
    }

    const std::uint8_t* frame = data->data() + m_address;
    m_flags = frame[1];
    m_block = assets::decode_adpcm_frame(frame, m_history);

    // A loop-start flag marks where a loop jumps back to, so the voice remembers the address.
    if (m_flags & assets::kAdpcmFlagLoopStart) {
        m_loop_address = m_address;
    }
}

void Voice::next_block() {
    m_previous = {m_block[25], m_block[26], m_block[27]};

    // The end flag says this was the last block of the sound or stream part.
    if (m_flags & assets::kAdpcmFlagEnd) {
        // A sound in memory ends or loops here; a stream goes on to its next queued part.
        if (!m_streaming) {
            m_address = m_loop_address;

            // Without the repeat flag the voice stops at the end instead of looping.
            if ((m_flags & assets::kAdpcmFlagRepeat) == 0) {
                m_envelope.stop();
                return;
            }
        } else {
            // A stream moves to the next part that was queued, and the loop address restarts in
            // that part.
            if (!m_queue.empty()) {
                m_part = std::move(m_queue.front());
                m_queue.pop_front();
            } else if (!m_part.looped) {
                // With nothing queued a stream part either repeats itself (looped) or ends the
                // voice.
                m_envelope.stop();
                return;
            }

            m_address = 0;
            m_loop_address = 0;
            m_part_frame = 0;
        }
    } else {
        m_address += assets::kAdpcmFrameBytes;
        ++m_part_frame;
    }

    load_block();
}

std::pair<int, int> Voice::run() {
    // A stopped or held voice outputs silence and does not advance.
    if (m_envelope.phase() == EnvelopePhase::Stopped || m_held) {
        return {0, 0};
    }

    const auto index = static_cast<int>(m_counter >> 12);
    auto at = [this](int k) -> int {
        return k >= 0 ? m_block[static_cast<std::size_t>(k)]
                      : m_previous[static_cast<std::size_t>(3 + k)];
    };
    const int interpolated = static_cast<std::int16_t>(gaussian_interpolate(
        (m_counter >> 4) & 0xff, {at(index - 3), at(index - 2), at(index - 1), at(index)}
    ));
    const int sample = static_cast<std::int16_t>((interpolated * m_envelope.level()) >> 15);
    auto level = [](std::uint16_t reg) {
        return static_cast<int>(static_cast<std::int16_t>(reg << 1));
    };
    const int left = static_cast<std::int16_t>((sample * level(m_volume[0])) >> 15);
    const int right = static_cast<std::int16_t>((sample * level(m_volume[1])) >> 15);
    m_counter += std::min<std::uint16_t>(m_pitch, 0x3fff);

    // A step can cross more than one block at a high pitch, so decode until the position is within
    // the current block.
    while ((m_counter >> 12) >= assets::kAdpcmFrameSamples) {
        m_counter -= static_cast<std::uint32_t>(assets::kAdpcmFrameSamples) << 12;
        next_block();

        // Stop early when the end of the sound stopped the voice: the rest of the step has nothing
        // to play.
        if (m_envelope.phase() == EnvelopePhase::Stopped) {
            break;
        }
    }

    m_envelope.run();
    return {left, right};
}

std::int16_t master_out(std::int64_t x, std::uint16_t reg) {
    const std::int64_t level = static_cast<std::int16_t>(reg << 1);
    return static_cast<std::int16_t>(std::clamp<std::int64_t>((x * level) >> 15, -32768, 32767));
}

std::array<std::int16_t, 2> VoiceMixer::mix(std::array<int, 2> extra) {
    std::int64_t left = extra[0];
    std::int64_t right = extra[1];
    std::array<int, 2> send{};

    // Every voice runs every sample, so the ones that are stopped cost almost nothing but still
    // advance.
    for (Voice& v : m_voices) {
        const auto [a, b] = v.run();
        left += a;
        right += b;

        // Voices flagged for reverb also feed the reverb send, before the master volume.
        if (v.reverb()) {
            send[0] += a;
            send[1] += b;
        }
    }

    // The reverb is added only when an effect is set, so that unused effects cost nothing.
    if (m_reverb.active()) {
        const auto wet = m_reverb.run(send);
        left += wet[0];
        right += wet[1];
    }

    return {master_out(left, m_master[0]), master_out(right, m_master[1])};
}

std::size_t VoiceMixer::active_voices() const {
    return static_cast<std::size_t>(
        std::count_if(m_voices.begin(), m_voices.end(), [](const Voice& v) { return v.active(); })
    );
}

}  // namespace openrac::audio
