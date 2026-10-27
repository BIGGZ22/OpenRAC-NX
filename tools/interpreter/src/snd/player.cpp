// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The sound effect player: banks, the sounds that play, their voices and the mix (see player.h).
 */

#include "snd/player.h"

#include <algorithm>
#include <utility>

namespace snd {
namespace {

/** A group's volume when nothing has set it, and the top of the range. */
constexpr s32 kFullGroupVolume = 1024;

/** The group number that is the volume of everything. */
constexpr u32 kMasterGroup = 16;

/** A group number the library leaves alone (seen in the reference: writes to it are dropped). */
constexpr u32 kNoGroup = 15;

}  // namespace

Player::Player() {
    group_volume_.fill(kFullGroupVolume);
}

bool Player::load_bank(u32 handle, const u8* file, std::size_t bytes) {
    std::unique_ptr<Bank> bank = Bank::parse(file, bytes);

    // Not a sound effect bank, or damaged.
    if (!bank) {
        return false;
    }

    // A bank loaded again under the same number replaces the old one.
    unload_bank(handle);
    banks_[handle] = std::move(bank);

    return true;
}

void Player::unload_bank(u32 handle) {
    auto found = banks_.find(handle);

    // No such bank.
    if (found == banks_.end()) {
        return;
    }

    const Bank* bank = found->second.get();

    // Its sounds go first: they point into the bank.
    std::erase_if(sounds_, [bank](const auto& entry) { return entry.second->bank() == bank; });

    // A voice left sounding would read sample data that is about to go.
    for (Slot& slot : slots_) {
        // A voice of another bank.
        if (slot.bank != bank) {
            continue;
        }

        slot.voice.stop();
        slot.bank = nullptr;
        slot.tone = nullptr;
    }

    banks_.erase(found);
}

bool Player::play(u32 handle, u32 bank, SoundStart start) {
    auto found = banks_.find(bank);

    // No such bank.
    if (found == banks_.end()) {
        return false;
    }

    // No such sound in it.
    if (start.index >= found->second->sounds.size()) {
        return false;
    }

    const Sfx* sfx = &found->second->sounds[start.index];
    s32 volume = start.volume == SoundStart::kKeepVolume ? kFullGroupVolume : start.volume;

    // The sound's limit on how many of it may play at once keeps this one out.
    if (!make_room(sfx, volume)) {
        return false;
    }

    start.bank = found->second.get();
    start.start_tick = ticks_;

    // The old sound under this number, if any, goes before the new one starts its voices.
    sounds_.erase(handle);
    sounds_[handle] = std::make_unique<PlayingSound>(*this, start);

    return true;
}

void Player::stop(u32 handle) {
    PlayingSound* playing = sound(handle);

    // Already over.
    if (!playing) {
        return;
    }

    playing->stop();
}

void Player::stop_all() {
    // Each sound silences its voices as it goes.
    sounds_.clear();
}

PlayingSound* Player::sound(u32 handle) {
    auto found = sounds_.find(handle);

    return found == sounds_.end() ? nullptr : found->second.get();
}

void Player::set_groups_paused(u32 groups, bool paused) {
    for (auto& entry : sounds_) {
        u32 group = entry.second->group();

        // A group number with no bit in the mask, or a group that was not named.
        if (group >= kGroups || ((groups >> group) & 1) == 0) {
            continue;
        }

        entry.second->set_paused(paused);
    }
}

void Player::set_group_volume(u32 group, s32 volume) {
    // Not a group.
    if (group >= kGroups || group == kNoGroup) {
        return;
    }

    group_volume_[group] = std::clamp(volume, 0, kFullGroupVolume);

    for (Slot& slot : slots_) {
        // Not sounding, or silent until it is continued: it gets its levels then.
        if (slot.voice.is_free() || slot.paused) {
            continue;
        }

        // The volume of everything changes every voice; a group's only its own.
        if (group == kMasterGroup || slot.group == group) {
            apply_levels(slot);
        }
    }
}

void Player::mix(std::size_t frames, s32* sums) {
    std::size_t done = 0;

    while (done < frames) {
        // Time for the sounds to move on.
        if (frames_to_tick_ == 0) {
            tick();
            frames_to_tick_ = kTickFrames;
        }

        std::size_t run = std::min(frames - done, static_cast<std::size_t>(frames_to_tick_));

        for (Slot& slot : slots_) {
            // Nothing to hear from it.
            if (slot.voice.is_free() || slot.paused) {
                continue;
            }

            for (std::size_t n = 0; n < run; n++) {
                s32 left = 0;
                s32 right = 0;

                slot.voice.run(left, right);

                // The library's own master level is a half (seen in the reference).
                sums[2 * (done + n)] += left / 2;
                sums[2 * (done + n) + 1] += right / 2;
            }
        }

        done += run;
        frames_to_tick_ -= static_cast<int>(run);
    }
}

VoiceUse Player::start_voice(const VoiceStart& start) {
    std::size_t index = slots_.size();

    // A free voice first.
    for (std::size_t n = 0; n < slots_.size(); n++) {
        // This one has ended.
        if (slots_[n].voice.is_free()) {
            index = n;
            break;
        }
    }

    // Then a new one, while the processor has voices left.
    if (index == slots_.size() && slots_.size() < kVoices) {
        slots_.emplace_back();
    }

    // Then the voice that matters least; among equals the one started first (assumed).
    if (index == slots_.size()) {
        index = 0;

        for (std::size_t n = 1; n < slots_.size(); n++) {
            const Slot& slot = slots_[n];
            const Slot& best = slots_[index];
            bool lower = slot.priority < best.priority;
            bool older = slot.priority == best.priority && slot.serial < best.serial;

            // A better one to give up.
            if (lower || older) {
                index = n;
            }
        }

        slots_[index].voice.stop();
    }

    Slot* chosen = &slots_[index];

    voice_serial_++;

    VoiceUse use;

    use.slot = static_cast<unsigned>(index);
    use.serial = voice_serial_;

    chosen->serial = voice_serial_;
    chosen->tone = start.tone;
    chosen->bank = start.bank;
    chosen->priority = start.tone->priority;
    chosen->levels = start.levels;
    chosen->tone_volume = start.tone_volume;
    chosen->tone_pan = start.tone_pan;
    chosen->group = start.group;
    chosen->pitch = note_pitch(*start.tone, start.note);
    chosen->paused = false;

    const std::vector<u8>& samples = start.bank->samples;

    // A tone whose sample lies outside the bank's data stays silent: the voice is not started.
    if (start.tone->sample >= samples.size()) {
        return use;
    }

    chosen->voice.key_on(
        samples.data() + start.tone->sample,
        samples.size() - start.tone->sample,
        start.tone->adsr1,
        start.tone->adsr2
    );
    chosen->voice.set_pitch(chosen->pitch);
    apply_levels(*chosen);

    return use;
}

Player::Slot* Player::slot(VoiceUse use) {
    // Not a voice.
    if (use.slot >= slots_.size()) {
        return nullptr;
    }

    Slot& found = slots_[use.slot];

    // The voice has ended, or another sound has taken the place since.
    if (found.serial != use.serial || found.voice.is_free()) {
        return nullptr;
    }

    return &found;
}

void Player::apply_levels(Slot& target) {
    u32 group = target.group < kGroups ? target.group : 0;
    s32 volume = group_volume_[group] * group_volume_[kMasterGroup] / kFullGroupVolume;

    target.voice.set_volume(
        group_level(target.levels.left, volume), group_level(target.levels.right, volume)
    );
}

s32 Player::random() {
    // The constants of the C standard's example generator; its upper bits are the better ones.
    random_state_ = random_state_ * 1103515245u + 12345u;

    return static_cast<s32>((random_state_ >> 16) & 0x7FFF);
}

s8& Player::shared_register(s32 which) {
    // Not one of the 32.
    if (which < 0 || static_cast<std::size_t>(which) >= shared_registers_.size()) {
        return spare_register_;
    }

    return shared_registers_[static_cast<std::size_t>(which)];
}

void Player::tick() {
    ticks_++;

    // A sound is forgotten once its script has ended and its voices have died away.
    std::erase_if(sounds_, [](const auto& entry) { return entry.second->tick(); });
}

bool Player::make_room(const Sfx* sfx, s32 volume) {
    bool by_volume = (sfx->flags & Sfx::kLimitByVolume) != 0;
    bool by_age = (sfx->flags & Sfx::kLimitByAge) != 0;

    // No limit asked for. A limit of nothing is taken as no limit (assumed).
    if ((sfx->flags & Sfx::kLimitInstances) == 0 || sfx->instance_limit <= 0) {
        return true;
    }

    u32 weakest_handle = 0;
    PlayingSound* weakest = nullptr;
    s32 playing = 0;

    // Count the ones playing and find the one to give way: the quietest or the oldest.
    for (auto& entry : sounds_) {
        PlayingSound* other = entry.second.get();

        // Another sound.
        if (other->playing_as() != sfx) {
            continue;
        }

        playing++;

        bool quieter = weakest && by_volume && other->caller_volume() < weakest->caller_volume();
        bool older = weakest && by_age && other->start_tick() < weakest->start_tick();

        // The first one found, or one weaker than the weakest so far.
        if (!weakest || quieter || older) {
            weakest = other;
            weakest_handle = entry.first;
        }
    }

    // Room left.
    if (playing < sfx->instance_limit) {
        return true;
    }

    // Full: the new one gets in only by age, or by being louder than the quietest.
    bool wins = by_age || (by_volume && weakest->caller_volume() < volume);

    // The newcomer loses.
    if (!wins) {
        return false;
    }

    stop(weakest_handle);

    return true;
}

}  // namespace snd
