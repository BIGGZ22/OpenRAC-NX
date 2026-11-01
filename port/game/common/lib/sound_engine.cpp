// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac/sound_engine.h: one SoundPlayer for the game, and the VAG streams (music, speech),
// behind a lock, since the audio output renders on its own thread.

#include "openrac/sound_engine.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <deque>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
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
// The threads reading a track, and the flag that tells them the program is leaving. A track of
// minutes is read on a thread of its own, and the player may close the window while one is
// reading: the program then waits for them (openrac_sound_stop_readers), so that the globals
// this file has are not taken away under a thread that is using them.
// A reader that has finished says so (done), and the next track's start joins it: a thread
// that ended is not left unjoined, holding its stack, for the rest of the session.
struct Reader {
    std::thread thread;
    std::shared_ptr<std::atomic<bool>> done;
};
std::vector<Reader> g_readers;
bool g_stopping = false;

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
    bool trace=false;
    std::uint32_t sector=0;
    std::int64_t requested_ns=0;
    // OPENRAC_AUDIO_TRACE's tick view (openrac_sound_trace_tick), game thread only: the tick the
    // stream was asked for, the tick its first sound was seen at, the next tick to report at.
    std::uint64_t requested_tick=0, sounding_tick=0, report_tick=0;
    unsigned reports=0;
    mutable std::atomic<std::int64_t> consumed_ns{0};
    mutable std::atomic<bool> consumed_reported{false};
    std::vector<std::int16_t> samples;
    std::uint32_t rate = 44100;
    // The samples the header announces (28 per 16-byte frame), known when the stream starts: the
    // time left before the decode has ended (openrac_stream_remaining_ms).
    std::size_t length = 0;
    bool ready = false;
    bool failed = false;
    // The samples the mixer may play (under g_lock): all of them once ready, or, with
    // OPENRAC_STREAM_EARLY_START, those decoded so far while `complete` is false (the mixer then
    // holds at the end of them, silent, until the reader has more).
    std::size_t available = 0;
    bool complete = false;
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

std::int64_t audio_clock_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// The tick openrac_sound_trace_tick last saw (game thread; 0 before the first).
std::uint64_t g_trace_tick = 0;

// OPENRAC_STREAM_EARLY_START=1 (experiment, off by default): a stream starts to sound once its
// first part is decoded, the rest decoded in parts while it plays, instead of after the whole file
// has been read and decoded (read_track). A track of minutes is a megabyte or more read from the SD
// card and decoded before its first sound; the game's scene goes on meanwhile, so the speech of a
// scene starts late (the landing's). The samples are the same; only when they may first sound
// changes. The mixer never plays past what is decoded: should the reader fall behind, the stream
// holds silent where it is, as it held before its first sound.
bool stream_early_start() {
    static const bool on=[] {
        const auto* v=std::getenv("OPENRAC_STREAM_EARLY_START");
        const bool set=v && std::strcmp(v,"1")==0;
        if(set) openrac::log::info("sound: EXPERIMENT OPENRAC_STREAM_EARLY_START=1: streams sound from their first decoded part");
        return set;
    }();
    return on;
}

bool audio_trace_enabled() {
    static const bool on=[] {
        const auto* v=std::getenv("OPENRAC_AUDIO_TRACE");return v && std::strcmp(v,"1")==0;
    }();
    return on;
}

// Traced tracks whose first mixer consumption has not been logged yet (g_lock). The mixer only
// stores a timestamp in the track; game-thread calls drain this list and log outside the lock, so
// a track the game never queries (a cutscene's speech) is still reported.
std::vector<std::shared_ptr<Track>> g_traced;

// Called from game-side queries only. Never write files from the mixer callback.
void report_consumed(const std::shared_ptr<Track>& track) {
    if(!track || !track->trace) return;
    const auto at=track->consumed_ns.load(std::memory_order_relaxed);
    if(at && !track->consumed_reported.exchange(true))
        log::info("audio trace sector {}: first mixer consumption at {:.3f} ms, {:.3f} ms after request (not speaker latency)",
            track->sector,double(at)/1e6,double(at-track->requested_ns)/1e6);
}

// Game thread only (stream calls and sound commands): never from the mixer.
void drain_traced() {
    if(!audio_trace_enabled()) return;
    std::vector<std::shared_ptr<Track>> consumed;
    {
        std::lock_guard lock(g_lock);
        std::erase_if(g_traced,[&](const std::shared_ptr<Track>& t) {
            if(t->consumed_ns.load(std::memory_order_relaxed)) { consumed.push_back(t); return true; }
            // Failed, or no stream or reader holds it any more: it will never be consumed.
            return t->failed || t.use_count()==1;
        });
    }
    for(const auto& t:consumed) report_consumed(t);
}

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
    const auto started=track->trace ? audio_clock_ns() : 0;
    std::int64_t read_done=started,decode_done=started;
    // The program is leaving: nothing to read for.
    {
        std::lock_guard lock(g_lock);
        if (g_stopping) {
            track->failed = true;
            return;
        }
    }
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
                if(track->trace) read_done=audio_clock_ns();
                try {
                    const assets::VagFile vag = assets::parse_vag(file);
                    rate = vag.header.sample_rate;
                    samples = assets::decode_adpcm(vag.body);
                    if(track->trace) decode_done=audio_clock_ns();
                } catch (const std::exception& e) {
                    log::warn("music: the stream at sector {}: {}", sector, e.what());
                }
            }
        }
        std::fclose(disc);
    }
    {
        std::lock_guard lock(g_lock);
        if (samples.empty() || rate == 0) {
            log::warn("music: no VAG stream at sector {}", sector);
            track->failed = true;
            return;
        }
        track->samples = std::move(samples);
        track->rate = rate;
        track->available = track->samples.size();
        track->complete = true;
        track->ready = true;
    }
    if(track->trace) log::info("audio trace sector {}: read/header {:.3f} ms, decode {:.3f} ms, ready {:.3f} ms after request; ready at {:.3f} ms",
        sector,double(read_done-started)/1e6,double(decode_done-read_done)/1e6,
        double(audio_clock_ns()-track->requested_ns)/1e6,double(audio_clock_ns())/1e6);
}

// read_track for OPENRAC_STREAM_EARLY_START: the body read and decoded in parts, each published to
// the mixer as it is done (Track::available), the first small so that the stream sounds soon. The
// decoding is read_track's (assets::decode_adpcm: every whole 16-byte frame from zero history,
// rounded), frame by frame with the history carried, so the samples are the same.
void read_track_early(std::uint32_t sector, std::shared_ptr<Track> track) {
    const auto started=track->trace ? audio_clock_ns() : 0;
    {
        std::lock_guard lock(g_lock);
        if (g_stopping) {
            track->failed = true;
            return;
        }
    }
    std::FILE* disc = openrac_game_disc_image ? std::fopen(openrac_game_disc_image, "rb") : nullptr;
    std::uint32_t size = 0;
    std::uint32_t rate = 0;
    const bool header = disc != nullptr && read_vag_header(disc, sector, size, rate) && rate != 0
                        && size >= assets::kAdpcmFrameBytes && size < 64u * 1024 * 1024;
    if (!header) {
        if (disc != nullptr) {
            std::fclose(disc);
        }
        std::lock_guard lock(g_lock);
        log::warn("music: no VAG stream at sector {}", sector);
        track->failed = true;
        return;
    }
    const std::size_t frames = size / assets::kAdpcmFrameBytes;
    // Sized once, before the mixer can see it: the reader writes past `available`, the mixer reads
    // before it, and the vector itself does not change again.
    std::vector<std::int16_t> whole(frames * assets::kAdpcmFrameSamples);
    std::int16_t* const out = whole.data();
    // The first part about a tenth of a second at 44.1 kHz; the rest in parts of about 2.6 s.
    constexpr std::size_t kFirstFrames = 160;
    constexpr std::size_t kPartFrames = 4096;
    std::vector<std::uint8_t> part(kPartFrames * assets::kAdpcmFrameBytes);
    assets::AdpcmHistory history;
    std::size_t done = 0;
    bool published = false;
    bool short_read = false;
    std::int64_t first_ns = 0;
    if (!seek_disc(disc, static_cast<std::uint64_t>(sector) * 2048 + kVagHeader)) {
        short_read = true;
    }
    while (!short_read && done < frames) {
        const std::size_t want = std::min(published ? kPartFrames : kFirstFrames, frames - done);
        const std::size_t got = std::fread(part.data(), 1, want * assets::kAdpcmFrameBytes, disc);
        // The whole frames read; a file that ends before its header says ends the track there.
        const std::size_t take = got / assets::kAdpcmFrameBytes;
        short_read = take < want;
        if (take == 0) {
            break;
        }
        for (std::size_t f = 0; f < take; ++f) {
            const auto pcm = assets::decode_adpcm_frame(
                part.data() + f * assets::kAdpcmFrameBytes, history, assets::AdpcmRounding::Rounded
            );
            std::copy(pcm.begin(), pcm.end(), out + (done + f) * assets::kAdpcmFrameSamples);
        }
        done += take;
        std::lock_guard lock(g_lock);
        if (!published) {
            track->samples = std::move(whole);
            track->rate = rate;
            track->ready = true;
            published = true;
            if (track->trace) first_ns = audio_clock_ns();
        }
        track->available = done * assets::kAdpcmFrameSamples;
        track->complete = done == frames;
        if (g_stopping) {
            break;  // leaving: what is decoded stays, the reader ends
        }
    }
    std::fclose(disc);
    {
        std::lock_guard lock(g_lock);
        if (!published) {
            log::warn("music: no VAG stream at sector {}", sector);
            track->failed = true;
            return;
        }
        // A file shorter than its header says, or the program leaving: what was decoded is the
        // track.
        track->complete = true;
    }
    if (short_read) {
        log::warn("music: the stream at sector {} ends early ({} of {} frames)", sector, done, frames);
    }
    if(track->trace) log::info("audio trace sector {}: first part ready {:.3f} ms after request (reader start {:.3f} ms); whole track {:.3f} ms after request (OPENRAC_STREAM_EARLY_START)",
        sector,double(first_ns-track->requested_ns)/1e6,double(started-track->requested_ns)/1e6,
        double(audio_clock_ns()-track->requested_ns)/1e6);
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
            // The samples that may be played: all of a whole track, the decoded ones of one still
            // being read (OPENRAC_STREAM_EARLY_START).
            const std::size_t n = t.available;
            if (!t.complete && static_cast<std::size_t>(s.position) >= n) {
                break;  // the reader has not got this far: silent until it has
            }
            if(t.trace && !t.consumed_ns.load(std::memory_order_relaxed))
                t.consumed_ns.store(audio_clock_ns(),std::memory_order_relaxed);
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
            if (!t.complete) {
                break;  // held at the end of what is decoded; the rest comes on a later call
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
    drain_traced();  // game thread, every sound command: cheap unless OPENRAC_AUDIO_TRACE=1
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
    drain_traced();
    static std::atomic<unsigned> traced{0};
    track->trace=audio_trace_enabled() && traced.fetch_add(1)<32;
    track->sector=sector;
    if(track->trace) { track->requested_ns=audio_clock_ns(); track->requested_tick=g_trace_tick; }
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
    if(track->trace) log::info("audio trace sector {}: requested at {:.3f} ms, tick {}; header done {:.3f} ms after request; group {} queue {} flags {:#x}",
        sector,double(track->requested_ns)/1e6,track->requested_tick,double(audio_clock_ns()-track->requested_ns)/1e6,group,queue_on,flags);
    std::uint32_t handle;
    // OPENRAC_AUDIO_TRACE: what plays before a part queued behind a live stream (its first sound
    // waits for them: queued playback, not a late read).
    std::string queued_behind;
    {
        std::lock_guard lock(g_lock);
        handle = OPENRAC_STREAM_TAG | (g_next_stream++ & 0x00FFFFFFu);
        Stream* stream = find_stream(queue_on);
        if (track->trace && stream != nullptr && !stream->parts.empty()) {
            double ahead_ms = 0.0;
            bool looping = false;
            for (std::size_t i = 0; i < stream->parts.size(); ++i) {
                const Track& t = *stream->parts[i].track;
                const std::size_t length = t.ready ? t.samples.size() : t.length;
                const double left = static_cast<double>(length) - (i == 0 ? stream->position : 0.0);
                ahead_ms += t.rate != 0 ? std::max(0.0, left) * 1000.0 / t.rate : 0.0;
                looping = looping || stream->parts[i].loop;
            }
            queued_behind = std::format(
                "queued behind {} part(s) of sector {} with about {:.0f} ms left{}{}",
                stream->parts.size(), stream->parts.front().track->sector, ahead_ms,
                looping ? " (the last loops until this part replaces it at the end of a pass)" : "",
                stream->paused ? " (the stream is paused)" : "");
        }
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
        if(track->trace) g_traced.push_back(track);
    }
    if (!queued_behind.empty()) {
        log::info("audio trace sector {}: {}", sector, queued_behind);
    }
    // The whole file first, or (OPENRAC_STREAM_EARLY_START) part by part as it plays.
    const auto reader = stream_early_start() ? read_track_early : read_track;
    if (g_read_at_once) {
        reader(sector, track);
    } else {
        std::lock_guard lock(g_lock);
        if (g_stopping) {
            // The program is leaving: the track is not read, and no thread is left behind.
            track->failed = true;
            return handle;
        }
        // The readers that have finished are joined (their work is done: the join only waits for
        // the thread to end); the others are kept.
        std::erase_if(g_readers, [](Reader& reader) {
            if (!reader.done->load()) {
                return false;
            }
            reader.thread.join();
            return true;
        });
        auto done = std::make_shared<std::atomic<bool>>(false);
        g_readers.push_back({std::thread([reader, sector, track, done] {
                                 reader(sector, track);
                                 done->store(true);
                             }),
                             done});
    }
    return handle;
}

// Stops the threads reading a track and waits for them: the program calls this before it
// exits, so that nothing they use is taken away while they are still working (a reader that
// locks g_lock after the static destructors have run is a crash at quit).
void openrac_sound_stop_readers() {
    std::vector<Reader> readers;
    {
        std::lock_guard lock(g_lock);
        g_stopping = true;
        readers.swap(g_readers);
    }
    // Outside the lock: a reader that is waiting for it must be able to take it and end.
    for (auto& reader : readers) {
        if (reader.thread.joinable()) {
            reader.thread.join();
        }
    }
}

void openrac_stream_read_at_once(int at_once) {
    g_read_at_once = at_once != 0;
}

int openrac_stream_live(uint32_t stream) {
    drain_traced();
    std::lock_guard lock(g_lock);
    return find_stream(stream) != nullptr;
}

int openrac_stream_buffered(uint32_t stream) {
    std::shared_ptr<Track> track;
    bool ready=false;
    {
        std::lock_guard lock(g_lock);
        const Stream* s=find_stream(stream);
        if(s && !s->parts.empty()) {track=s->parts.front().track;ready=track->ready;}
    }
    report_consumed(track);
    return ready;
}

int32_t openrac_stream_remaining_ms(uint32_t stream) {
    std::shared_ptr<Track> traced;
    const auto remaining=[&]() -> int32_t {
    std::lock_guard lock(g_lock);
    const Stream* s = find_stream(stream);
    if (s == nullptr || s->parts.empty()) {
        return 0;
    }
    const Track& t = *s->parts.front().track;
    if(t.trace) traced=s->parts.front().track;
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
    }();
    report_consumed(traced);
    return remaining;
}

void openrac_stream_stop(uint32_t stream) {
    {
        std::lock_guard lock(g_lock);
        if (stream == 0) {
            g_streams.clear();
        } else {
            std::erase_if(g_streams, [&](const std::unique_ptr<Stream>& s) {
                return std::find(s->handles.begin(), s->handles.end(), stream) != s->handles.end();
            });
        }
    }
    drain_traced();  // a stopped track that was already consumed is still reported
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

void openrac_sound_trace_tick(uint64_t tick, int ticks_a_second) {
    g_trace_tick = tick;
    if (!audio_trace_enabled() || ticks_a_second <= 0) {
        return;
    }
    // Where each traced stream that sounds is: its audio played against the game's time since
    // its first sound. Audio ahead and growing: the game's ticks run slower than real time while
    // the audio plays in real time. A fixed lead from the start: the stream started late or early
    // (requested tick against first-sound tick). The position is the mixer's, which renders a
    // little ahead of the speaker (the output's buffer), not what is heard.
    std::vector<std::string> lines;
    {
        std::lock_guard lock(g_lock);
        for (const auto& s : g_streams) {
            if (s->parts.empty()) {
                continue;
            }
            Track& t = *s->parts.front().track;
            const std::int64_t sounded = t.consumed_ns.load(std::memory_order_relaxed);
            if (!t.trace || sounded == 0 || t.rate == 0) {
                continue;
            }
            if (t.sounding_tick == 0) {
                t.sounding_tick = tick;
                t.report_tick = tick;
                lines.push_back(std::format(
                    "audio trace sector {}: first sound seen at tick {} ({} ticks = {:.3f} s of "
                    "game time after its request at tick {}; {:.3f} ms of wall time)",
                    t.sector, tick, tick - t.requested_tick,
                    double(tick - t.requested_tick) / ticks_a_second, t.requested_tick,
                    double(sounded - t.requested_ns) / 1e6));
            }
            if (tick < t.report_tick || t.reports >= 120) {
                continue;
            }
            t.report_tick = tick + static_cast<std::uint64_t>(ticks_a_second);
            ++t.reports;
            const double audio = s->position / t.rate;
            const double game = double(tick - t.sounding_tick) / ticks_a_second;
            const double wall = double(audio_clock_ns() - sounded) / 1e9;
            lines.push_back(std::format(
                "audio trace sector {}: {:.2f} s of wall time after its first sound: audio played "
                "{:.2f} s, the game advanced {} ticks = {:.2f} s; audio ahead of the game by {:+.2f} s{}",
                t.sector, wall, audio, tick - t.sounding_tick, game, audio - game,
                s->paused ? " (paused)" : !t.complete ? " (still being read)" : ""));
        }
    }
    for (const auto& line : lines) {
        log::info("{}", line);
    }
}

}  // extern "C"
