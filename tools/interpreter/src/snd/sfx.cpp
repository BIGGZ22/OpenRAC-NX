// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * A sound effect while it plays (see sfx.h).
 */

#include "snd/sfx.h"

#include <algorithm>
#include <cstdlib>

#include "snd/player.h"

namespace snd {
namespace {

/** The top of the library's volume range, and of a register's. */
constexpr s32 kFullVolume = 127;

/** A full turn of pan, in degrees. */
constexpr s32 kTurn = 360;

/** The range of a pitch bend. */
constexpr s32 kLowestBend = -0x8000;
constexpr s32 kHighestBend = 0x7FFF;

/** What a modulator moves, by the number the bank stores. */
constexpr u8 kTargetVolume = 1;
constexpr u8 kTargetPan = 2;
constexpr u8 kTargetPitchModifier = 3;
constexpr u8 kTargetPitchBend = 4;

/** A modulator's wave, by the number the bank stores. */
constexpr u8 kShapeSine = 1;
constexpr u8 kShapeSquare = 2;
constexpr u8 kShapeTriangle = 3;
constexpr u8 kShapeSaw = 4;
constexpr u8 kShapeRandom = 5;

/** Steps in one turn of a modulator's wave, and the fixed-point position past its end. */
constexpr s32 kWaveSteps = 2048;
constexpr s32 kWaveEnd = kWaveSteps << 16;

/** The most steps a sound may run in one tick before it is taken to be stuck in a loop. */
constexpr unsigned kMostStepsATick = 1000;

/**
 * Brings an angle into one turn.
 *
 * @param degrees Any angle.
 * @return The same direction, 0 to 359.
 */
s32 wrap_degrees(s32 degrees) {
    s32 wrapped = degrees % kTurn;

    // The remainder of a negative angle is negative.
    if (wrapped < 0) {
        wrapped += kTurn;
    }

    return wrapped;
}

/**
 * Keeps a value in the range of a register.
 *
 * @param value Any value.
 * @return The nearest value a signed byte holds.
 */
s8 to_register(s32 value) {
    return static_cast<s8>(std::clamp(value, -128, 127));
}

}  // namespace

PlayingSound::PlayingSound(Player& player, const SoundStart& start)
    : player_(player),
      bank_(start.bank),
      start_tick_(start.start_tick) {
    sfx_ = &bank_->sounds[start.index];
    started_as_ = sfx_;
    group_ = static_cast<u32>(sfx_->group);
    registers_ = start.registers;

    // A parent sound may replace the bank's volume and pan for its child.
    sound_volume_ = start.sound_volume == -1 ? sfx_->volume : start.sound_volume;
    s32 own_pan = start.sound_pan == -1 ? sfx_->pan : start.sound_pan;
    bool keeps_pan = start.pan == SoundStart::kOwnPan || start.pan == SoundStart::kKeepPan;

    caller_volume_ = start.volume == SoundStart::kKeepVolume ? 1024 : start.volume;
    caller_pan_ = keeps_pan ? own_pan : start.pan;
    caller_pitch_modifier_ = start.pitch_modifier;
    caller_pitch_bend_ = start.pitch_bend;

    volume_ = std::min((sound_volume_ * caller_volume_) >> 10, kFullVolume);
    pan_ = caller_pan_;
    pitch_modifier_ = caller_pitch_modifier_;
    pitch_bend_ = caller_pitch_bend_;

    // A sound with no steps is over before it starts.
    if (sfx_->grains.empty()) {
        done_ = true;
        return;
    }

    countdown_ = sfx_->grains[0].delay;

    // Steps with no delay run at once, as far as they go.
    for (unsigned n = 0; countdown_ <= 0 && !done_ && n < kMostStepsATick; n++) {
        run_next_grain();
    }
}

PlayingSound::~PlayingSound() {
    // A sound that is gone leaves no voice sounding.
    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->voice.stop();
    }
}

bool PlayingSound::tick() {
    drop_ended_voices();
    tick_modulators();

    // Children run beside their parent and are forgotten when they are over.
    for (std::size_t n = 0; n < children_.size();) {
        // (Erasing moves the next child into this place.)
        if (children_[n]->tick()) {
            children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(n));
        } else {
            n++;
        }
    }

    // The script has ended: the sound is over once nothing of it is left sounding.
    if (done_ && children_.empty()) {
        return voices_.empty();
    }

    // A paused sound does not count down.
    if (paused_) {
        return false;
    }

    countdown_--;

    // Run every step that is due. A script that never waits is stopped.
    for (unsigned n = 0; countdown_ <= 0 && !done_; n++) {
        // Stuck in a loop with no delay in it.
        if (n == kMostStepsATick) {
            done_ = true;
            break;
        }

        run_next_grain();
    }

    return false;
}

void PlayingSound::stop() {
    done_ = true;

    for (auto& child : children_) {
        child->stop();
    }

    // Released, not cut: each voice dies away along its own curve.
    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->voice.key_off();
    }
}

void PlayingSound::set_paused(bool paused) {
    paused_ = paused;

    for (auto& child : children_) {
        child->set_paused(paused);
    }

    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->paused = paused;

        // A paused voice is silent and does not move through its sample.
        if (paused) {
            slot->voice.set_volume(0, 0);
            slot->voice.set_pitch(0);
        } else {
            player_.apply_levels(*slot);
            slot->voice.set_pitch(slot->pitch);
        }
    }
}

void PlayingSound::set_volume_pan(s32 volume, s32 pan) {
    // A negative volume is on the bank's scale of 0 to 127, negated.
    if (volume < 0) {
        caller_volume_ = -1024 * volume / kFullVolume;
    } else if (volume != SoundStart::kKeepVolume) {
        caller_volume_ = volume;
    }

    // The pan: back to the sound's own, left as it is, or a new one.
    if (pan == SoundStart::kOwnPan) {
        caller_pan_ = sfx_->pan;
    } else if (pan != SoundStart::kKeepPan) {
        caller_pan_ = pan;
    }

    s32 new_volume = ((caller_volume_ * sound_volume_) >> 10) + lfo_volume_;
    s32 new_pan = wrap_degrees(caller_pan_ + lfo_pan_);

    new_volume = std::clamp(new_volume, 0, kFullVolume);

    // Nothing changed for the voices.
    if (new_volume == volume_ && new_pan == pan_) {
        return;
    }

    volume_ = new_volume;
    pan_ = new_pan;

    // A child's caller is its parent: it gets the parent's volume on the caller's scale.
    for (auto& child : children_) {
        child->set_volume_pan(caller_volume_ * sound_volume_ / kFullVolume, pan);
    }

    update_levels();
}

void PlayingSound::set_pitch_modifier(s32 modifier) {
    for (auto& child : children_) {
        child->set_pitch_modifier(modifier);
    }

    caller_pitch_modifier_ = modifier;
    update_pitch();
}

void PlayingSound::set_pitch_bend(s32 bend) {
    for (auto& child : children_) {
        child->set_pitch_bend(bend);
    }

    caller_pitch_bend_ = bend;
    update_pitch();
}

void PlayingSound::run_next_grain() {
    Grain& grain = sfx_->grains[static_cast<std::size_t>(next_grain_)];
    s32 extra = run_grain(grain);

    // A pick step plays one group of steps; after it, the other groups are jumped over.
    if (skipping_) {
        grains_to_play_--;

        // That was the group's last step.
        if (grains_to_play_ == 0) {
            next_grain_ += grains_to_skip_;
            skipping_ = false;
        }
    }

    next_grain_++;

    // Past the last step (or before the first, from a bad jump): the script has ended.
    if (next_grain_ < 0 || static_cast<std::size_t>(next_grain_) >= sfx_->grains.size()) {
        done_ = true;
        return;
    }

    countdown_ = sfx_->grains[static_cast<std::size_t>(next_grain_)].delay + extra;
}

s32 PlayingSound::run_grain(Grain& grain) {
    switch (grain.type) {
        // Starts a voice.
        case GrainType::Tone:
        case GrainType::Tone2:
            return run_tone(grain);

        // Sets up one of the four modulators.
        case GrainType::LfoSettings:
            return run_lfo_settings(grain);

        // Starts another sound of the bank beside this one.
        case GrainType::StartChildSound:
            return run_start_child(grain);

        // Ends the children that play a given sound.
        case GrainType::StopChildSound:
            return run_stop_child(grain);

        // Carries on as another sound of the bank.
        case GrainType::Branch:
            return run_branch(grain);

        // The marks of a loop and the jumps between them.
        case GrainType::LoopStart:
        case GrainType::LoopEnd:
        case GrainType::LoopContinue:
            return run_loop(grain);

        // Ends the script; voices already started play on.
        case GrainType::Stop:
            done_ = true;
            return 0;

        // Plays one of the groups of steps that follow.
        case GrainType::RandPlay:
        case GrainType::PlayCycle:
            return run_pick(grain);

        case GrainType::RandDelay:
            // Wait a random number of ticks below the step's amount.
            return grain.amount > 0 ? player_.random() % grain.amount : 0;

        // Bends the pitch of every voice.
        case GrainType::RandPitchBend:
        case GrainType::PitchBend:
        case GrainType::AddPitchBend:
            return run_pitch_bend(grain);

        // The counters a script keeps and tests.
        case GrainType::SetRegister:
        case GrainType::SetRegisterRand:
        case GrainType::IncRegister:
        case GrainType::DecRegister:
        case GrainType::TestRegister:
        case GrainType::AddRegister:
        case GrainType::CopyRegister:
            return run_register(grain);

        // Named places in the script and the jumps to them.
        case GrainType::Marker:
        case GrainType::GotoMarker:
        case GrainType::GotoRandomMarker:
        case GrainType::OnStopMarker:
            return run_marker(grain);

        // Waits for the voices to end, or ends them.
        case GrainType::WaitForAllVoices:
        case GrainType::KeyOffVoices:
        case GrainType::KillVoices:
            return run_voices(grain);

        case GrainType::Null:
        case GrainType::XrefId:
        case GrainType::XrefNum:
        case GrainType::PluginMessage:
        case GrainType::ControlNull:
            // Steps that do nothing here: placeholders, and messages for code a game adds.
            return 0;
    }

    // A number that is no kind of step: skipped.
    return 0;
}

s32 PlayingSound::run_tone(const Grain& grain) {
    const Tone& tone = grain.tone;

    // Bit 3 asks for the processor's noise generator, which is not made here.
    if (tone.flags & 8) {
        return 0;
    }

    // A tone starts with the volume and pan in force now.
    volume_ = std::clamp(((caller_volume_ * sound_volume_) >> 10) + lfo_volume_, 0, kFullVolume);
    pan_ = wrap_degrees(caller_pan_ + lfo_pan_);

    s32 tone_volume = std::max(resolve(tone.volume, kFullVolume), 0);
    s32 tone_pan = resolve(tone.pan, kTurn);

    // A pan taken from a register is the register's 0 to 127 spread over a full turn.
    if (tone.pan < 0 && tone.pan != -5) {
        tone_pan = kTurn * tone_pan / kFullVolume;
    }

    tone_pan = wrap_degrees(tone_pan);

    VoiceStart start;

    start.tone = &tone;
    start.bank = bank_;
    start.levels = pan_levels(volume_, pan_, tone_volume, tone_pan, player_.mono());
    start.tone_volume = tone_volume;
    start.tone_pan = tone_pan;
    start.group = group_;
    start.note = bent_note(tone, pitch_bend_, pitch_modifier_, note_);

    voices_.push_back(player_.start_voice(start));

    return 0;
}

s32 PlayingSound::run_lfo_settings(const Grain& grain) {
    Modulator& modulator = modulators_[grain.lfo.which & 3];

    modulator = Modulator{};
    modulator.settings = grain.lfo;

    // Target 0 switches the modulator off.
    if (grain.lfo.target == 0) {
        modulator.settings.shape = 0;
        return 0;
    }

    // Bit 1 of the flags starts the wave at a random point instead of the step's own.
    bool random_start = (grain.lfo.flags & 2) != 0;
    s32 first_step = random_start ? player_.random() & (kWaveSteps - 1) : grain.lfo.start_offset;

    modulator.next_step = first_step << 16;

    // A square flips at its duty cycle.
    if (grain.lfo.shape == kShapeSquare) {
        modulator.hold = grain.lfo.duty_cycle;
    }

    // A random wave starts with a height drawn now: zero, or somewhere below it.
    if (grain.lfo.shape == kShapeRandom) {
        modulator.hold = -(player_.random() & 0x7FFF) * (player_.random() & 1);
        modulator.high_half = true;
    }

    // How far it reaches is a part (in 1,024ths) of its target's whole range.
    s32 depth = grain.lfo.depth;

    switch (grain.lfo.target) {
        // Volume moves within the sound's own.
        case kTargetVolume:
            modulator.range = (sfx_->volume * depth) >> 10;
            break;

        // Pan moves up to half a turn, 180 degrees, to either side.
        case kTargetPan:
            modulator.range = (180 * depth) >> 10;
            break;

        case kTargetPitchModifier:
            // 6,096 fine steps: a little under four octaves.
            modulator.range = (6096 * depth) >> 10;
            break;

        // A bend moves up to its whole range.
        case kTargetPitchBend:
            modulator.range = (kHighestBend * depth) >> 10;
            break;

        default:
            // Two more targets exist in the bank format; the library does nothing for them.
            break;
    }

    return 0;
}

s32 PlayingSound::run_start_child(const Grain& grain) {
    s32 index = grain.child.sound;

    // A child named by its name instead of its index, or an index outside the bank.
    if (index < 0 || static_cast<std::size_t>(index) >= bank_->sounds.size()) {
        return 0;
    }

    s32 child_volume = std::clamp(std::abs(resolve(grain.child.volume, kFullVolume)), 0, 127);
    s32 child_pan = resolve(grain.child.pan, kTurn);

    // A pan taken from a register is the register's 0 to 127 spread over a full turn.
    if (grain.child.pan < 0 && grain.child.pan != -5) {
        child_pan = kTurn * std::min(std::abs(child_pan), kFullVolume) / kFullVolume;
    }

    SoundStart start;

    start.bank = bank_;
    start.index = static_cast<u32>(index);
    start.sound_volume = child_volume;
    start.sound_pan = child_pan;
    start.volume = caller_volume_ * sound_volume_ / kFullVolume;
    start.pan = caller_pan_;
    start.pitch_modifier = caller_pitch_modifier_;
    start.pitch_bend = caller_pitch_bend_;
    start.registers = registers_;
    start.start_tick = start_tick_;

    children_.push_back(std::make_unique<PlayingSound>(player_, start));

    return 0;
}

s32 PlayingSound::run_stop_child(const Grain& grain) {
    s32 index = grain.child.sound;

    // A child named by its name instead of its index, or an index outside the bank.
    if (index < 0 || static_cast<std::size_t>(index) >= bank_->sounds.size()) {
        return 0;
    }

    const Sfx* which = &bank_->sounds[static_cast<std::size_t>(index)];

    // Every child that plays that sound goes, voices and all.
    std::erase_if(children_, [which](const std::unique_ptr<PlayingSound>& child) {
        return child->playing_as() == which;
    });

    return 0;
}

s32 PlayingSound::run_branch(const Grain& grain) {
    s32 index = grain.child.sound;

    // A branch to a sound named by its name, or outside the bank: carry on with this one.
    if (index < 0 || static_cast<std::size_t>(index) >= bank_->sounds.size()) {
        return 0;
    }

    // The voices of the sound left behind are released and silenced.
    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->voice.key_off();
        slot->voice.set_volume(0, 0);
    }

    sfx_ = &bank_->sounds[static_cast<std::size_t>(index)];
    sound_volume_ = sfx_->volume;
    volume_ = (caller_volume_ * sfx_->volume) >> 10;
    group_ = static_cast<u32>(sfx_->group);

    // The caller steps to the next step after this one: the new sound's first.
    next_grain_ = -1;
    skipping_ = false;
    grains_to_play_ = 0;
    grains_to_skip_ = 0;

    return 0;
}

s32 PlayingSound::run_loop(const Grain& grain) {
    s32 count = static_cast<s32>(sfx_->grains.size());

    // The end of a loop goes back to the nearest start before it.
    if (grain.type == GrainType::LoopEnd) {
        for (s32 n = next_grain_ - 1; n >= 0; n--) {
            // The caller steps on from here, onto the start itself.
            if (sfx_->grains[static_cast<std::size_t>(n)].type == GrainType::LoopStart) {
                next_grain_ = n - 1;
                break;
            }
        }
    }

    // "Continue" goes forward to the nearest end, which then loops back.
    if (grain.type == GrainType::LoopContinue) {
        for (s32 n = next_grain_ + 1; n < count; n++) {
            // The caller steps on from the end, so the end itself is not run.
            if (sfx_->grains[static_cast<std::size_t>(n)].type == GrainType::LoopEnd) {
                next_grain_ = n;
                break;
            }
        }
    }

    return 0;
}

s32 PlayingSound::run_pick(Grain& grain) {
    // Both kinds are followed by `groups` groups of `size` steps each, and play one group.
    s32 groups = grain.parameters[0];
    s32 size = grain.parameters[1];
    s32 picked = 0;

    // Nothing to pick from.
    if (groups <= 0) {
        return 0;
    }

    if (grain.type == GrainType::RandPlay) {
        // A random group, but not the one picked last time; the step remembers that.
        picked = player_.random() % groups;

        // The same group as last time: take the one after it.
        if (picked == grain.parameters[2]) {
            picked = (picked + 1) % groups;
        }

        grain.parameters[2] = static_cast<s16>(picked);
    } else {
        // The groups in turn; the step remembers which is next.
        picked = grain.parameters[2];
        grain.parameters[2] = static_cast<s16>((picked + 1) % groups);
    }

    next_grain_ += picked * size;
    grains_to_play_ = size + 1;
    grains_to_skip_ = (groups - 1 - picked) * size;
    skipping_ = true;

    return 0;
}

s32 PlayingSound::run_pitch_bend(const Grain& grain) {
    s32 amount = grain.parameters[0];

    switch (grain.type) {
        case GrainType::RandPitchBend: {
            // A random bend over the whole range, scaled to `amount` hundredths of it.
            s32 anywhere = (0xFFFF * (player_.random() % 0x7FFF)) / 0x7FFF - 0x8000;

            set_pitch_bend(amount * anywhere / 100);
            break;
        }

        case GrainType::AddPitchBend:
            // The amount is on a scale of 127 for a full bend.
            set_pitch_bend(
                std::clamp(pitch_bend_ + kHighestBend * amount / 127, kLowestBend, kHighestBend)
            );
            break;

        case GrainType::PitchBend:
            // A plain bend: 127 is full up, -128 full down.
            set_pitch_bend(amount >= 0 ? kHighestBend * amount / 127 : kLowestBend * amount / -128);
            break;

        default:
            // Not reached: the caller sends only the three bend steps.
            break;
    }

    return 0;
}

s32 PlayingSound::run_register(const Grain& grain) {
    const s16* p = grain.parameters;

    switch (grain.type) {
        // The register, then the value.
        case GrainType::SetRegister:
            register_at(p[0]) = to_register(p[1]);
            break;

        case GrainType::SetRegisterRand: {
            // A random value from p[1] to p[2], both included.
            s32 range = p[2] - p[1] + 1;

            register_at(p[0]) = to_register(range > 0 ? player_.random() % range + p[1] : p[1]);
            break;
        }

        // One up; a register stops at the top of a signed byte.
        case GrainType::IncRegister:
            register_at(p[0]) = to_register(register_at(p[0]) + 1);
            break;

        // One down; it stops at the bottom of a signed byte.
        case GrainType::DecRegister:
            register_at(p[0]) = to_register(register_at(p[0]) - 1);
            break;

        case GrainType::AddRegister:
            // (This one names the amount first and the register second.)
            register_at(p[1]) = to_register(register_at(p[1]) + p[0]);
            break;

        // From the first register named to the second.
        case GrainType::CopyRegister:
            register_at(p[1]) = register_at(p[0]);
            break;

        case GrainType::TestRegister: {
            // A test: when it fails, the step after this one is skipped.
            s32 value = register_at(p[0]);
            bool skip = false;

            // The kind of test: 0 passes below the value, 1 at it, anything else above it.
            if (p[1] == 0) {
                skip = value >= p[2];
            } else if (p[1] == 1) {
                skip = value != p[2];
            } else {
                skip = p[2] >= value;
            }

            // Failed: the caller steps over one step more.
            if (skip) {
                next_grain_++;
            }

            break;
        }

        default:
            // Not reached: the caller sends only the seven register steps.
            break;
    }

    return 0;
}

s32 PlayingSound::run_marker(const Grain& grain) {
    s32 count = static_cast<s32>(sfx_->grains.size());
    s32 wanted = grain.parameters[0];

    // A marker is only a place to go to.
    if (grain.type == GrainType::Marker) {
        return 0;
    }

    // The steps a stop runs are the last ones: go to the end.
    if (grain.type == GrainType::OnStopMarker) {
        next_grain_ = count - 1;
        return 0;
    }

    // A random marker from p[0] to p[1], both included.
    if (grain.type == GrainType::GotoRandomMarker) {
        s32 range = grain.parameters[1] - grain.parameters[0] + 1;

        wanted = range > 0 ? player_.random() % range + grain.parameters[0] : wanted;
    }

    for (s32 n = 0; n < count; n++) {
        const Grain& other = sfx_->grains[static_cast<std::size_t>(n)];

        // The caller steps on from here, onto the step after the marker.
        if (other.type == GrainType::Marker && other.parameters[0] == wanted) {
            next_grain_ = n - 1;
            break;
        }
    }

    return 0;
}

s32 PlayingSound::run_voices(const Grain& grain) {
    drop_ended_voices();

    // Waiting: run this step again next tick for as long as a voice sounds.
    if (grain.type == GrainType::WaitForAllVoices) {
        // All have ended.
        if (voices_.empty()) {
            return 0;
        }

        next_grain_--;
        return 1;
    }

    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->voice.key_off();

        // "Kill" does not let the release be heard.
        if (grain.type == GrainType::KillVoices) {
            slot->voice.set_volume(0, 0);
        }
    }

    return 0;
}

s32 PlayingSound::resolve(s32 value, s32 random_range) {
    // The value itself.
    if (value >= 0) {
        return value;
    }

    // -1 to -4: the sound's own registers.
    if (value >= -4) {
        return registers_[static_cast<std::size_t>(-value - 1)];
    }

    // -5: anything in the range.
    if (value == -5) {
        return player_.random() % random_range;
    }

    return player_.shared_register(-value - 6);
}

s8& PlayingSound::register_at(s32 which) {
    // Negative numbers count through the registers all sounds share.
    if (which < 0) {
        return player_.shared_register(-which - 1);
    }

    // Not one of the sound's four.
    if (which > 3) {
        return spare_register_;
    }

    return registers_[static_cast<std::size_t>(which)];
}

void PlayingSound::update_levels() {
    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->levels = pan_levels(volume_, pan_, slot->tone_volume, slot->tone_pan, player_.mono());
        player_.apply_levels(*slot);
    }
}

void PlayingSound::update_pitch() {
    pitch_modifier_ = caller_pitch_modifier_ + lfo_pitch_modifier_;
    pitch_bend_ = std::clamp(caller_pitch_bend_ + lfo_pitch_bend_, kLowestBend, kHighestBend);

    for (const VoiceUse& use : voices_) {
        Player::Slot* slot = player_.slot(use);

        // Already ended.
        if (!slot) {
            continue;
        }

        slot->pitch =
            note_pitch(*slot->tone, bent_note(*slot->tone, pitch_bend_, pitch_modifier_, note_));

        // A paused voice gets its pitch back when it continues.
        if (!slot->paused) {
            slot->voice.set_pitch(slot->pitch);
        }
    }
}

void PlayingSound::drop_ended_voices() {
    std::erase_if(voices_, [this](const VoiceUse& use) { return player_.slot(use) == nullptr; });
}

void PlayingSound::tick_modulators() {
    for (Modulator& modulator : modulators_) {
        modulator.ticks++;

        // Off, or the tick between two on which it moves (it runs at half the tick rate).
        if (modulator.settings.shape == 0 || (modulator.ticks & 1) == 0) {
            continue;
        }

        s32 height = modulator_height(modulator);

        switch (modulator.settings.target) {
            case kTargetVolume:
                // Volume is only ever lowered: the wave's top is no change.
                lfo_volume_ = (modulator.range * (height - 0x7FFF)) >> 16;
                set_volume_pan(SoundStart::kKeepVolume, SoundStart::kKeepPan);
                break;

            // Pan swings to both sides of where the caller put it.
            case kTargetPan:
                lfo_pan_ = (modulator.range * height) >> 15;
                set_volume_pan(SoundStart::kKeepVolume, SoundStart::kKeepPan);
                break;

            // In 128ths of a semitone, up and down.
            case kTargetPitchModifier:
                lfo_pitch_modifier_ = (height * modulator.range) >> 15;
                update_pitch();
                break;

            // Added to the caller's bend; `update_pitch` keeps the sum in range.
            case kTargetPitchBend:
                lfo_pitch_bend_ = (height * modulator.range) >> 15;
                update_pitch();
                break;

            default:
                // A target the library does nothing for.
                break;
        }
    }
}

s32 PlayingSound::modulator_height(Modulator& modulator) {
    s32 step = modulator.next_step >> 16;
    s32 height = 0;

    // Two steps' worth, because it moves on every other tick.
    modulator.next_step += static_cast<s32>(2 * modulator.settings.step_size);

    // Past the end of the wave: round to its start.
    if (modulator.next_step >= kWaveEnd || modulator.next_step < 0) {
        modulator.next_step &= kWaveEnd - 1;
    }

    switch (modulator.settings.shape) {
        // The library's table starts at the top of the wave.
        case kShapeSine:
            height = sine_step(step);
            break;

        // At the top until the flip step, at the bottom after it.
        case kShapeSquare:
            height = step >= modulator.hold ? -32767 : 32767;
            break;

        case kShapeTriangle:
            // Up over the first quarter, down over the middle half, up over the last quarter.
            if (step < 512) {
                height = 0x7FFF * step / 512;
            } else if (step >= 1536) {
                height = 0x7FFF * (step - 1536) / 512 - 0x7FFF;
            } else {
                height = 0x7FFF - 65534 * (step - 512) / 1024;
            }

            break;

        case kShapeSaw:
            // Up over the first half, then from the bottom up again over the second.
            height = step >= 1024 ? 0x7FFF * (step - 1024) / 1024 - 0x7FFF : 0x7FFF * step / 1023;
            break;

        case kShapeRandom:
            // A new height at the start of each half of the wave.
            if (step >= 1024 && modulator.high_half) {
                modulator.high_half = false;
                modulator.hold = 2 * ((player_.random() & 0x7FFF) - 0x3FFF);
            } else if (step < 1024 && !modulator.high_half) {
                modulator.high_half = true;
                modulator.hold = -(player_.random() & 0x7FFF) * (player_.random() & 1);
            }

            height = modulator.hold;
            break;

        default:
            // Off, or a shape that does not exist: no movement.
            break;
    }

    // Bit 0 of the flags turns the wave upside down.
    return (modulator.settings.flags & 1) ? -height : height;
}

}  // namespace snd
