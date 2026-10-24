// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game's sounds on the player's device (program.h): the output opened at the start, watched
// each frame and opened again when it stalls or was missing, closed before the program's globals
// go. With OPENRAC_AUDIO_TICKED there is no device, and each tick renders its own samples.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "common/env.h"
#include "common/log.h"
#include "openrac/sound_engine.h"
#include "program/program.h"

#ifdef OPENRAC_FRONTEND
#include "platform/audio.h"
#include "settings.h"
#endif

namespace openrac::program {

#ifdef OPENRAC_FRONTEND
namespace {

// The game's sounds (openrac/sound_engine.h), rendered on the output's thread.
std::unique_ptr<platform::AudioOutput> g_sound;
long g_sound_retry_frame = 0;

}  // namespace
#endif

bool audio_ticked() {
    return env::flag(env::Var::AudioTicked);
}

void audio_tick() {
#ifdef OPENRAC_FRONTEND
    if (!audio_ticked() || !g_window) {
        return;
    }

    static std::vector<std::int16_t> samples(
        static_cast<std::size_t>(platform::kAudioRate / tick_rate()) * 2
    );
    openrac_sound_render(samples.data(), samples.size() / 2);
#endif
}

#ifdef OPENRAC_FRONTEND
void close_sound() {
    g_sound.reset();
}

void open_sound(bool report) {
    std::string why;
    g_sound = platform::AudioOutput::open(why, [](std::span<std::int16_t> samples) {
        openrac_sound_render(samples.data(), samples.size() / 2);

        // The player's volume (settings.h): at 0 the output is silence itself.
        if (const float gain = settings::output_gain(); gain < 1.0f) {
            for (std::int16_t& sample : samples) {
                sample = static_cast<std::int16_t>(static_cast<float>(sample) * gain);
            }
        }
    });

    if (!g_sound && report) {
        log::warn("no sound: {}", why);
    }
}

void watch_sound() {
    if (!g_window || audio_ticked()) {
        return;
    }

    if (!g_sound) {
        if (g_frame >= g_sound_retry_frame) {
            g_sound_retry_frame = g_frame + 300;
            open_sound(false);

            if (g_sound) {
                log::info("sound is back");
            }
        }

        return;
    }

    std::string why;
    static bool reported = false;

    if (!g_sound->watch(why)) {
        if (!reported) {
            log::warn("sound stopped and the device could not be opened again: {}", why);
            reported = true;
        }
    } else if (reported) {
        log::info("sound is back");
        reported = false;
    }
}
#endif

}  // namespace openrac::program
