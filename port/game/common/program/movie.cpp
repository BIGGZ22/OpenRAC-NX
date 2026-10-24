// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game's movies in the window (program::play_movie, openrac_game_play_movie's body in
// program/game_host.cpp): a PSS movie read from the player's disc image, its picture shown and its
// sound played, blocking as the game's own player does. Without a window or without the movie
// support (OPENRAC_MOVIES) there is none, and the game goes on as if it had played.
// The player follows ReRAC's movie mode (ISC License): the comment above play_movie credits it.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "common/log.h"
#include "openrac/game_host.h"
#include "program/program.h"

#if defined(OPENRAC_FRONTEND) && defined(OPENRAC_MOVIES)
#include "frontend.h"
#include "media/movie.h"
#include "platform/audio.h"
#include "settings.h"
#endif

namespace openrac::program {

/*
 * The movie player, after ReRAC's movie mode (crates/rc-game/src/movie_player.rs,
 * crates/rc-engine/src/movie_render.rs; ISC License, Copyright (c) 2026 ReRAC contributors):
 * the audio is the clock. Each vertical blank hands the device one blank's worth of the movie's
 * 48 kHz audio, and the picture shown is the one whose time holds the sample being heard (what has
 * been handed over less what the device still holds), so picture and sound cannot drift apart.
 * When the video ends (or is skipped) the last picture stays up for the console's FadeToBlack(4).
 */
int play_movie(uint32_t lsn, uint32_t bytes, int channel, int start_skips) {
    if (g_options.skip_movies) {
        log::info("movie at sector {}: skipped by --skip-movies (development probe)", lsn);
        return 1;  // the same result as the movie player's Start-skip path
    }
#if defined(OPENRAC_FRONTEND) && defined(OPENRAC_MOVIES)
    if (!g_window || openrac_game_disc_image == nullptr || bytes == 0) {
        return 0;
    }
    std::vector<std::uint8_t> file(bytes);
    {
        std::FILE* disc = std::fopen(openrac_game_disc_image, "rb");
        if (disc == nullptr) {
            return 0;
        }
#if defined(_WIN32)
        // Windows' C library has no fseeko; _fseeki64 is its 64-bit seek.
        const bool sought = _fseeki64(disc, static_cast<long long>(lsn) * 2048, SEEK_SET) == 0;
#else
        const bool sought = fseeko(disc, static_cast<off_t>(lsn) * 2048, SEEK_SET) == 0;
#endif
        const bool read = sought && std::fread(file.data(), 1, file.size(), disc) == file.size();
        std::fclose(disc);
        if (!read) {
            log::warn("movie at sector {}: the disc image is too short", lsn);
            return 0;
        }
    }
    std::optional<media::MoviePlayer> player;
    try {
        player.emplace(media::Movie::open(file, static_cast<std::uint8_t>(channel)));
    } catch (const std::exception& e) {
        log::warn("movie at sector {}: {}", lsn, e.what());
        return 0;
    }
    const media::VideoSequence& sequence = player->movie().sequence();
    log::info(
        "movie at sector {}: {}x{}, {}/{} fps, audio channel {}", lsn, sequence.width, sequence.height,
        sequence.fps_num, sequence.fps_den, player->movie().audio_channel() ? int(*player->movie().audio_channel()) : -1
    );
    std::string error;
    auto output = platform::AudioOutput::open(error);
    if (!output) {
        log::warn("movie: no sound ({})", error);
    }
    const int rate = tick_rate();
    const std::size_t per_blank = static_cast<std::size_t>(platform::kAudioRate / rate);
    std::vector<std::int16_t> samples(per_blank * 2);
    std::vector<std::uint8_t> rgba;
    std::shared_ptr<const media::VideoFrame> shown;
    std::uint16_t previous = 0xFFFF;
    int skipped = 0;
    auto pace = [&] {
        using namespace std::chrono;
        const auto now = steady_clock::now();
        if (g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + milliseconds(100)) {
            g_next_frame = now;
        }
        g_next_frame += microseconds(1000000 / rate);
        std::this_thread::sleep_until(g_next_frame);
        g_frame++;
    };
    for (std::uint64_t blank = 0;; ++blank) {
        if (output) {
            output->set_gain(settings::output_gain());
            const std::size_t got = player->read_audio(samples);
            if (got > 0) {
                output->queue(std::span<const std::int16_t>(samples.data(), got * 2));
            }
        }
        const std::uint64_t handed = (blank + 1) * per_blank;
        const std::uint64_t held = output ? static_cast<std::uint64_t>(output->queued_frames()) : 0;
        const double heard = static_cast<double>(handed > held ? handed - held : 0) / platform::kAudioRate;
        auto frame = player->frame_at(heard);
        if (frame) {
            shown = frame;
        }
        if (player->video_finished()) {
            break;
        }
        if (shown) {
            media::frame_to_rgba(*shown, rgba);
            if (!frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), 0.0f)) {
                leave();
            }
        }
        // Start skips when pressed during the movie: what is held as it starts is not a press.
        std::uint16_t buttons = 0xFFFF;
        std::uint8_t analog[4];
        openrac_game_pad_through_mods(0, &buttons, analog);  // a mod's key bound to Start skips too
        if (blank == 0) {
            previous = buttons;
        }
        const bool start_now = (buttons & 0x0008) == 0;
        const bool start_before = (previous & 0x0008) == 0;
        previous = buttons;
        if (start_skips && start_now && !start_before) {
            log::info("movie at sector {}: Start pressed (pad {:#06x})", lsn, buttons);
            skipped = 1;
            break;
        }
        pace();
    }
    // FadeToBlack(4) over the last picture, then its closing black blank.
    for (int step = 0; step <= 4 && shown; ++step) {
        frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), step >= 4 ? 1.0f : float(step + 1) / 4.0f);
        pace();
    }
    if (output) {
        output->pause(true);
    }
    log::info("movie at sector {}: {}", lsn, skipped ? "skipped" : "played");
    return skipped;
#else
    (void)lsn;
    (void)bytes;
    (void)channel;
    (void)start_skips;
    return 0;
#endif
}

}  // namespace openrac::program
