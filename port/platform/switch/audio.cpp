// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/audio.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstring>

namespace openrac::platform {

std::unique_ptr<AudioOutput> AudioOutput::open(std::string& error, Callback callback) {
    auto output = std::unique_ptr<AudioOutput>(new AudioOutput);
    output->m_callback = std::move(callback);
    SDL_AudioSpec wanted{};
    wanted.freq = kAudioRate;
    wanted.channels = kAudioChannels;
    wanted.format = AUDIO_S16SYS;
    wanted.samples = 1024;
    wanted.userdata = output.get();
    if (output->m_callback) {
        wanted.callback = +[](void* userdata, Uint8* bytes, int count) {
            auto* self = static_cast<AudioOutput*>(userdata);
            auto samples = std::span(reinterpret_cast<std::int16_t*>(bytes), static_cast<std::size_t>(count) / 2);
            std::fill(samples.begin(), samples.end(), 0);
            self->m_callback(samples);
            for (auto& sample : samples) sample = static_cast<std::int16_t>(sample * self->m_gain);
        };
    }
    output->m_switch_device = SDL_OpenAudioDevice(nullptr, 0, &wanted, nullptr, 0);
    if (!output->m_switch_device) { error = SDL_GetError(); return nullptr; }
    SDL_PauseAudioDevice(output->m_switch_device, 0);
    return output;
}

AudioOutput::~AudioOutput() {
    if (m_switch_device) SDL_CloseAudioDevice(m_switch_device);
}

bool AudioOutput::queue(std::span<const std::int16_t> samples) {
    if (m_callback) return false;
    m_scratch.assign(samples.begin(), samples.end());
    for (auto& sample : m_scratch) sample = static_cast<std::int16_t>(sample * m_gain);
    return SDL_QueueAudio(m_switch_device, m_scratch.data(), static_cast<Uint32>(m_scratch.size() * 2)) == 0;
}
int AudioOutput::queued_frames() const { return static_cast<int>(SDL_GetQueuedAudioSize(m_switch_device) / 4); }
void AudioOutput::pause(bool paused) { SDL_PauseAudioDevice(m_switch_device, paused ? 1 : 0); }
void AudioOutput::set_gain(float gain) {
    SDL_LockAudioDevice(m_switch_device);
    m_gain = std::clamp(gain, 0.0f, 1.0f);
    SDL_UnlockAudioDevice(m_switch_device);
}
bool AudioOutput::watch(std::string&) { return m_switch_device != 0; }

}  // namespace openrac::platform
