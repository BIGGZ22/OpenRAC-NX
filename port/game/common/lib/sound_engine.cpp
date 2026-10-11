// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac/sound_engine.h: one SoundPlayer for the game, behind a lock, since the audio output
// renders on its own thread.

#include "openrac/sound_engine.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <span>
#include <vector>

#include "audio/sound_player.h"
#include "common/log.h"
#include "openrac/game_host.h"

namespace {

using namespace openrac;

std::mutex g_lock;

// Never destroyed: the output's thread may still render while the program exits.
audio::SoundPlayer& player() {
    static auto* p = new audio::SoundPlayer;
    return *p;
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
    const long at = static_cast<long>(sector) * 2048 + static_cast<long>(offset);
    std::uint8_t header[72];
    std::vector<std::uint8_t> file;
    if (std::fseek(disc, at, SEEK_SET) == 0
        && std::fread(header, 1, sizeof header, disc) == sizeof header) {
        const std::size_t size = bank_size(header, sizeof header);
        file.resize(size);
        if (size == 0 || std::fseek(disc, at, SEEK_SET) != 0
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

void openrac_sound_render(int16_t* interleaved, size_t frames) {
    std::lock_guard lock(g_lock);
    player().render(std::span(interleaved, frames * 2));
}

}  // extern "C"
