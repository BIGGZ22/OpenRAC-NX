// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "audio/sound_player.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "audio/pitch.h"

namespace openrac::audio {

using assets::GrainType;

namespace {

int wrap_degrees(int pan) {
    pan %= 360;
    return pan < 0 ? pan + 360 : pan;
}

// The constant-power curve: entry i is 0x3fff * (cos, sin) of i / 2 degrees,
// truncated. The library keeps it as a table; it is computed here.
std::array<std::int16_t, 2> pan_curve(int i) {
    const double a = i * 0.5 * std::numbers::pi / 180.0;
    return {
        static_cast<std::int16_t>(0x3fff * std::cos(a)),
        static_cast<std::int16_t>(0x3fff * std::sin(a)),
    };
}

// The LFO sine: 2048 steps of 32767 cos.
int lfo_sine(int step) {
    return static_cast<int>(32767.0 * std::cos(2.0 * std::numbers::pi * step / 2048.0));
}

std::uint16_t volume_register(std::int16_t v) {
    return static_cast<std::uint16_t>(v >> 1);
}

}  // namespace

std::array<std::int16_t, 2> make_volume(
    int vol1, int pan1, int vol2, int pan2, int vol3, int pan3
) {
    int vol = vol1 * 258;
    vol = vol * vol2 / 0x7f;
    vol = vol * vol3 / 0x7f;

    if (vol == 0) {
        return {0, 0};
    }

    int pan = wrap_degrees(pan1 + pan2 + pan3);
    // 0 degrees is straight ahead: the curve starts a quarter turn left.
    pan = pan >= 270 ? pan - 270 : pan + 90;
    const bool mirrored = pan >= 180;
    const auto c = pan_curve(mirrored ? pan - 180 : pan);
    const auto a = static_cast<std::int16_t>(c[0] * vol / 0x3fff);
    const auto b = static_cast<std::int16_t>(c[1] * vol / 0x3fff);
    return mirrored ? std::array<std::int16_t, 2>{b, a} : std::array<std::int16_t, 2>{a, b};
}

std::int16_t group_volume(std::int16_t volume, std::int32_t master) {
    std::int32_t v = std::min<std::int32_t>(volume, 0x7ffe);
    v = v * master / 0x400;
    const int sign = v < 0 ? -1 : 1;
    return static_cast<std::int16_t>(v * v / 0x7ffe * sign);
}

struct SoundPlayer::Bank {
    assets::SoundBank bank;
    SampleData samples;
    // RAND_PLAY's last pick and PLAY_CYCLE's position live in the grain on
    // the console, shared by every play of the sound: here, per grain.
    std::vector<std::int16_t> grain_state;
};

struct SoundPlayer::Slot {
    SoundHandle owner = 0;
    std::uint32_t generation = 0;
    std::uint64_t started = 0;
    int priority = 0;
    assets::Tone tone;
    std::array<std::int16_t, 2> base{};  // before the group
    int tone_volume = 0;
    int tone_pan = 0;
};

struct Lfo {
    std::uint8_t target = 0;  // 0 none, 1 volume, 2 pan, 3 pitch modulation, 4 pitch bend
    std::uint8_t shape = 0;   // 1 sine, 2 square, 3 triangle, 4 saw, 5 random
    std::uint16_t flags = 0;  // 1 inverted
    std::int32_t depth = 0;
    std::int32_t range = 0;
    std::uint32_t step_size = 0;
    std::int32_t next_step = 0;
    std::int32_t hold1 = 0;
    std::int32_t hold2 = 0;
    std::uint32_t ticks = 0;
};

struct SoundPlayer::Script {
    SoundHandle handle = 0;
    BankHandle bank_handle = 0;
    Bank* bank = nullptr;
    std::size_t sound = 0;
    std::span<const assets::Grain> grains;
    std::size_t first_grain = 0;  // in the bank's grain list
    int group = 0;
    std::uint64_t started = 0;
    bool done = false;
    bool paused = false;
    std::int32_t next = 0;
    std::int32_t countdown = 0;
    std::uint32_t to_play = 0;  // RAND_PLAY / PLAY_CYCLE: grains still to run, then skip
    std::uint32_t to_skip = 0;
    bool skipping = false;
    std::int32_t own_volume = 0;                                                  // the sound's
    std::int32_t volume = 0, pan = 0, pitch_mod = 0, pitch_bend = 0;              // in effect
    std::int32_t asked_volume = 0, asked_pan = 0, asked_mod = 0, asked_bend = 0;  // the caller's
    std::int32_t lfo_volume = 0, lfo_pan = 0, lfo_mod = 0, lfo_bend = 0;
    std::array<std::int8_t, 4> registers{};
    std::array<Lfo, 4> lfos{};
};

SoundPlayer::SoundPlayer() : m_slots(kVoiceCount) {
    m_master.fill(kFullVolume);
}

SoundPlayer::~SoundPlayer() = default;

BankHandle SoundPlayer::load_bank(std::span<const std::uint8_t> file) {
    auto b = std::make_unique<Bank>();
    b->bank = assets::parse_sound_bank(file);
    b->samples = std::make_shared<const std::vector<std::uint8_t>>(b->bank.samples);
    b->grain_state.assign(b->bank.grains.size(), 0);
    const BankHandle handle = m_next_bank++;
    m_banks.emplace(handle, std::move(b));
    return handle;
}

void SoundPlayer::unload_bank(BankHandle bank) {
    for (auto it = m_scripts.begin(); it != m_scripts.end();) {
        if (it->second->bank_handle == bank) {
            for (const std::size_t v : voices_of(*it->second)) {
                m_mixer.voice(v).stop();
                m_slots[v].owner = 0;
            }

            it = m_scripts.erase(it);
        } else {
            ++it;
        }
    }

    m_banks.erase(bank);
}

const assets::SoundBank* SoundPlayer::bank(BankHandle bank) const {
    const auto it = m_banks.find(bank);
    return it == m_banks.end() ? nullptr : &it->second->bank;
}

SoundHandle SoundPlayer::play(BankHandle bank_handle, std::uint32_t sound, const PlayRequest& r) {
    const auto it = m_banks.find(bank_handle);

    if (it == m_banks.end() || sound >= it->second->bank.sounds.size()) {
        return 0;
    }

    Bank& b = *it->second;
    const assets::SfxSound& sfx = b.bank.sounds[sound];
    const auto grains = b.bank.sound_grains(sound);

    if (grains.empty()) {
        return 0;
    }

    const std::int32_t volume = r.volume == kVolumeKeep ? 1024 : r.volume;

    // The instance limit (flag 8): when the sound already plays that many
    // times, the quietest (flag 0x10) or the oldest (flag 0x20) one gives way,
    // or the new one does not start.
    if (sfx.instance_limit > 0 && (sfx.flags & 8)) {
        Script* weakest = nullptr;
        int count = 0;

        for (auto& [h, s] : m_scripts) {
            if (s->bank_handle != bank_handle || s->sound != sound) {
                continue;
            }

            ++count;

            if (!weakest || ((sfx.flags & 0x10) && s->asked_volume < weakest->asked_volume)
                || ((sfx.flags & 0x20) && s->started < weakest->started)) {
                weakest = s.get();
            }
        }

        if (count >= sfx.instance_limit) {
            const bool replace =
                weakest
                && (((sfx.flags & 0x10) && weakest->asked_volume < volume) || (sfx.flags & 0x20));

            if (!replace) {
                return 0;
            }

            stop(weakest->handle);
        }
    }

    auto s = std::make_unique<Script>();
    s->handle = m_next_sound++;

    if (m_next_sound == 0) {
        m_next_sound = 1;
    }

    s->bank_handle = bank_handle;
    s->bank = &b;
    s->sound = sound;
    s->grains = grains;
    s->first_grain = static_cast<std::size_t>(grains.data() - b.bank.grains.data());
    s->group = sfx.volume_group;
    s->started = m_ticks;
    s->own_volume = sfx.volume;
    s->volume = std::min((sfx.volume * volume) >> 10, 127);
    s->pan = r.pan == kPanReset || r.pan == kPanKeep ? sfx.pan : r.pan;
    s->asked_volume = volume;
    s->asked_pan = s->pan;
    s->pitch_mod = s->asked_mod = r.pitch_mod;
    s->pitch_bend = s->asked_bend = r.pitch_bend;
    s->registers = r.registers;
    s->countdown = grains[0].delay;

    // Grains due at once run before play returns.
    while (s->countdown <= 0 && !s->done) {
        run_grain(*s);
    }

    const SoundHandle handle = s->handle;
    m_scripts.emplace(handle, std::move(s));
    return handle;
}

bool SoundPlayer::playing(SoundHandle sound) const {
    return m_scripts.contains(sound);
}

void SoundPlayer::stop(SoundHandle sound) {
    const auto it = m_scripts.find(sound);

    if (it == m_scripts.end()) {
        return;
    }

    it->second->done = true;

    for (const std::size_t v : voices_of(*it->second)) {
        m_mixer.voice(v).key_off();
    }
}

void SoundPlayer::stop_all() {
    for (auto& [h, s] : m_scripts) {
        for (const std::size_t v : voices_of(*s)) {
            m_mixer.voice(v).stop();
            m_slots[v].owner = 0;
        }
    }

    m_scripts.clear();
}

void SoundPlayer::set_volume_pan(SoundHandle sound, std::int32_t volume, std::int32_t pan) {
    if (const auto it = m_scripts.find(sound); it != m_scripts.end()) {
        apply_volume(*it->second, volume, pan);
    }
}

void SoundPlayer::set_pitch_mod(SoundHandle sound, std::int32_t pitch_mod) {
    if (const auto it = m_scripts.find(sound); it != m_scripts.end()) {
        it->second->asked_mod = pitch_mod;
        apply_pitch(*it->second);
    }
}

void SoundPlayer::set_pitch_bend(SoundHandle sound, std::int32_t pitch_bend) {
    if (const auto it = m_scripts.find(sound); it != m_scripts.end()) {
        it->second->asked_bend = pitch_bend;
        apply_pitch(*it->second);
    }
}

void SoundPlayer::set_register(SoundHandle sound, std::size_t which, std::int8_t value) {
    if (const auto it = m_scripts.find(sound); it != m_scripts.end() && which < 4) {
        it->second->registers[which] = value;
    }
}

void SoundPlayer::set_master_volume(std::size_t group, std::int32_t volume) {
    if (group >= kVolumeGroups) {
        return;
    }

    m_master[group] = volume;

    for (auto& [h, s] : m_scripts) {
        if (s->group != static_cast<int>(group) || s->paused) {
            continue;
        }

        for (const std::size_t v : voices_of(*s)) {
            const Slot& slot = m_slots[v];
            m_mixer.voice(v).set_volume({
                volume_register(group_volume(slot.base[0], volume)),
                volume_register(group_volume(slot.base[1], volume)),
            });
        }
    }
}

void SoundPlayer::pause_groups(std::uint32_t mask) {
    for (auto& [h, s] : m_scripts) {
        if ((mask >> (s->group & 31) & 1) == 0 || s->paused) {
            continue;
        }

        s->paused = true;

        for (const std::size_t v : voices_of(*s)) {
            m_mixer.voice(v).set_volume({0, 0});
            m_mixer.voice(v).set_pitch(0);
        }
    }
}

void SoundPlayer::continue_groups(std::uint32_t mask) {
    for (auto& [h, s] : m_scripts) {
        if ((mask >> (s->group & 31) & 1) == 0 || !s->paused) {
            continue;
        }

        s->paused = false;
        const std::int32_t master =
            s->group < static_cast<int>(kVolumeGroups) ? m_master[s->group] : kFullVolume;

        for (const std::size_t v : voices_of(*s)) {
            const Slot& slot = m_slots[v];
            m_mixer.voice(v).set_volume({
                volume_register(group_volume(slot.base[0], master)),
                volume_register(group_volume(slot.base[1], master)),
            });
        }

        apply_pitch(*s);
    }
}

void SoundPlayer::render(std::span<std::int16_t> interleaved) {
    for (std::size_t i = 0; i + 1 < interleaved.size(); i += 2) {
        if (m_frame_in_tick == 0) {
            tick();
        }

        m_frame_in_tick = (m_frame_in_tick + 1) % kFramesPerTick;
        const auto out = m_mixer.mix();
        interleaved[i] = out[0];
        interleaved[i + 1] = out[1];
    }
}

void SoundPlayer::tick() {
    ++m_ticks;

    for (auto it = m_scripts.begin(); it != m_scripts.end();) {
        Script& s = *it->second;

        if (!s.paused) {
            tick_lfos(s);
        }

        if (s.done) {
            if (voices_of(s).empty()) {
                it = m_scripts.erase(it);
                continue;
            }
        } else if (!s.paused) {
            --s.countdown;

            while (s.countdown <= 0 && !s.done) {
                run_grain(s);
            }
        }

        ++it;
    }
}

std::size_t SoundPlayer::sounds_playing() const {
    return m_scripts.size();
}

bool SoundPlayer::owns(const Script& s, std::size_t v) const {
    const Slot& slot = m_slots[v];
    const Voice& voice = m_mixer.voices()[v];
    return slot.owner == s.handle && voice.generation() == slot.generation && voice.active();
}

std::vector<std::size_t> SoundPlayer::voices_of(const Script& s) const {
    std::vector<std::size_t> out;

    for (std::size_t v = 0; v < kVoiceCount; ++v) {
        if (owns(s, v)) {
            out.push_back(v);
        }
    }

    return out;
}

std::optional<std::size_t> SoundPlayer::allocate_voice(int priority) {
    // A free voice, else the lowest-priority one at or below this tone's,
    // the oldest first.
    std::optional<std::size_t> best;

    for (std::size_t v = 0; v < kVoiceCount; ++v) {
        if (!m_mixer.voices()[v].active()) {
            return v;
        }

        const Slot& slot = m_slots[v];

        if (slot.priority > priority) {
            continue;
        }

        if (!best || slot.priority < m_slots[*best].priority
            || (slot.priority == m_slots[*best].priority
                && slot.started < m_slots[*best].started)) {
            best = v;
        }
    }

    return best;
}

std::int32_t SoundPlayer::random() {
    // A deterministic stand-in for the IOP's rand(): the same LCG as
    // newlib's 15-bit rand, seeded with 1.
    m_random = m_random * 0x41c6'4e6du + 0x3039u;
    return static_cast<std::int32_t>((m_random >> 16) & 0x7fff);
}

std::int8_t& SoundPlayer::reg(Script& s, std::int32_t which) {
    if (which < 0) {
        return m_global_registers[static_cast<std::size_t>(std::min(-which - 1, 31))];
    }

    return s.registers[static_cast<std::size_t>(std::min(which, 3))];
}

// A tone's volume or pan selector: -1..-4 a sound register, -5 random,
// below that a global register. `scale` 0 gives the register as it is,
// otherwise register * scale / 127.
std::int32_t SoundPlayer::register_value(const Script& s, std::int32_t selector, int scale) {
    std::int32_t raw = 0;

    if (selector >= -4) {
        raw = s.registers[static_cast<std::size_t>(-selector - 1)];
    } else if (selector == -5) {
        return random() % (scale == 0 ? 0x7f : scale);
    } else {
        raw = m_global_registers[static_cast<std::size_t>(std::min(-selector - 6, 31))];
    }

    return scale == 0 ? raw : scale * raw / 127;
}

void SoundPlayer::unsupported(const char* what) {
    ++m_unsupported[what];
}

void SoundPlayer::start_tone(Script& s, const assets::Tone& tone) {
    s.volume = std::clamp(((s.asked_volume * s.own_volume) >> 10) + s.lfo_volume, 0, 127);
    s.pan = wrap_degrees(s.asked_pan + s.lfo_pan);

    if (tone.flags & assets::kToneNoise) {
        unsupported("noise tone");
        return;
    }

    if (tone.flags & assets::kToneReverbOnly) {
        unsupported("reverb-only tone");
        return;
    }

    const int vol = std::max(tone.volume < 0 ? register_value(s, tone.volume, 0) : tone.volume, 0);
    const int pan = wrap_degrees(tone.pan < 0 ? register_value(s, tone.pan, 360) : tone.pan);
    const auto v = allocate_voice(tone.priority);

    if (!v) {
        unsupported("tone dropped: no voice");
        return;
    }

    Slot& slot = m_slots[*v];
    slot.owner = s.handle;
    slot.started = m_ticks;
    slot.priority = tone.priority;
    slot.tone = tone;
    slot.tone_volume = vol;
    slot.tone_pan = pan;
    slot.base = make_volume(127, 0, s.volume, s.pan, vol, pan);
    Voice& voice = m_mixer.voice(*v);
    voice.key_on_memory(
        s.bank->samples,
        tone.sample_offset,
        tone_pitch(tone, s.pitch_bend, s.pitch_mod),
        tone.adsr1,
        tone.adsr2
    );
    slot.generation = voice.generation();
    voice.set_reverb((tone.flags & assets::kToneToReverb) != 0);
    const std::int32_t master =
        s.group < static_cast<int>(kVolumeGroups) ? m_master[s.group] : kFullVolume;
    voice.set_volume({
        volume_register(group_volume(slot.base[0], master)),
        volume_register(group_volume(slot.base[1], master)),
    });
}

void SoundPlayer::apply_volume(Script& s, std::int32_t volume, std::int32_t pan) {
    if (volume >= 0) {
        if (volume != kVolumeKeep) {
            s.asked_volume = volume;
        }
    } else {
        s.asked_volume = -1024 * volume / 127;
    }

    if (pan == kPanReset) {
        s.asked_pan = s.bank->bank.sounds[s.sound].pan;
    } else if (pan != kPanKeep) {
        s.asked_pan = pan;
    }

    const std::int32_t new_volume =
        std::clamp(((s.asked_volume * s.own_volume) >> 10) + s.lfo_volume, 0, 127);
    const std::int32_t new_pan = wrap_degrees(s.asked_pan + s.lfo_pan);

    if (new_volume == s.volume && new_pan == s.pan) {
        return;
    }

    s.volume = new_volume;
    s.pan = new_pan;
    const std::int32_t master =
        s.group < static_cast<int>(kVolumeGroups) ? m_master[s.group] : kFullVolume;

    for (const std::size_t v : voices_of(s)) {
        Slot& slot = m_slots[v];
        slot.base = make_volume(127, 0, s.volume, s.pan, slot.tone_volume, slot.tone_pan);

        if (!s.paused) {
            m_mixer.voice(v).set_volume({
                volume_register(group_volume(slot.base[0], master)),
                volume_register(group_volume(slot.base[1], master)),
            });
        }
    }
}

void SoundPlayer::apply_pitch(Script& s) {
    s.pitch_mod = s.asked_mod + s.lfo_mod;
    s.pitch_bend = std::clamp<std::int32_t>(s.asked_bend + s.lfo_bend, -0x8000, 0x7fff);

    if (s.paused) {
        return;
    }

    for (const std::size_t v : voices_of(s)) {
        m_mixer.voice(v).set_pitch(tone_pitch(m_slots[v].tone, s.pitch_bend, s.pitch_mod));
    }
}

void SoundPlayer::tick_lfos(Script& s) {
    for (Lfo& l : s.lfos) {
        ++l.ticks;

        // The library's LFOs move every other tick.
        if (l.target == 0 || (l.ticks & 1) == 0) {
            continue;
        }

        const std::int32_t step = l.next_step >> 16;
        l.next_step += static_cast<std::int32_t>(2 * l.step_size);

        if (l.next_step > 0x7ffffff) {
            l.next_step -= 0x8000000;
        }

        std::int32_t wave = 0;

        switch (l.shape) {
            case 1:
                wave = lfo_sine(step);
                break;
            case 2:
                wave = step >= l.hold1 ? -32767 : 32767;
                break;
            case 3:
                wave = step < 512     ? 0x7fff * step / 512
                       : step >= 1536 ? 0x7fff * (step - 1536) / 512 - 0x7fff
                                      : 0x7fff - 65534 * (step - 512) / 1024;
                break;
            case 4:
                wave = step >= 1024 ? 0x7fff * (step - 1024) / 1024 - 0x7fff : 0x7fff * step / 1023;
                break;
            case 5:
                if (step >= 1024 && l.hold2 == 1) {
                    l.hold2 = 0;
                    l.hold1 = 2 * ((random() & 0x7fff) - 0x3fff);
                } else if (step < 1024 && l.hold2 == 0) {
                    l.hold2 = 1;
                    l.hold1 = -(random() & 0x7fff) * (random() & 1);
                }

                wave = l.hold1;
                break;
            default:
                break;
        }

        if (l.flags & 1) {
            wave = -wave;
        }

        switch (l.target) {
            case 1:
                if (const std::int32_t v = (l.range * (wave - 0x7fff)) >> 16; v != s.lfo_volume) {
                    s.lfo_volume = v;
                    apply_volume(s, kVolumeKeep, kPanKeep);
                }

                break;
            case 2:
                if (const std::int32_t p = (l.range * wave) >> 15; p != s.lfo_pan) {
                    s.lfo_pan = p;
                    apply_volume(s, kVolumeKeep, kPanKeep);
                }

                break;
            case 3:
                if (const std::int32_t m = (wave * l.range) >> 15; m != s.lfo_mod) {
                    s.lfo_mod = m;
                    apply_pitch(s);
                }

                break;
            case 4:
                if (const std::int32_t b = (wave * l.range) >> 15; b != s.lfo_bend) {
                    s.lfo_bend = b;
                    apply_pitch(s);
                }

                break;
            default:
                break;
        }
    }
}

void SoundPlayer::run_grain(Script& s) {
    const auto at = static_cast<std::size_t>(s.next);
    const assets::Grain& g = s.grains[at];
    std::int32_t extra_delay = 0;
    const auto p = g.params();
    auto count = static_cast<std::int32_t>(s.grains.size());

    // Sets up RAND_PLAY / PLAY_CYCLE: play `run` grains from `offset`, then
    // skip `skip`.
    auto choose = [&](std::int32_t offset, std::uint32_t run, std::uint32_t skip) {
        s.next += offset;
        s.to_play = run;
        s.to_skip = skip;
        s.skipping = true;
    };
    auto find = [&](GrainType type,
                    std::int32_t from,
                    std::int32_t direction,
                    std::optional<std::int16_t> mark = {}) {
        for (std::int32_t i = from; i >= 0 && i < count; i += direction) {
            if (s.grains[static_cast<std::size_t>(i)].type == type
                && (!mark || s.grains[static_cast<std::size_t>(i)].params()[0] == *mark)) {
                return std::optional<std::int32_t>(i);
            }
        }

        return std::optional<std::int32_t>();
    };
    auto clamp8 = [](std::int32_t v) {
        return static_cast<std::int8_t>(std::clamp(v, -128, 127));
    };

    switch (g.type) {
        case GrainType::Tone:
        case GrainType::Tone2:
            start_tone(s, g.tone());
            break;
        case GrainType::LfoSettings: {
            const assets::LfoSettings set = g.lfo();
            Lfo& l = s.lfos[set.which & 3];
            l = Lfo{};
            l.target = set.target;

            if (l.target != 0) {
                l.shape = set.shape;
                l.flags = set.flags;
                l.depth = set.depth;
                l.step_size = set.step_size;
                l.hold1 = l.shape == 2 ? set.duty_cycle : 0;
                l.next_step = (set.flags & 2) ? (random() & 0x7ff) << 16 : set.start_offset << 16;

                if (l.shape == 5) {
                    l.hold1 = -(random() & 0x7fff) * (random() & 1);
                    l.hold2 = 1;
                }

                switch (l.target) {
                    case 1:
                        l.range = (s.own_volume * l.depth) >> 10;
                        break;
                    case 2:
                        l.range = (180 * l.depth) >> 10;
                        break;
                    case 3:
                        l.range = (6096 * l.depth) >> 10;
                        break;
                    case 4:
                        l.range = (0x7fff * l.depth) >> 10;
                        break;
                    default:
                        break;
                }
            }

            break;
        }
        case GrainType::LoopEnd:
            if (const auto start = find(GrainType::LoopStart, s.next - 1, -1)) {
                s.next = *start - 1;
            }

            break;
        case GrainType::LoopContinue:
            if (const auto end = find(GrainType::LoopEnd, s.next + 1, 1)) {
                s.next = *end;
            }

            break;
        case GrainType::Stop:
            s.done = true;
            break;
        case GrainType::RandPlay: {
            const std::int32_t options = std::max<std::int32_t>(p[0], 1);
            std::int16_t& previous = s.bank->grain_state[s.first_grain + at];
            std::int32_t pick = random() % options;

            if (pick == previous && ++pick >= options) {
                pick = 0;
            }

            previous = static_cast<std::int16_t>(pick);
            choose(
                pick * p[1],
                static_cast<std::uint32_t>(p[1] + 1),
                static_cast<std::uint32_t>((options - 1 - pick) * p[1])
            );
            break;
        }
        case GrainType::PlayCycle: {
            std::int16_t& position = s.bank->grain_state[s.first_grain + at];
            const std::int32_t current = position;
            position = static_cast<std::int16_t>(current + 1 == p[0] ? 0 : current + 1);
            choose(
                p[1] * current,
                static_cast<std::uint32_t>(p[1] + 1),
                static_cast<std::uint32_t>((p[0] - 1 - current) * p[1])
            );
            break;
        }
        case GrainType::RandDelay:
            extra_delay = random() % std::max(g.rand_delay_amount(), 1);
            break;
        case GrainType::RandPitchBend:
            s.asked_bend = p[0] * ((0xffff * (random() % 0x7fff)) / 0x7fff - 0x8000) / 100;
            apply_pitch(s);
            break;
        case GrainType::AddPitchBend:
            s.asked_bend =
                std::clamp<std::int32_t>(s.pitch_bend + 0x7fff * p[0] / 127, -0x8000, 0x7fff);
            apply_pitch(s);
            break;
        case GrainType::PitchBend:
            s.asked_bend = p[0] >= 0 ? 0x7fff * p[0] / 127 : -0x8000 * p[0] / -128;
            apply_pitch(s);
            break;
        case GrainType::SetRegister:
            reg(s, p[0]) = static_cast<std::int8_t>(p[1]);
            break;
        case GrainType::SetRegisterRand:
            reg(s, p[0]) = static_cast<std::int8_t>(random() % std::max(p[2] - p[1] + 1, 1) + p[1]);
            break;
        case GrainType::IncRegister:
            reg(s, p[0]) = clamp8(reg(s, p[0]) + 1);
            break;
        case GrainType::DecRegister:
            reg(s, p[0]) = clamp8(reg(s, p[0]) - 1);
            break;
        case GrainType::AddRegister:
            reg(s, p[1]) = clamp8(reg(s, p[1]) + p[0]);
            break;
        case GrainType::CopyRegister:
            reg(s, p[1]) = reg(s, p[0]);
            break;
        case GrainType::TestRegister: {
            // Skips the next grain unless the test holds.
            const std::int32_t value = reg(s, p[0]);
            const bool skip = p[1] == 0 ? value >= p[2] : p[1] == 1 ? value != p[2] : p[2] >= value;

            if (skip) {
                ++s.next;
            }

            break;
        }
        case GrainType::GotoMarker:
            if (const auto m = find(GrainType::Marker, 0, 1, p[0])) {
                s.next = *m - 1;
            }

            break;
        case GrainType::GotoRandomMarker: {
            const std::int32_t mark = random() % std::max(p[1] - p[0] + 1, 1) + p[0];

            if (const auto m = find(GrainType::Marker, 0, 1, static_cast<std::int16_t>(mark))) {
                s.next = *m - 1;
            }

            break;
        }
        case GrainType::WaitForAllVoices:
            if (!voices_of(s).empty()) {
                --s.next;
                extra_delay = 1;
            }

            break;
        case GrainType::KeyOffVoices:
            for (const std::size_t v : voices_of(s)) {
                m_mixer.voice(v).key_off();
            }

            break;
        case GrainType::KillVoices:
            for (const std::size_t v : voices_of(s)) {
                m_mixer.voice(v).key_off();
                m_mixer.voice(v).set_volume({0, 0});
            }

            break;
        case GrainType::OnStopMarker:
            s.next = count - 1;
            break;
        case GrainType::Null:
        case GrainType::ControlNull:
        case GrainType::LoopStart:
        case GrainType::Marker:
            break;
        case GrainType::XrefId:
        case GrainType::XrefNum:
            unsupported("cross-bank reference grain");
            break;
        case GrainType::StartChildSound:
        case GrainType::StopChildSound:
        case GrainType::Branch:
            unsupported("child sound or branch grain");
            break;
        case GrainType::PluginMessage:
            unsupported("plugin message grain");
            break;
    }

    if (s.skipping && --s.to_play == 0) {
        s.next += static_cast<std::int32_t>(s.to_skip);
        s.skipping = false;
    }

    ++s.next;

    if (s.next >= count) {
        s.done = true;
        return;
    }

    s.countdown = s.grains[static_cast<std::size_t>(s.next)].delay + extra_delay;
}

}  // namespace openrac::audio
