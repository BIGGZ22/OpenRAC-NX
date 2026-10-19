// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac/sound_engine.h: one SoundPlayer for the game, and the VAG streams (music, speech),
// behind a lock, since the audio output renders on its own thread.

#include "openrac/sound_engine.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

#include "assets/sound/adpcm.h"
#include "assets/sound/vag.h"
#include "audio/sound_player.h"
#include "common/log.h"
#include "openrac/game_host.h"

namespace {

using namespace openrac;

std::mutex g_lock;
// openrac_stream_read_at_once: streams read on the caller's thread.
bool g_read_at_once = false;

// A place on the disc image, which is larger than 2 GB: fseek takes a long, 32 bits on Windows,
// where a sector past the first million (the music) would wrap.
bool seek_disc(std::FILE* disc, std::uint64_t at) {
#if defined(_WIN32)
    return _fseeki64(disc, static_cast<long long>(at), SEEK_SET) == 0;
#else
    return fseeko(disc, static_cast<off_t>(at), SEEK_SET) == 0;
#endif
}

// Never destroyed: the output's thread may still render while the program exits.
audio::SoundPlayer& player() {
    static auto* p = new audio::SoundPlayer;
    return *p;
}

// The master volume of each group, as the game last set it (the streams apply it themselves).
std::array<std::int32_t, audio::kVolumeGroups> g_master = [] {
    std::array<std::int32_t, audio::kVolumeGroups> a{};
    a.fill(audio::kFullVolume);
    return a;
}();

// ---- VAG streams ----

// A track read from the disc and decoded on a thread of its own; `ready` (or `failed`) is set
// under g_lock.
struct Track {
    std::vector<std::int16_t> samples;
    std::uint32_t rate = 44100;
    // The samples the header announces (28 per 16-byte frame), known when the stream starts: the
    // time left before the decode has ended (openrac_stream_remaining_ms).
    std::size_t length = 0;
    bool ready = false;
    bool failed = false;
};

struct Part {
    std::shared_ptr<Track> track;
    bool loop = false;
};

// One stream: what it plays now and what is queued after it, under one or more handles.
struct Stream {
    std::vector<std::uint32_t> handles;
    std::deque<Part> parts;
    double position = 0.0;  // in the front part's samples
    std::int32_t volume = audio::kFullVolume;
    std::int32_t group = 1;
    bool paused = false;
};

std::vector<std::unique_ptr<Stream>> g_streams;
std::uint32_t g_next_stream = 1;

Stream* find_stream(std::uint32_t handle) {
    if ((handle & 0xFF000000u) != OPENRAC_STREAM_TAG) {
        return nullptr;
    }
    for (auto& s : g_streams) {
        if (std::find(s->handles.begin(), s->handles.end(), handle) != s->handles.end()) {
            return s.get();
        }
    }
    return nullptr;
}

// A mono VAG file at `sector`: {"VAGp", version, pad, data size, rate, ...} big-endian, 0x30 bytes.
constexpr std::size_t kVagHeader = 0x30;

// The body's size and the rate from the header at `sector`, or false.
bool read_vag_header(std::FILE* disc, std::uint32_t sector, std::uint32_t& size, std::uint32_t& rate) {
    std::uint8_t header[kVagHeader];
    if (!seek_disc(disc, static_cast<std::uint64_t>(sector) * 2048)
        || std::fread(header, 1, sizeof header, disc) != sizeof header
        || std::memcmp(header, "VAGp", 4) != 0) {
        return false;
    }
    const auto be32 = [&](int at) {
        return static_cast<std::uint32_t>(header[at]) << 24
               | static_cast<std::uint32_t>(header[at + 1]) << 16
               | static_cast<std::uint32_t>(header[at + 2]) << 8 | header[at + 3];
    };
    size = be32(12);
    rate = be32(16);
    return true;
}

void read_track(std::uint32_t sector, std::shared_ptr<Track> track) {
    std::vector<std::int16_t> samples;
    std::uint32_t rate = 0;
    if (std::FILE* disc =
            openrac_game_disc_image ? std::fopen(openrac_game_disc_image, "rb") : nullptr) {
        const std::uint64_t at = static_cast<std::uint64_t>(sector) * 2048;
        std::uint32_t size = 0;
        std::uint32_t header_rate = 0;
        if (read_vag_header(disc, sector, size, header_rate)) {
            std::vector<std::uint8_t> file(kVagHeader + size);
            if (size > 0 && size < 64u * 1024 * 1024 && seek_disc(disc, at)
                && std::fread(file.data(), 1, file.size(), disc) == file.size()) {
                try {
                    const assets::VagFile vag = assets::parse_vag(file);
                    rate = vag.header.sample_rate;
                    samples = assets::decode_adpcm(vag.body);
                } catch (const std::exception& e) {
                    log::warn("music: the stream at sector {}: {}", sector, e.what());
                }
            }
        }
        std::fclose(disc);
    }
    std::lock_guard lock(g_lock);
    if (samples.empty() || rate == 0) {
        log::warn("music: no VAG stream at sector {}", sector);
        track->failed = true;
        return;
    }
    track->samples = std::move(samples);
    track->rate = rate;
    track->ready = true;
}

// Adds the streams to `out`. A part not decoded yet holds its stream silent; an ended part gives
// way to the next, the position's fraction carried over.
void mix_streams(std::span<std::int16_t> out) {
    const std::size_t frames = out.size() / 2;
    for (auto it = g_streams.begin(); it != g_streams.end();) {
        Stream& s = **it;
        const double master =
            static_cast<double>(g_master[static_cast<std::size_t>(s.group)]) / audio::kFullVolume;
        const double gain = static_cast<double>(s.volume) / audio::kFullVolume * master * master;
        for (std::size_t f = 0; f < frames && !s.parts.empty() && !s.paused;) {
            const Track& t = *s.parts.front().track;
            if (t.failed) {
                s.parts.pop_front();
                s.position = 0.0;
                continue;
            }
            if (!t.ready) {
                break;
            }
            const std::size_t n = t.samples.size();
            const double step = static_cast<double>(t.rate) / audio::kOutputRate;
            for (; f < frames; ++f) {
                auto i = static_cast<std::size_t>(s.position);
                if (i >= n) {
                    break;
                }
                const double frac = s.position - static_cast<double>(i);
                const double a = t.samples[i];
                const double b = i + 1 < n ? t.samples[i + 1] : a;
                const double v = (a + (b - a) * frac) * gain;
                for (int c = 0; c < 2; ++c) {
                    const double mixed = out[f * 2 + c] + v;
                    out[f * 2 + c] =
                        static_cast<std::int16_t>(std::clamp(mixed, -32768.0, 32767.0));
                }
                s.position += step;
            }
            if (static_cast<std::size_t>(s.position) >= n) {
                s.position -= static_cast<double>(n);
                if (!s.parts.front().loop || s.parts.size() > 1) {
                    s.parts.pop_front();
                }
            }
        }
        it = s.parts.empty() ? g_streams.erase(it) : it + 1;
    }
}

std::int32_t word(const std::uint8_t* data, std::uint32_t i) {
    std::int32_t v;
    std::memcpy(&v, data + i * 4, 4);
    return v;
}

// The bank file's size from its header: {u32 type, u32 count, {u32 offset, u32 size}[count]}.
std::size_t bank_size(const std::uint8_t* header, std::size_t available) {
    if (available < 8) {
        return 0;
    }
    std::uint32_t count;
    std::memcpy(&count, header + 4, 4);
    if (count == 0 || count > 8 || available < 8 + count * 8) {
        return 0;
    }
    std::size_t end = 0;
    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t offset, size;
        std::memcpy(&offset, header + 8 + i * 8, 4);
        std::memcpy(&size, header + 12 + i * 8, 4);
        end = std::max<std::size_t>(end, std::size_t{offset} + size);
    }
    return end;
}

}  // namespace

extern "C" {

uint32_t openrac_sound_load_bank(const void* bytes, size_t size) {
    const auto* p = static_cast<const std::uint8_t*>(bytes);
    const std::size_t whole = bank_size(p, size);
    if (whole == 0 || whole > size) {
        log::warn("sound: a bank with a bad header");
        return 0;
    }
    std::lock_guard lock(g_lock);
    try {
        const audio::BankHandle bank = player().load_bank(std::span(p, whole));
        log::info("sound: bank {} loaded, {} sounds", bank, player().bank(bank)->sounds.size());
        return bank;
    } catch (const std::exception& e) {
        log::warn("sound: bank not loaded: {}", e.what());
        return 0;
    }
}

uint32_t openrac_sound_load_bank_from_disc(uint32_t sector, uint32_t offset) {
    if (openrac_game_disc_image == nullptr) {
        return 0;
    }
    std::FILE* disc = std::fopen(openrac_game_disc_image, "rb");
    if (disc == nullptr) {
        return 0;
    }
    const std::uint64_t at = static_cast<std::uint64_t>(sector) * 2048 + offset;
    std::uint8_t header[72];
    std::vector<std::uint8_t> file;
    if (seek_disc(disc, at)
        && std::fread(header, 1, sizeof header, disc) == sizeof header) {
        const std::size_t size = bank_size(header, sizeof header);
        file.resize(size);
        if (size == 0 || !seek_disc(disc, at)
            || std::fread(file.data(), 1, size, disc) != size) {
            file.clear();
        }
    }
    std::fclose(disc);
    if (file.empty()) {
        log::warn("sound: no bank at sector {} + {}", sector, offset);
        return 0;
    }
    return openrac_sound_load_bank(file.data(), file.size());
}

void openrac_sound_unload_bank(uint32_t bank) {
    std::lock_guard lock(g_lock);
    player().unload_bank(bank);
}

uint32_t openrac_sound_play(
    uint32_t bank, int32_t sound, int32_t volume, int32_t pan, int32_t pitch_mod, int32_t pitch_bend
) {
    audio::PlayRequest request;
    request.volume = volume;
    request.pan = pan;
    request.pitch_mod = pitch_mod;
    request.pitch_bend = pitch_bend;
    std::lock_guard lock(g_lock);
    if (player().bank(bank) == nullptr || sound < 0) {
        return 0;
    }
    return player().play(bank, static_cast<std::uint32_t>(sound), request);
}

uint32_t openrac_sound_set_params(
    uint32_t sound, int32_t mask, int32_t volume, int32_t pan, int32_t pitch_mod, int32_t pitch_bend
) {
    std::lock_guard lock(g_lock);
    if (Stream* stream = find_stream(sound)) {
        if ((mask & 1) != 0) {
            stream->volume = volume;
        }
        return sound;
    }
    audio::SoundPlayer& p = player();
    if (sound == 0 || !p.playing(sound)) {
        return 0;
    }
    if ((mask & 7) != 0) {
        p.set_volume_pan(
            sound,
            (mask & 1) != 0 ? volume : audio::kVolumeKeep,
            (mask & 6) != 0 ? pan : audio::kPanKeep
        );
    }
    if ((mask & 8) != 0) {
        p.set_pitch_mod(sound, pitch_mod);
    }
    if ((mask & 0x10) != 0) {
        p.set_pitch_bend(sound, pitch_bend);
    }
    return sound;
}

int openrac_sound_playing(uint32_t sound) {
    std::lock_guard lock(g_lock);
    return sound != 0 && player().playing(sound);
}

void openrac_sound_stop(uint32_t sound) {
    std::lock_guard lock(g_lock);
    player().stop(sound);
}

void openrac_sound_stop_all(void) {
    std::lock_guard lock(g_lock);
    player().stop_all();
}

void openrac_sound_set_master_volume(int32_t group, int32_t volume) {
    if (group < 0 || static_cast<std::size_t>(group) >= audio::kVolumeGroups) {
        return;
    }
    std::lock_guard lock(g_lock);
    g_master[static_cast<std::size_t>(group)] = volume;
    player().set_master_volume(static_cast<std::size_t>(group), volume);
}

void openrac_sound_pause_groups(uint32_t mask) {
    std::lock_guard lock(g_lock);
    player().pause_groups(mask);
}

void openrac_sound_continue_groups(uint32_t mask) {
    std::lock_guard lock(g_lock);
    player().continue_groups(mask);
}

uint32_t openrac_sound_command(uint32_t command, const void* data, uint32_t size) {
    const auto* d = static_cast<const std::uint8_t*>(data);
    switch (command) {
        case OPENRAC_SND_PLAY_VOL_PAN_PMPB:  // bank, sound, volume, pan, pitch mod, pitch bend
            if (size >= 24) {
                return openrac_sound_play(
                    static_cast<uint32_t>(word(d, 0)),
                    word(d, 1),
                    word(d, 2),
                    word(d, 3),
                    word(d, 4),
                    word(d, 5)
                );
            }
            return 0;
        case OPENRAC_SND_SET_SOUND_PARAMS:  // handle, mask, volume, pan, pitch mod, pitch bend
            if (size >= 24) {
                return openrac_sound_set_params(
                    static_cast<uint32_t>(word(d, 0)),
                    word(d, 1),
                    word(d, 2),
                    word(d, 3),
                    word(d, 4),
                    word(d, 5)
                );
            }
            return 0;
        default:
            return 0;
    }
}

uint32_t openrac_stream_play(
    uint32_t sector, int32_t volume, int32_t group, uint32_t queue_on, uint32_t flags
) {
    if (sector == 0) {
        return 0;
    }
    auto track = std::make_shared<Track>();
    // The header now, the body on the reader's thread: the game asks how long the stream has left
    // from the next frame on, long before a track of minutes is read and decoded.
    if (std::FILE* disc =
            openrac_game_disc_image ? std::fopen(openrac_game_disc_image, "rb") : nullptr) {
        std::uint32_t size = 0;
        std::uint32_t rate = 0;
        if (read_vag_header(disc, sector, size, rate) && rate != 0) {
            track->length = static_cast<std::size_t>(size / 16) * 28;
            track->rate = rate;
        }
        std::fclose(disc);
    }
    std::uint32_t handle;
    {
        std::lock_guard lock(g_lock);
        handle = OPENRAC_STREAM_TAG | (g_next_stream++ & 0x00FFFFFFu);
        Stream* stream = find_stream(queue_on);
        if (stream == nullptr) {
            g_streams.push_back(std::make_unique<Stream>());
            stream = g_streams.back().get();
            stream->volume = volume;
            stream->group = std::clamp<std::int32_t>(group, 0, audio::kVolumeGroups - 1);
            stream->paused = (flags & 1) != 0;
        }
        // A part queued behind a loop replaces the loop when the loop ends its pass.
        if (!stream->parts.empty() && stream->parts.back().loop) {
            stream->parts.back().loop = false;
        }
        stream->parts.push_back({track, (flags & 4) != 0});
        stream->handles.push_back(handle);
    }
    if (g_read_at_once) {
        read_track(sector, track);
    } else {
        std::thread(read_track, sector, track).detach();
    }
    return handle;
}

void openrac_stream_read_at_once(int at_once) {
    g_read_at_once = at_once != 0;
}

int openrac_stream_live(uint32_t stream) {
    std::lock_guard lock(g_lock);
    return find_stream(stream) != nullptr;
}

int openrac_stream_buffered(uint32_t stream) {
    std::lock_guard lock(g_lock);
    const Stream* s = find_stream(stream);
    return s != nullptr && !s->parts.empty() && s->parts.front().track->ready;
}

int32_t openrac_stream_remaining_ms(uint32_t stream) {
    std::lock_guard lock(g_lock);
    const Stream* s = find_stream(stream);
    if (s == nullptr || s->parts.empty()) {
        return 0;
    }
    const Track& t = *s->parts.front().track;
    // Not decoded yet: all of it is left, as its header gives it (the stream holds silent at its
    // start until then). RAC1's crossfade measures the old music's fade against the first answer
    // it gets for the transition (music_Update, case 2), so a made-up length there cut the old
    // music at once and started the next track with it.
    const std::size_t length = t.ready ? t.samples.size() : t.length;
    if (!t.ready && length == 0) {
        return 60000;  // no header either: the game only needs it to be playing
    }
    const double left = static_cast<double>(length) - s->position;
    return static_cast<int32_t>(std::max(0.0, left) * 1000.0 / t.rate);
}

void openrac_stream_stop(uint32_t stream) {
    std::lock_guard lock(g_lock);
    if (stream == 0) {
        g_streams.clear();
        return;
    }
    std::erase_if(g_streams, [&](const std::unique_ptr<Stream>& s) {
        return std::find(s->handles.begin(), s->handles.end(), stream) != s->handles.end();
    });
}

void openrac_stream_pause(uint32_t stream, int paused) {
    std::lock_guard lock(g_lock);
    if (Stream* s = find_stream(stream)) {
        s->paused = paused != 0;
    }
}

void openrac_stream_set_volume(uint32_t stream, int32_t volume) {
    std::lock_guard lock(g_lock);
    if (Stream* s = find_stream(stream)) {
        s->volume = volume;
    }
}

void openrac_sound_render(int16_t* interleaved, size_t frames) {
    std::lock_guard lock(g_lock);
    player().render(std::span(interleaved, frames * 2));
    mix_streams(std::span(interleaved, frames * 2));
}

}  // extern "C"
