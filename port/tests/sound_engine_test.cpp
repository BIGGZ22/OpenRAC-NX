// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The VAG streams of the game's sound engine (port/game/common/lib/sound_engine.cpp) on a disc
// image made here: the time a stream has left, from the frame it starts on, which RAC1's music
// crossfade measures its fade against (games/rac1/src/game/music.c, music_Update).

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "openrac/sound_engine.h"
#include "tests/check.h"

extern "C" const char* openrac_game_disc_image;
const char* openrac_game_disc_image = nullptr;

namespace {

constexpr std::uint32_t kRate = 48000;
constexpr std::uint32_t kFrames = 2000;  // 16-byte ADPCM frames: 56,000 samples, 1,166 ms
constexpr std::int32_t kWholeMs = static_cast<std::int32_t>(kFrames * 28ull * 1000 / kRate);

void put_be32(std::vector<std::uint8_t>& v, std::size_t at, std::uint32_t x) {
    for (int i = 0; i < 4; ++i) {
        v[at + i] = static_cast<std::uint8_t>(x >> (24 - 8 * i));
    }
}

// A VAG header at `sector` announcing kFrames frames, with `body_frames` of them written (silence).
void put_vag(std::vector<std::uint8_t>& disc, std::uint32_t sector, std::uint32_t body_frames) {
    const std::size_t at = static_cast<std::size_t>(sector) * 2048;
    disc.resize(std::max(disc.size(), at + 0x30 + body_frames * 16), 0);
    disc[at] = 'V';
    disc[at + 1] = 'A';
    disc[at + 2] = 'G';
    disc[at + 3] = 'p';
    put_be32(disc, at + 4, 0x20);
    put_be32(disc, at + 12, kFrames * 16);
    put_be32(disc, at + 16, kRate);
}

void render_ms(int ms) {
    std::vector<std::int16_t> out(static_cast<std::size_t>(kRate / 1000 * ms) * 2);
    openrac_sound_render(out.data(), out.size() / 2);
}

}  // namespace

int main() {
    std::vector<std::uint8_t> disc;
    put_vag(disc, 4, kFrames);   // a whole track
    put_vag(disc, 200, 0);       // a header whose body is past the image's end
    disc.resize(200 * 2048 + 0x30);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("openrac_sound_engine_test_" +
                                                  std::to_string(std::chrono::steady_clock::now()
                                                                     .time_since_epoch()
                                                                     .count()) +
                                                  ".iso");
    {
        std::FILE* f = std::fopen(path.string().c_str(), "wb");
        CHECK(f != nullptr);
        if (f == nullptr) {
            return openrac::test::result();
        }
        std::fwrite(disc.data(), 1, disc.size(), f);
        std::fclose(f);
    }
    const std::string image = path.string();
    openrac_game_disc_image = image.c_str();

    // Read on its own thread, as in play: the answer the frame it starts is the whole track,
    // decoded or not (it was a made-up minute while the decode ran).
    openrac_stream_read_at_once(0);
    const std::uint32_t h = openrac_stream_play(4, 0x400, 1, 0, 0x20);
    CHECK(h != 0);
    const std::int32_t first = openrac_stream_remaining_ms(h);
    CHECK(first == kWholeMs);
    for (int i = 0; i < 500 && !openrac_stream_buffered(h); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CHECK(openrac_stream_buffered(h));
    CHECK(openrac_stream_remaining_ms(h) == first);

    // The game's crossfade (music_Update, case 2): the old music at vol * (interval - played) /
    // interval, interval a quarter of the first answer. An eighth in, it is at half, not cut.
    const std::int32_t interval = first / 4;
    render_ms(first / 8);
    const std::int32_t now = openrac_stream_remaining_ms(h);
    CHECK(now <= first - first / 8 + 1 && now >= first - first / 8 - 1);
    const std::int32_t fade = 0x400 * (interval - (first - now)) / interval;
    CHECK(fade > 0x400 / 2 - 0x20 && fade < 0x400 / 2 + 0x20);
    openrac_stream_stop(h);
    CHECK(!openrac_stream_live(h));

    // Not decoded (here never: its body is not on the image): still the header's length.
    openrac_stream_read_at_once(1);
    const std::uint32_t broken = openrac_stream_play(200, 0x400, 1, 0, 0x20);
    CHECK(openrac_stream_remaining_ms(broken) == kWholeMs);
    openrac_stream_stop(0);

    // The program is leaving: the readers are stopped and waited for, and no track is read
    // after that (a reader that is still working when the program exits is a crash at quit).
    openrac_stream_read_at_once(0);
    const std::uint32_t late = openrac_stream_play(4, 0x400, 1, 0, 0x20);
    CHECK(late != 0);
    openrac_sound_stop_readers();
    // Stopping again is harmless, and the track is not read after the stop.
    openrac_sound_stop_readers();
    const std::uint32_t after = openrac_stream_play(4, 0x400, 1, 0, 0x20);
    CHECK(after != 0);
    CHECK(!openrac_stream_buffered(after));
    openrac_stream_stop(0);

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    return openrac::test::result();
}
