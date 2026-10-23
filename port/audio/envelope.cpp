// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-game/src/audio.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The ADSR envelope (envelope.h).

#include "audio/envelope.h"

#include <algorithm>

namespace openrac::audio {

void Envelope::update_settings() {
    const std::uint32_t r = static_cast<std::uint32_t>(m_adsr2) << 16 | m_adsr1;
    auto bits = [r](unsigned pos, unsigned n) {
        return static_cast<int>((r >> pos) & ((1u << n) - 1));
    };

    // Each phase reads its own fields of the two registers: a different bit layout for attack,
    // decay,
    // sustain and release, so the settings are loaded again whenever the phase changes.
    switch (m_phase) {
        case EnvelopePhase::Attack:
            m_exponential = bits(15, 1) != 0;
            m_decrease = false;
            m_shift = bits(10, 5);
            m_step = 7 - bits(8, 2);
            m_target = 0x7fff;
            break;
        case EnvelopePhase::Decay:
            m_exponential = true;
            m_decrease = true;
            m_shift = bits(4, 4);
            m_step = -8;
            m_target = (bits(0, 4) + 1) << 11;
            break;
        case EnvelopePhase::Sustain: {
            m_exponential = bits(31, 1) != 0;
            m_decrease = bits(30, 1) != 0;
            m_shift = bits(24, 5);
            const int s = bits(22, 2);
            m_step = m_decrease ? -8 + s : 7 - s;
            m_target = 0;
            break;
        }
        case EnvelopePhase::Release:
            m_exponential = bits(21, 1) != 0;
            m_decrease = true;
            m_shift = bits(16, 5);
            m_step = -8;
            m_target = 0;
            break;
        case EnvelopePhase::Stopped:
            break;
    }
}

void Envelope::attack() {
    m_phase = EnvelopePhase::Attack;
    m_level = 0;
    m_counter = 0;
    update_settings();
}

void Envelope::release() {
    // A voice that has already stopped has nothing to release, and must not restart a counter.
    if (m_phase == EnvelopePhase::Stopped) {
        return;
    }

    m_phase = EnvelopePhase::Release;
    m_counter = 0;
    update_settings();
}

void Envelope::stop() {
    m_phase = EnvelopePhase::Stopped;
    m_level = 0;
}

void Envelope::step() {
    std::uint32_t counter_step = 0x80'0000;

    // Above shift 11 the level steps less often than every sample: the counter takes longer to
    // fill.
    if (m_shift > 11) {
        counter_step >>= m_shift - 11;
    }

    // The step as the hardware's signed 16-bit value.
    int step = static_cast<std::int16_t>(m_step << std::max(0, 11 - m_shift));

    // Only the exponential mode is bent by the level; the linear mode steps by a constant.
    if (m_exponential) {
        // An exponential rise slows to a quarter of its rate above 0x6000, as the hardware does.
        if (!m_decrease && m_level > 0x6000) {
            counter_step >>= 2;
        }

        // An exponential fall steps in proportion to the current level, so it decays smoothly.
        if (m_decrease) {
            step = (step * m_level) >> 15;
        }
    }

    m_counter += counter_step;

    // The level moves only when the counter has filled; the other samples leave it as it is.
    if (m_counter >= 0x80'0000) {
        m_counter = 0;
        m_level = std::clamp(m_level + step, 0, 0x7fff);
    }
}

void Envelope::run() {
    // A stopped envelope stays at level zero and needs no update.
    if (m_phase == EnvelopePhase::Stopped) {
        return;
    }

    step();

    // Sustain has no target: it holds (or slides) until a release, so the phase never advances.
    if (m_phase == EnvelopePhase::Sustain) {
        return;
    }

    const bool reached = m_decrease ? m_level <= m_target : m_level >= m_target;

    // Phase change only once the level has crossed the phase's target, in the direction it moves.
    if (reached) {
        // The next phase after the one that reached its target; sustain and stopped never get here.
        switch (m_phase) {
            case EnvelopePhase::Attack:
                m_phase = EnvelopePhase::Decay;
                break;
            case EnvelopePhase::Decay:
                m_phase = EnvelopePhase::Sustain;
                break;
            case EnvelopePhase::Release:
                m_phase = EnvelopePhase::Stopped;
                break;
            default:
                break;
        }

        update_settings();
    }
}

}  // namespace openrac::audio
