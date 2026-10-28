// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The sound output: one SDL3 audio stream, 48 kHz, stereo, signed 16-bit,
// which is what the console's sound processor mixes to. The replacement of
// the game's sound library (989snd, at its API) mixes into it in one of two
// ways:
//
//   - pull: a callback that SDL's audio thread calls whenever the device
//     needs more, with a buffer to fill (the way the console's mixer runs);
//   - push: queue() from the game's thread, with queued_frames() to keep the
//     queue about one frame deep.
//
// A callback, when given, runs on SDL's audio thread: it must not touch the
// game's state without a lock of its own.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

#ifdef __SWITCH__
typedef struct _SDL_AudioStream SDL_AudioStream;
#else
struct SDL_AudioStream;
#endif

namespace openrac::platform {

inline constexpr int kAudioRate = 48000;
inline constexpr int kAudioChannels = 2;

class AudioOutput {
public:
    // Fill `samples` (interleaved left, right; samples.size() / 2 frames).
    using Callback = std::function<void(std::span<std::int16_t> samples)>;

    // Opens the default playback device and starts it. Without a callback,
    // the output plays what queue() hands it, and silence when it runs dry.
    static std::unique_ptr<AudioOutput> open(std::string& error, Callback callback = {});

    ~AudioOutput();
    AudioOutput(const AudioOutput&) = delete;
    AudioOutput& operator=(const AudioOutput&) = delete;

    // Push model: append interleaved stereo samples.
    bool queue(std::span<const std::int16_t> samples);

    // Frames queued and not played yet.
    int queued_frames() const;

    void pause(bool paused);

    // Output gain, 0 (silent) to 1 (as mixed).
    void set_gain(float gain);

    // Call once a frame from the game's thread. When the callback has not run for kStallMs while
    // playing (the system took the device and never gave it back, as Android does after a call or
    // another app's sound), the stream is opened again on the default device. Returns false, with
    // `error`, when that failed; the next call tries again.
    bool watch(std::string& error);

    static constexpr std::uint64_t kStallMs = 1500;

private:
    AudioOutput() = default;

    bool start(std::string& error);

    static void feed(void* self, SDL_AudioStream* stream, int additional, int total);

    SDL_AudioStream* m_stream = nullptr;
#ifdef __SWITCH__
    std::uint32_t m_switch_device = 0;
#endif
    Callback m_callback;
    std::vector<std::int16_t> m_scratch;
    float m_gain = 1.0f;
    bool m_paused = false;
    // SDL_GetTicks() of the callback's last run (written on the audio thread), of the stream's
    // start, and of the last watch() (a longer gap means the game itself stood still).
    std::atomic<std::uint64_t> m_fed_ms{0};
    std::uint64_t m_started_ms = 0;
    std::uint64_t m_watched_ms = 0;
};

}  // namespace openrac::platform
