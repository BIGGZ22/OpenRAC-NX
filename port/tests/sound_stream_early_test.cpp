// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// OPENRAC_STREAM_EARLY_START (port/game/common/lib/sound_engine.cpp): a stream read and decoded in
// parts sounds the same samples as one read and decoded whole. Run once without the variable and
// once with OPENRAC_STREAM_EARLY_START=1 (ctest: sound_stream_early, sound_stream_early_on); each
// checks the hash of what it rendered against the same pinned value. OPENRAC_AUDIO_TRACE=1 also
// walks the trace's tick report.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#include "openrac/sound_engine.h"
#include "tests/check.h"

extern "C" const char* openrac_game_disc_image;
const char* openrac_game_disc_image = nullptr;

namespace {

constexpr std::uint32_t kRate = 48000;     // the output's rate: one sample a frame, no blending
constexpr std::uint32_t kFrames = 9000;    // 16-byte ADPCM frames: 252,000 samples, 5.25 s
constexpr std::uint32_t kShortFrames = 3000;

void put_be32(std::vector<std::uint8_t>& v, std::size_t at, std::uint32_t x) {
    for (int i = 0; i < 4; ++i) {
        v[at + i] = static_cast<std::uint8_t>(x >> (24 - 8 * i));
    }
}

// A VAG at `sector` announcing kFrames frames, `body_frames` of them written: a signal of every
// filter and shift the disc uses, from a fixed seed.
void put_vag(std::vector<std::uint8_t>& disc, std::uint32_t sector, std::uint32_t body_frames) {
    const std::size_t at = static_cast<std::size_t>(sector) * 2048;
    disc.resize(std::max(disc.size(), at + 0x30 + std::size_t{body_frames} * 16), 0);
    std::memcpy(disc.data() + at, "VAGp", 4);
    put_be32(disc, at + 4, 0x20);
    put_be32(disc, at + 12, kFrames * 16);
    put_be32(disc, at + 16, kRate);
    std::uint32_t seed = 12345;
    for (std::uint32_t f = 0; f < body_frames; ++f) {
        std::uint8_t* frame = disc.data() + at + 0x30 + std::size_t{f} * 16;
        seed = seed * 1103515245u + 12345u;
        frame[0] = static_cast<std::uint8_t>(((seed >> 16) % 5) << 4 | ((seed >> 20) % 13));
        frame[1] = 0;
        for (int i = 2; i < 16; ++i) {
            seed = seed * 1103515245u + 12345u;
            frame[i] = static_cast<std::uint8_t>(seed >> 16);
        }
    }
}

// Renders `ms` milliseconds and folds them into `hash`; counts the samples that are not silent.
void render(int ms, std::uint64_t& hash, std::size_t& sounding, std::uint64_t& tick) {
    for (int done = 0; done < ms; done += 10) {
        std::vector<std::int16_t> out(static_cast<std::size_t>(kRate / 100) * 2);
        openrac_sound_render(out.data(), out.size() / 2);
        for (const std::int16_t v : out) {
            hash = (hash ^ static_cast<std::uint16_t>(v)) * 0x100000001b3ull;
            sounding += v != 0 ? 1 : 0;
        }
        openrac_sound_trace_tick(++tick, 60);
    }
}

}  // namespace

int main() {
    const char* early = std::getenv("OPENRAC_STREAM_EARLY_START");
    const bool early_on = early != nullptr && std::strcmp(early, "1") == 0;
    std::vector<std::uint8_t> disc;
    put_vag(disc, 4, kFrames);              // a whole track
    put_vag(disc, 800, kShortFrames);       // a track whose file ends before its header says
    disc.resize(800 * 2048 + 0x30 + kShortFrames * 16);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path()
        / ("openrac_sound_stream_early_test_"
           + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".iso");
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
    std::uint64_t tick = 0;

    // Read before play returns: every sample is there when the first is mixed, in either mode.
    openrac_stream_read_at_once(1);
    std::uint64_t at_once = 0xcbf29ce484222325ull;
    std::size_t at_once_sounding = 0;
    const std::uint32_t a = openrac_stream_play(4, 0x400, 1, 0, 0);
    CHECK(openrac_stream_buffered(a));
    render(5400, at_once, at_once_sounding, tick);
    CHECK(!openrac_stream_live(a));  // played to its end and gone
    CHECK(at_once_sounding > kFrames * 28 / 2);

    // Read on a thread, as in play: given the time to finish, the same samples.
    openrac_stream_read_at_once(0);
    const std::uint32_t b = openrac_stream_play(4, 0x400, 1, 0, 0);
    for (int i = 0; i < 500 && !openrac_stream_buffered(b); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    CHECK(openrac_stream_buffered(b));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::uint64_t threaded = 0xcbf29ce484222325ull;
    std::size_t threaded_sounding = 0;
    render(5400, threaded, threaded_sounding, tick);
    CHECK(threaded == at_once);
    CHECK(threaded_sounding == at_once_sounding);

    // Mixed while it is still being read (the reader and the mixer at once): with early start the
    // mixer may catch the reader up and hold silent, never play past it; either way the track is
    // played to its end and goes (and the sanitizers see the two threads share it).
    const std::uint32_t d = openrac_stream_play(4, 0x400, 1, 0, 0);
    std::uint64_t racing = 0xcbf29ce484222325ull;
    std::size_t racing_sounding = 0;
    // Offline mixing can outrun the reader by orders of magnitude. Bound this by wall time,
    // and let the reader run even on hosts with a coarse scheduler; silent holds are expected.
    const auto racing_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (openrac_stream_live(d) && std::chrono::steady_clock::now() < racing_deadline) {
        render(10, racing, racing_sounding, tick);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    CHECK(!openrac_stream_live(d));
    CHECK(racing_sounding == at_once_sounding);

    // Queued playback (the landing's speech is queued on the music's stream): a part queued on a
    // live stream starts when the part before it ends; a looping part plays on until a part is
    // queued behind it, then ends at the end of its pass. Both modes, every sample once a pass.
    openrac_stream_read_at_once(1);
    const std::uint32_t first = openrac_stream_play(4, 0x400, 1, 0, 0);
    const std::uint32_t second = openrac_stream_play(4, 0x400, 1, first, 0);
    std::uint64_t queued = 0xcbf29ce484222325ull;
    std::size_t queued_sounding = 0;
    for (int i = 0; i < 1200 && openrac_stream_live(second); ++i) {
        render(10, queued, queued_sounding, tick);
    }
    CHECK(!openrac_stream_live(second));
    CHECK(queued_sounding == 2 * at_once_sounding);

    const std::uint32_t loop = openrac_stream_play(4, 0x400, 1, 0, 4);
    std::uint64_t looped = 0xcbf29ce484222325ull;
    std::size_t looped_sounding = 0;
    render(8000, looped, looped_sounding, tick);  // past the 5.25 s track: it loops
    CHECK(openrac_stream_live(loop));
    const std::uint32_t after_loop = openrac_stream_play(4, 0x400, 1, loop, 0);
    for (int i = 0; i < 2000 && openrac_stream_live(after_loop); ++i) {
        render(10, looped, looped_sounding, tick);
    }
    CHECK(!openrac_stream_live(loop));
    CHECK(!openrac_stream_live(after_loop));
    // Two whole passes of the loop and the part after it.
    CHECK(looped_sounding == 3 * at_once_sounding);

    // A file shorter than its header: whole, it never sounds; in parts, what was read sounds
    // and the stream ends after it.
    openrac_stream_read_at_once(1);
    const std::uint32_t c = openrac_stream_play(800, 0x400, 1, 0, 0);
    std::uint64_t short_hash = 0xcbf29ce484222325ull;
    std::size_t short_sounding = 0;
    render(2000, short_hash, short_sounding, tick);
    if (early_on) {
        CHECK(short_sounding > 0);
        CHECK(short_sounding <= std::size_t{kShortFrames} * 28 * 2);
        CHECK(!openrac_stream_live(c));
    } else {
        CHECK(short_sounding == 0);
    }
    openrac_stream_stop(0);
    openrac_sound_stop_readers();

    // The mix of this signal (one sample a frame at the output's rate, so no blending), the same
    // whichever way the track was read: pinned, so that the two runs are compared to each other.
    CHECK(at_once == 0x2bd2a9fa9628097bull);
    std::printf("rendered hash %016llx, %zu samples sounding (early start %s)\n",
                static_cast<unsigned long long>(at_once), at_once_sounding, early_on ? "on" : "off");
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    return openrac::test::result();
}
