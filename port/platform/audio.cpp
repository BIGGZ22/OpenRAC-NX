// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/audio.h"

#include <algorithm>
#include <format>
#include <utility>

#include <SDL3/SDL.h>

namespace openrac::platform {
namespace {

constexpr int kFrameBytes = kAudioChannels * static_cast<int>(sizeof(std::int16_t));

}  // namespace

std::unique_ptr<AudioOutput> AudioOutput::open(std::string& error, Callback callback) {
    std::unique_ptr<AudioOutput> out(new AudioOutput());
    out->m_callback = std::move(callback);
    if (!out->start(error)) {
        return nullptr;
    }
    return out;
}

bool AudioOutput::start(std::string& error) {
    const SDL_AudioSpec spec{SDL_AUDIO_S16, kAudioChannels, kAudioRate};
    // SDL converts to whatever the device really runs at; the game always
    // sees 48 kHz stereo.
    m_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        m_callback ? &AudioOutput::feed : nullptr,
        m_callback ? this : nullptr
    );
    m_started_ms = SDL_GetTicks();
    if (m_stream == nullptr) {
        error = std::format("could not open the audio device: {}", SDL_GetError());
        return false;
    }
    SDL_SetAudioStreamGain(m_stream, m_gain);
    // Opened paused; start it unless the game paused it.
    if (!m_paused) {
        SDL_ResumeAudioStreamDevice(m_stream);
    }
    return true;
}

bool AudioOutput::watch(std::string& error) {
    const std::uint64_t now = SDL_GetTicks();
    const bool steady = m_watched_ms != 0 && now - m_watched_ms < kStallMs;
    m_watched_ms = now;
    if (!steady) {
        // The game stood still (in the background, or loading): the device may well have too.
        // Count from now.
        m_started_ms = now;
        return true;
    }
    if (!m_callback || m_paused) {
        return true;
    }
    const std::uint64_t last = std::max(m_fed_ms.load(std::memory_order_relaxed), m_started_ms);
    if (now - last < kStallMs) {
        return true;
    }
    if (m_stream != nullptr) {
        SDL_DestroyAudioStream(m_stream);
        m_stream = nullptr;
    }
    return start(error);
}

AudioOutput::~AudioOutput() {
    if (m_stream != nullptr) {
        SDL_DestroyAudioStream(m_stream);
    }
}

void AudioOutput::feed(void* self, SDL_AudioStream* stream, int additional, int /*total*/) {
    auto* out = static_cast<AudioOutput*>(self);
    out->m_fed_ms.store(SDL_GetTicks(), std::memory_order_relaxed);
    if (additional <= 0) {
        return;
    }
    const int frames = (additional + kFrameBytes - 1) / kFrameBytes;
    out->m_scratch.assign(static_cast<std::size_t>(frames * kAudioChannels), 0);
    out->m_callback(out->m_scratch);
    SDL_PutAudioStreamData(stream, out->m_scratch.data(), frames * kFrameBytes);
}

bool AudioOutput::queue(std::span<const std::int16_t> samples) {
    if (samples.empty() || m_stream == nullptr) {
        return true;
    }
    return SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size_bytes()));
}

int AudioOutput::queued_frames() const {
    if (m_stream == nullptr) {
        return 0;
    }
    const int bytes = SDL_GetAudioStreamQueued(m_stream);
    return bytes > 0 ? bytes / kFrameBytes : 0;
}

void AudioOutput::pause(bool paused) {
    m_paused = paused;
    if (m_stream == nullptr) {
        return;
    }
    if (paused) {
        SDL_PauseAudioStreamDevice(m_stream);
    } else {
        SDL_ResumeAudioStreamDevice(m_stream);
    }
}

void AudioOutput::set_gain(float gain) {
    m_gain = gain;
    if (m_stream == nullptr) {
        return;
    }
    SDL_SetAudioStreamGain(m_stream, gain);
}

}  // namespace openrac::platform
