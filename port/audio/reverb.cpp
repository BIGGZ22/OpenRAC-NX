// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-game/src/audio/reverb.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The reverb effect (reverb.h).

#include "audio/reverb.h"

#include <algorithm>
#include <cmath>

namespace openrac::audio {

namespace {

constexpr float kRate = 48000.0f;
// The input band limit (the hardware's reverb runs at 24 kHz): a one-pole
// low-pass at about 11 kHz.
constexpr float kInputPole = 0.237f;
// The wet scale of the comb sum: four comb taps at about 0.6 over a loop
// gain of 0.5..0.8 give roughly the send's level back.
constexpr float kCombOut = 0.5f;
// Stereo decorrelation: the right channel's lines are this much longer.
constexpr std::size_t kSpread = 23;
// The four combs per channel: the same-side loop, the cross-side loop and
// two in between (mutually detuned): (which loop, scale).
constexpr std::pair<int, float> kCombs[4] = {{0, 1.0f}, {1, 1.0f}, {0, 0.781f}, {1, 0.853f}};

struct Line {
    std::vector<float> buffer;
    std::size_t pos = 0;

    explicit Line(std::size_t n) : buffer(std::max<std::size_t>(n, 1), 0.0f) {}

    float read() const { return buffer[pos]; }

    void write_advance(float x) {
        buffer[pos] = x;

        if (++pos == buffer.size()) {
            pos = 0;
        }
    }
};

struct Comb {
    Line line;
    float gain;
    float damp;
    float lowpass = 0;

    float run(float x) {
        const float y = line.read();
        lowpass = y + (lowpass - y) * damp;
        line.write_advance(x + lowpass * gain);
        return y;
    }
};

struct Allpass {
    Line line;
    float gain;

    float run(float x) {
        const float d = line.read();
        const float v = x + d * gain;
        line.write_advance(v);
        return d - v * gain;
    }
};

}  // namespace

struct ReverbEffect::Network {
    bool echo = false;
    std::vector<Comb> combs;       // 4 per channel, left first
    std::vector<Allpass> allpass;  // 2 per channel
    std::vector<Line> lines;       // echo: one per channel
    float feedback = 0;

    std::array<float, 2> run(std::array<float, 2> x) {
        std::array<float, 2> out{};

        if (echo) {
            for (std::size_t ch = 0; ch < 2; ++ch) {
                const float y = lines[ch].read();
                lines[ch].write_advance(x[ch] + y * feedback);
                out[ch] = y;
            }

            return out;
        }

        for (std::size_t ch = 0; ch < 2; ++ch) {
            // Combs 0 and 2 take this side's send, 1 and 3 the other side's
            // (the hardware's same-side and cross-side paths).
            float sum = 0;

            for (std::size_t k = 0; k < 4; ++k) {
                sum += combs[ch * 4 + k].run(x[kCombs[k].first == 0 ? ch : 1 - ch]);
            }

            float y = sum * kCombOut;

            for (std::size_t k = 0; k < 2; ++k) {
                y = allpass[ch * 2 + k].run(y);
            }

            out[ch] = y;
        }

        return out;
    }
};

namespace {

float echo_feedback(ReverbMode mode, std::uint8_t feedback) {
    return mode == ReverbMode::Echo ? static_cast<float>(std::min<int>(feedback, 127)) / 128.0f
                                    : 0.0f;
}

}  // namespace

std::string_view reverb_mode_name(ReverbMode mode) {
    static constexpr std::string_view kNames[] = {
        "off", "room", "studio A", "studio B", "studio C", "hall", "space", "echo", "delay", "pipe"
    };
    const auto i = static_cast<std::size_t>(mode);
    return i < std::size(kNames) ? kNames[i] : "unknown";
}

std::optional<ReverbPreset> reverb_preset(ReverbMode mode) {
    auto p = [](float same,
                float cross,
                float rt60,
                float iir,
                std::array<float, 2> ap_ms,
                std::array<float, 2> ap_gain) {
        // The loop filter y += (x - y) * iir at 24 kHz; the same cut-off at
        // 48 kHz has the square-rooted pole.
        return ReverbPreset{{same, cross}, rt60, std::sqrt(1.0f - iir), ap_ms, ap_gain};
    };

    switch (mode) {
        case ReverbMode::Room:
            return p(69.3f, 64.0f, 0.78f, 0.855f, {20.8f, 15.2f}, {0.69f, 0.65f});
        case ReverbMode::StudioA:
            return p(33.3f, 70.5f, 0.93f, 0.882f, {8.5f, 6.2f}, {0.64f, 0.62f});
        case ReverbMode::StudioB:
            return p(68.0f, 150.5f, 0.88f, 0.882f, {29.5f, 21.2f}, {0.64f, 0.62f});
        case ReverbMode::StudioC:
            return p(112.3f, 244.7f, 2.17f, 0.870f, {37.8f, 28.2f}, {0.68f, 0.65f});
        case ReverbMode::Hall:
            return p(169.7f, 341.2f, 1.69f, 0.750f, {70.2f, 52.2f}, {0.75f, 0.72f});
        case ReverbMode::Space:
            return p(198.0f, 471.5f, 2.91f, 0.984f, {138.2f, 93.5f}, {0.75f, 0.66f});
        case ReverbMode::Pipe:
            return p(4.2f, 40.5f, 0.72f, 0.882f, {3.8f, 3.2f}, {0.75f, 0.66f});
        default:
            return std::nullopt;
    }
}

std::size_t echo_delay_samples(std::uint8_t delay) {
    return (static_cast<std::size_t>(std::min<std::uint8_t>(delay, 127)) + 1) * 256;
}

std::unique_ptr<ReverbEffect::Network> ReverbEffect::build_network(
    ReverbMode mode, std::uint8_t delay, std::uint8_t feedback
) {
    auto net = std::make_unique<ReverbEffect::Network>();

    if (mode == ReverbMode::Echo || mode == ReverbMode::Delay) {
        const std::size_t n = echo_delay_samples(delay);
        net->echo = true;
        net->lines.emplace_back(n);
        net->lines.emplace_back(n + kSpread);
        net->feedback = echo_feedback(mode, feedback);
        return net;
    }

    const std::optional<ReverbPreset> p = reverb_preset(mode);

    if (!p) {
        return nullptr;
    }

    auto samples = [](float ms) {
        return std::max<std::size_t>(
            static_cast<std::size_t>(std::lround(ms * kRate / 1000.0f)), 1
        );
    };

    for (std::size_t ch = 0; ch < 2; ++ch) {
        for (const auto& [which, scale] : kCombs) {
            const std::size_t n =
                samples(p->loops_ms[static_cast<std::size_t>(which)] * scale) + ch * kSpread;
            const float gain = std::pow(10.0f, -3.0f * static_cast<float>(n) / (p->rt60 * kRate));
            net->combs.push_back(Comb{Line(n), gain, p->damp});
        }
    }

    for (std::size_t ch = 0; ch < 2; ++ch) {
        for (std::size_t k = 0; k < 2; ++k) {
            net->allpass.push_back(
                Allpass{Line(samples(p->allpass_ms[k]) + ch * (kSpread / 2)), p->allpass_gain[k]}
            );
        }
    }

    return net;
}

ReverbEffect::ReverbEffect() = default;
ReverbEffect::~ReverbEffect() = default;
ReverbEffect::ReverbEffect(ReverbEffect&&) noexcept = default;
ReverbEffect& ReverbEffect::operator=(ReverbEffect&&) noexcept = default;

void ReverbEffect::set(ReverbMode mode, int depth, std::uint8_t delay, std::uint8_t feedback) {
    const bool delay_line = mode == ReverbMode::Echo || mode == ReverbMode::Delay;
    const bool rebuild = mode != m_mode || !m_network || (delay_line && delay != m_delay);
    m_mode = mode;
    m_delay = delay;
    m_feedback = feedback;

    if (rebuild) {
        m_network = build_network(mode, delay, feedback);
        m_lowpass = {};
    } else if (m_network && m_network->echo) {
        m_network->feedback = echo_feedback(mode, feedback);
    }

    m_depth = static_cast<float>(std::clamp(depth, 0, 0x7fff));
    m_target = m_depth;
    m_glide_left = 0;
}

void ReverbEffect::glide(int depth, int delta) {
    m_target = static_cast<float>(std::clamp(depth, 0, 0x7fff));
    const auto n = std::max<std::uint32_t>(static_cast<std::uint32_t>(std::max(delta, 0)) * 200, 1);
    m_step = (m_target - m_depth) / static_cast<float>(n);
    m_glide_left = n;
}

bool ReverbEffect::active() const {
    return m_enabled && m_network != nullptr;
}

std::array<int, 2> ReverbEffect::run(std::array<int, 2> send) {
    if (!active()) {
        return {0, 0};
    }

    if (m_glide_left > 0) {
        --m_glide_left;
        m_depth = m_glide_left == 0 ? m_target : m_depth + m_step;
    }

    std::array<float, 2> x{};

    for (std::size_t ch = 0; ch < 2; ++ch) {
        const auto s = static_cast<float>(send[ch]);
        m_lowpass[ch] = s + (m_lowpass[ch] - s) * kInputPole;
        x[ch] = m_lowpass[ch];
    }

    const std::array<float, 2> y = m_network->run(x);
    const float g = m_depth / 32768.0f;
    return {static_cast<int>(std::lround(y[0] * g)), static_cast<int>(std::lround(y[1] * g))};
}

}  // namespace openrac::audio
