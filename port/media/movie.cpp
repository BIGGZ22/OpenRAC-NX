// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/movie.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The movie, its player and the colour conversion (movie.h).

#include "media/movie.h"

#include <algorithm>
#include <cmath>

#include "assets/bytes.h"
#include "assets/sound/pss.h"

namespace openrac::media {

Movie Movie::open(std::span<const std::uint8_t> file, std::uint8_t channel) {
    assets::PssStreams streams = assets::demux_pss(assets::ByteView(file));
    Movie m;

    for (const auto& [c, bytes] : streams.audio) {
        m.m_channels.push_back(c);
    }

    if (streams.audio.contains(channel)) {
        m.m_audio_channel = channel;
    } else if (streams.audio.contains(0)) {
        m.m_audio_channel = 0;
    } else if (!m.m_channels.empty()) {
        m.m_audio_channel = m.m_channels.front();
    }

    if (m.m_audio_channel) {
        const auto audio =
            assets::PssAudio::parse(assets::ByteView(streams.audio.at(*m.m_audio_channel)));
        m.m_audio_rate = audio.header.rate;
        const std::vector<std::int16_t> pcm = audio.decode();
        m.m_audio = resample_stereo(pcm, audio.header.rate, kMovieOutputRate);
    }

    m.m_decoder = std::make_unique<Mpeg2Decoder>(std::move(streams.video));
    const auto sequence = m.m_decoder->sequence();

    if (!sequence) {
        throw VideoError(0, "no sequence header in the video stream");
    }

    m.m_sequence = *sequence;
    return m;
}

std::size_t MoviePlayer::read_audio(std::span<std::int16_t> out) {
    const std::vector<std::int16_t>& audio = m_movie.audio();
    const std::size_t total = audio.size() / 2;
    const std::size_t wanted = out.size() / 2;
    const std::size_t at = m_audio_cursor.load();
    const std::size_t n = std::min(wanted, total - std::min(at, total));
    std::copy_n(audio.data() + 2 * at, 2 * n, out.data());
    std::fill(out.begin() + static_cast<std::ptrdiff_t>(2 * n), out.end(), std::int16_t{0});
    m_audio_cursor.store(at + n);
    return n;
}

double MoviePlayer::audio_time() const {
    return static_cast<double>(m_audio_cursor.load()) / kMovieOutputRate;
}

bool MoviePlayer::audio_finished() const {
    return m_audio_cursor.load() >= m_movie.audio().size() / 2;
}

std::shared_ptr<const VideoFrame> MoviePlayer::frame_at(double seconds) {
    const double fps = m_movie.fps();
    const auto due = static_cast<std::size_t>(std::max(0.0, std::floor(seconds * fps)));

    while (!m_video_ended && m_shown <= due) {
        auto frame = m_movie.next_frame();

        if (!frame) {
            m_video_ended = true;
            break;
        }

        m_current = std::move(frame);
        ++m_shown;
    }

    return m_current;
}

std::vector<std::int16_t> resample_stereo(
    std::span<const std::int16_t> pcm, std::uint32_t from, std::uint32_t to
) {
    const std::size_t frames = pcm.size() / 2;

    if (from == to || frames == 0) {
        return {pcm.begin(), pcm.end()};
    }

    const std::size_t n = static_cast<std::size_t>(static_cast<std::uint64_t>(frames) * to / from);
    std::vector<std::int16_t> out(2 * n);

    for (std::size_t i = 0; i < n; ++i) {
        const std::uint64_t pos = static_cast<std::uint64_t>(i) * from;
        const auto k = static_cast<std::size_t>(pos / to);
        const auto frac = static_cast<std::int64_t>(pos % to);
        const std::size_t k1 = std::min(k + 1, frames - 1);

        for (std::size_t c = 0; c < 2; ++c) {
            const std::int64_t a = pcm[2 * k + c];
            const std::int64_t b = pcm[2 * k1 + c];
            out[2 * i + c] =
                static_cast<std::int16_t>(a + (b - a) * frac / static_cast<std::int64_t>(to));
        }
    }

    return out;
}

namespace {

// BT.601 limited range to full-range RGB, 16.16 fixed point.
constexpr int kLuma = 76'309;    // 255 / 219
constexpr int kRedV = 104'597;   // 1.402 * 255 / 224
constexpr int kGreenU = 25'675;  // 0.344136 * 255 / 224
constexpr int kGreenV = 53'279;  // 0.714136 * 255 / 224
constexpr int kBlueU = 132'201;  // 1.772 * 255 / 224

std::uint8_t to_byte(int x) {
    return static_cast<std::uint8_t>(std::clamp(x >> 16, 0, 255));
}

}  // namespace

std::array<std::uint8_t, 3> ycbcr_to_rgb(std::uint8_t y, std::uint8_t cb, std::uint8_t cr) {
    const int yy = (y - 16) * kLuma + 32'768;
    const int u = cb - 128;
    const int v = cr - 128;
    return {
        to_byte(yy + kRedV * v), to_byte(yy - kGreenU * u - kGreenV * v), to_byte(yy + kBlueU * u)
    };
}

void frame_to_rgba(const VideoFrame& frame, std::vector<std::uint8_t>& out) {
    const std::size_t w = frame.width;
    const std::size_t h = frame.height;
    const std::size_t cw = w / 2;
    out.resize(w * h * 4);

    for (std::size_t y = 0; y < h; ++y) {
        const std::uint8_t* row = frame.y.data() + y * w;
        const std::uint8_t* cb = frame.cb.data() + (y / 2) * cw;
        const std::uint8_t* cr = frame.cr.data() + (y / 2) * cw;
        std::uint8_t* o = out.data() + y * w * 4;

        for (std::size_t x = 0; x < w; ++x) {
            const auto rgb = ycbcr_to_rgb(row[x], cb[x / 2], cr[x / 2]);
            o[4 * x + 0] = rgb[0];
            o[4 * x + 1] = rgb[1];
            o[4 * x + 2] = rgb[2];
            o[4 * x + 3] = 255;
        }
    }
}

}  // namespace openrac::media
