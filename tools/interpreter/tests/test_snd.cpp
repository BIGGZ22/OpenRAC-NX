// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Tests of the sound effect side of the games' sound library: voices, banks and the player.
 *
 * The banks and samples are built here by the helpers below. What the tests expect follows from
 * the sound processor's documented sample and envelope formats, from the library's behaviour as
 * the open reimplementation in jak-project shows it, and from arithmetic.
 */

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "check.h"
#include "snd/bank.h"
#include "snd/envelope.h"
#include "snd/player.h"
#include "snd/tuning.h"
#include "snd/voice.h"

using namespace snd;

namespace {

/** A voice level of one half, as a 15-bit fraction. */
constexpr s32 kHalfLevel = 0x4000;

/** The envelope's highest level (documented). */
constexpr s32 kFullEnvelope = 0x7FFF;

/** The value every stored sample of a test block decodes to: the largest digit, unshifted. */
constexpr s32 kSampleValue = 0x7000;

/** The fastest straight attack, no decay, and a sustain that holds the full level. */
constexpr u16 kFastAdsr1 = 0x000F;

/** A sustain that does not fall, and the fastest straight release. */
constexpr u16 kFastAdsr2 = 0x0000;

/** Sample block flags (documented): the last block, go back after it, the loop starts here. */
constexpr u8 kFlagLast = 1;
constexpr u8 kFlagRepeat = 2;
constexpr u8 kFlagLoopStart = 4;

/** Samples in one block (documented). */
constexpr std::size_t kBlockSamples = 28;

/**
 * Appends one sample block whose 28 samples all decode to `kSampleValue`.
 *
 * @param out The sample data to add to.
 * @param flags The block's flags.
 */
void add_block(std::vector<u8>& out, u8 flags) {
    // Shift 0 and predictor 0: each digit is the sample's top four bits.
    out.push_back(0);
    out.push_back(flags);

    // Fourteen bytes of two digits, each the largest positive one.
    for (unsigned n = 0; n < 14; n++) {
        out.push_back(0x77);
    }
}

/**
 * Builds a version 1 sound effect bank file from sounds described step by step.
 */
class BankFile {
public:
    /** A value for `add_tone`: play a sample that ends by itself, or one that goes round. */
    enum class Sample {
        OneShot,
        Looping,
    };

    /**
     * Makes a bank with two samples: three blocks that end, and two blocks that go round.
     */
    BankFile() {
        add_block(samples_, 0);
        add_block(samples_, 0);
        add_block(samples_, kFlagLast);

        looping_at_ = static_cast<u32>(samples_.size());
        add_block(samples_, kFlagLoopStart);
        add_block(samples_, kFlagLast | kFlagRepeat);
    }

    /**
     * Starts a new sound; the steps added next belong to it.
     *
     * @param volume Its volume, 0 to 127.
     * @param group Its volume group.
     * @param flags Its flags (the instance limit bits).
     * @param limit How many may play at once.
     * @return The sound's index in the bank.
     */
    u32 add_sound(s8 volume, s8 group, u16 flags, s8 limit) {
        Sound sound;

        sound.volume = volume;
        sound.group = group;
        sound.flags = flags;
        sound.limit = limit;
        sound.first_grain = static_cast<u32>(grains_.size());
        sounds_.push_back(sound);

        return static_cast<u32>(sounds_.size() - 1);
    }

    /**
     * Adds a tone step to the newest sound, at full volume with the fast envelope.
     *
     * @param delay Ticks to wait before it.
     * @param sample Which of the bank's two samples to play.
     */
    void add_tone(s32 delay, Sample sample) {
        std::vector<u8>& step = add_step(GrainType::Tone, delay);

        // The tone record starts 8 bytes into the step: priority, volume, centre note.
        step[8 + 1] = 127;
        step[8 + 2] = static_cast<u8>(-60);
        put(step, 8 + 10, kFastAdsr1, 2);
        put(step, 8 + 12, kFastAdsr2, 2);
        put(step, 8 + 16, sample == Sample::Looping ? looping_at_ : 0, 4);
    }

    /**
     * Adds a control step to the newest sound.
     *
     * @param type What the step does.
     * @param delay Ticks to wait before it.
     * @param p0 Its first parameter.
     * @param p1 Its second parameter.
     * @param p2 Its third parameter.
     */
    void add_control(GrainType type, s32 delay, s16 p0 = 0, s16 p1 = 0, s16 p2 = 0) {
        std::vector<u8>& step = add_step(type, delay);

        // Three half-words at the start of the payload.
        put(step, 8, static_cast<u16>(p0), 2);
        put(step, 10, static_cast<u16>(p1), 2);
        put(step, 12, static_cast<u16>(p2), 2);
    }

    /**
     * Lays the bank out as a file.
     *
     * @return The file's bytes.
     */
    std::vector<u8> bytes() const;

private:
    /** One sound's record before it is laid out. */
    struct Sound {
        s8 volume = 0;        // 0 to 127.
        s8 group = 0;         // The volume group.
        u16 flags = 0;        // The instance limit bits.
        s8 limit = 0;         // How many may play at once.
        u32 first_grain = 0;  // Index of its first step among all steps.
        u8 grains = 0;        // How many steps it has.
    };

    /** Sizes of the format's parts (documented in bank.cpp). */
    static constexpr std::size_t kTableBytes = 0x18;
    static constexpr std::size_t kHeaderBytes = 0x38;
    static constexpr std::size_t kSoundBytes = 12;
    static constexpr std::size_t kGrainBytes = 0x28;

    /**
     * Writes a little-endian value into a byte vector.
     *
     * @param out The bytes.
     * @param at Offset of the value's first byte.
     * @param value The value.
     * @param size How many bytes of it to write.
     */
    static void put(std::vector<u8>& out, std::size_t at, u32 value, unsigned size) {
        for (unsigned n = 0; n < size; n++) {
            out[at + n] = static_cast<u8>(value >> (n * 8));
        }
    }

    /**
     * Adds an empty step of a kind to the newest sound.
     *
     * @param type What the step does.
     * @param delay Ticks to wait before it.
     * @return The step's bytes, with type and delay filled in.
     */
    std::vector<u8>& add_step(GrainType type, s32 delay) {
        std::vector<u8>& step = grains_.emplace_back(kGrainBytes, u8{0});

        put(step, 0, static_cast<u32>(type), 4);
        put(step, 4, static_cast<u32>(delay), 4);
        sounds_.back().grains++;

        return step;
    }

    std::vector<Sound> sounds_;            // The sounds so far.
    std::vector<std::vector<u8>> grains_;  // Every sound's steps, in order.
    std::vector<u8> samples_;              // The sample data.
    u32 looping_at_ = 0;                   // Offset of the looping sample in `samples_`.
};

std::vector<u8> BankFile::bytes() const {
    std::size_t sounds_at = kHeaderBytes;
    std::size_t grains_at = sounds_at + sounds_.size() * kSoundBytes;
    std::size_t block_bytes = grains_at + grains_.size() * kGrainBytes;
    std::vector<u8> file(kTableBytes + block_bytes, u8{0});

    // The file's table: two parts, the block and the samples, each with offset and size.
    put(file, 4, 2, 4);
    put(file, 8, static_cast<u32>(kTableBytes), 4);
    put(file, 12, static_cast<u32>(block_bytes), 4);
    put(file, 16, static_cast<u32>(kTableBytes + block_bytes), 4);
    put(file, 20, static_cast<u32>(samples_.size()), 4);

    // The block's header: "SBlk", version 1, the sound count, where sounds and steps start.
    std::memcpy(&file[kTableBytes], "SBlk", 4);
    put(file, kTableBytes + 4, 1, 4);
    put(file, kTableBytes + 0x16, static_cast<u32>(sounds_.size()), 2);
    put(file, kTableBytes + 0x1C, static_cast<u32>(sounds_at), 4);
    put(file, kTableBytes + 0x20, static_cast<u32>(grains_at), 4);

    for (std::size_t n = 0; n < sounds_.size(); n++) {
        std::size_t at = kTableBytes + sounds_at + n * kSoundBytes;
        const Sound& sound = sounds_[n];

        file[at] = static_cast<u8>(sound.volume);
        file[at + 1] = static_cast<u8>(sound.group);
        file[at + 4] = sound.grains;
        file[at + 5] = static_cast<u8>(sound.limit);
        put(file, at + 6, sound.flags, 2);
        put(file, at + 8, static_cast<u32>(sound.first_grain * kGrainBytes), 4);
    }

    for (std::size_t n = 0; n < grains_.size(); n++) {
        std::memcpy(
            &file[kTableBytes + grains_at + n * kGrainBytes], grains_[n].data(), kGrainBytes
        );
    }

    file.insert(file.end(), samples_.begin(), samples_.end());

    return file;
}

/** The number tests load their bank under, and the one they start their first sound under. */
constexpr u32 kBank = 7;
constexpr u32 kHandle = 100;

/**
 * Mixes a stretch of sound and says how loud it got.
 *
 * @param player The player.
 * @param frames How many samples to mix.
 * @return The largest size of any left or right sum.
 */
s32 loudest(Player& player, std::size_t frames) {
    std::vector<s32> sums(frames * 2, 0);
    s32 most = 0;

    player.mix(frames, sums.data());

    for (s32 sum : sums) {
        most = std::max(most, sum < 0 ? -sum : sum);
    }

    return most;
}

/**
 * Loads a bank file into a player under `kBank`.
 *
 * @param player The player.
 * @param file The bank to load.
 * @return True if the player took it.
 */
bool load(Player& player, const BankFile& file) {
    std::vector<u8> bytes = file.bytes();

    return player.load_bank(kBank, bytes.data(), bytes.size());
}

/**
 * Starts a sound of the bank under `kBank` with everything else as a caller leaves it.
 *
 * @param player The player.
 * @param handle The number to start it under.
 * @param index The sound's index in the bank.
 * @return True if it started.
 */
bool start(Player& player, u32 handle, u32 index) {
    SoundStart how;

    how.index = index;

    return player.play(handle, kBank, how);
}

/**
 * An envelope rises to the full level, holds it, and falls to nothing when released (documented).
 */
void test_envelope_rises_holds_and_releases() {
    Envelope envelope;

    envelope.set_registers(kFastAdsr1, kFastAdsr2);
    envelope.attack();

    // The fastest attack takes a few samples; a hundred is far more than enough.
    for (unsigned n = 0; n < 100; n++) {
        envelope.run();
    }

    CHECK_EQ(envelope.level(), kFullEnvelope);
    CHECK(envelope.phase() == Envelope::Phase::Sustain);

    envelope.release();

    for (unsigned n = 0; n < 100; n++) {
        envelope.run();
    }

    CHECK_EQ(envelope.level(), s32{0});
    CHECK(envelope.phase() == Envelope::Phase::Stopped);
}

/**
 * A voice plays its stored samples scaled by the envelope and by each side's level (documented).
 */
void test_voice_scales_samples_by_envelope_and_level() {
    std::vector<u8> sample;
    Voice voice;
    s32 left = 0;
    s32 right = 0;

    add_block(sample, 0);
    add_block(sample, kFlagLast);
    voice.key_on(sample.data(), sample.size(), kFastAdsr1, kFastAdsr2);
    voice.set_pitch(Voice::kUnitPitch);
    voice.set_volume(kHalfLevel, 0);

    // One block in, the envelope is at its top and both stored samples around the output agree.
    for (std::size_t n = 0; n < kBlockSamples; n++) {
        left = 0;
        right = 0;
        voice.run(left, right);
    }

    // The sample times the envelope, then times the left level; the right side is silent.
    CHECK_EQ(left, (((kSampleValue * kFullEnvelope) >> 15) * kHalfLevel) >> 15);
    CHECK_EQ(right, s32{0});
}

/**
 * A voice is free again once a sample whose last block does not repeat has run out (documented).
 */
void test_voice_ends_with_its_sample() {
    std::vector<u8> sample;
    Voice voice;
    s32 left = 0;
    s32 right = 0;

    add_block(sample, kFlagLast);
    voice.key_on(sample.data(), sample.size(), kFastAdsr1, kFastAdsr2);
    voice.set_pitch(Voice::kUnitPitch);
    CHECK(!voice.is_free());

    // One sample more than the block holds.
    for (std::size_t n = 0; n < kBlockSamples + 1; n++) {
        voice.run(left, right);
    }

    CHECK(voice.is_free());
}

/**
 * A note an octave above another has twice its pitch (arithmetic).
 */
void test_octave_doubles_the_pitch() {
    Tone tone;

    // A centre note stored negated marks a sample at the processor's own rate.
    tone.center_note = -60;

    CHECK_EQ(note_pitch(tone, Note{60, 0}), Voice::kUnitPitch);
    CHECK_EQ(note_pitch(tone, Note{72, 0}), Voice::kUnitPitch * 2);
    CHECK_EQ(note_pitch(tone, Note{48, 0}), Voice::kUnitPitch / 2);
}

/**
 * A sound straight ahead is as loud on the left as on the right (arithmetic).
 */
void test_centre_pan_is_even() {
    Levels ahead = pan_levels(127, 0, 127, 0, false);
    Levels one_speaker = pan_levels(127, 90, 127, 0, true);

    CHECK(ahead.left > 0);
    CHECK_EQ(ahead.left, ahead.right);

    // With one loudspeaker the pan does nothing.
    CHECK_EQ(one_speaker.left, one_speaker.right);
}

/**
 * A bank file's sounds and steps come out as they were put in (documented format).
 */
void test_bank_is_read_back() {
    BankFile file;

    file.add_sound(100, 3, Sfx::kLimitInstances, 2);
    file.add_tone(5, BankFile::Sample::OneShot);
    file.add_control(GrainType::SetRegister, 9, 2, -7);

    std::vector<u8> bytes = file.bytes();
    std::unique_ptr<Bank> bank = Bank::parse(bytes.data(), bytes.size());

    CHECK(bank != nullptr);
    CHECK_EQ(bank->sounds.size(), std::size_t{1});
    CHECK_EQ(bank->sounds[0].volume, s8{100});
    CHECK_EQ(bank->sounds[0].group, s8{3});
    CHECK_EQ(bank->sounds[0].instance_limit, s8{2});
    CHECK_EQ(bank->sounds[0].grains.size(), std::size_t{2});
    CHECK(bank->sounds[0].grains[0].type == GrainType::Tone);
    CHECK_EQ(bank->sounds[0].grains[0].delay, s32{5});
    CHECK_EQ(bank->sounds[0].grains[0].tone.volume, s8{127});
    CHECK(bank->sounds[0].grains[1].type == GrainType::SetRegister);
    CHECK_EQ(bank->sounds[0].grains[1].parameters[1], s16{-7});
}

/**
 * A file that is cut short, or is not a sound effect block, is refused.
 */
void test_bad_bank_is_refused() {
    BankFile file;

    file.add_sound(100, 0, 0, 0);
    file.add_tone(0, BankFile::Sample::OneShot);

    std::vector<u8> bytes = file.bytes();
    std::vector<u8> renamed = bytes;

    // Offset 0x18 is the block's first byte, the "S" of its name.
    renamed[0x18] = 'X';

    CHECK(Bank::parse(bytes.data(), bytes.size() / 2) == nullptr);
    CHECK(Bank::parse(renamed.data(), renamed.size()) == nullptr);
    CHECK(Bank::parse(bytes.data(), 0) == nullptr);
}

/**
 * A sound with one tone is heard, and is over once its sample has run out.
 */
void test_sound_plays_and_ends() {
    BankFile file;
    Player player;

    file.add_sound(127, 0, 0, 0);
    file.add_tone(0, BankFile::Sample::OneShot);
    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));

    CHECK(player.is_playing(kHandle));
    CHECK(loudest(player, kBlockSamples) > 0);

    // Three blocks of sample and two ticks to notice the voice has ended.
    loudest(player, 3 * kBlockSamples + 2 * Player::kTickFrames);
    CHECK(!player.is_playing(kHandle));
    CHECK_EQ(loudest(player, kBlockSamples), s32{0});
}

/**
 * A stopped sound is released, not cut: it is over a little later, and then silent.
 */
void test_stop_releases_the_voices() {
    BankFile file;
    Player player;

    file.add_sound(127, 0, 0, 0);
    file.add_tone(0, BankFile::Sample::Looping);
    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));

    // A looping sample plays for as long as nobody stops it.
    loudest(player, 10 * Player::kTickFrames);
    CHECK(player.is_playing(kHandle));

    player.stop(kHandle);
    loudest(player, 2 * Player::kTickFrames);
    CHECK(!player.is_playing(kHandle));
    CHECK_EQ(loudest(player, kBlockSamples), s32{0});
}

/**
 * A group at volume zero is silent, and a paused group is silent until it continues.
 */
void test_group_volume_and_pause() {
    BankFile file;
    Player player;

    // Group 2, so that the pause mask below has one bit, bit 2.
    file.add_sound(127, 2, 0, 0);
    file.add_tone(0, BankFile::Sample::Looping);
    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));
    CHECK(loudest(player, Player::kTickFrames) > 0);

    player.set_group_volume(2, 0);
    CHECK_EQ(loudest(player, Player::kTickFrames), s32{0});

    player.set_group_volume(2, 1024);
    CHECK(loudest(player, Player::kTickFrames) > 0);

    player.set_groups_paused(1u << 2, true);
    CHECK_EQ(loudest(player, Player::kTickFrames), s32{0});
    CHECK(player.is_playing(kHandle));

    player.set_groups_paused(1u << 2, false);
    CHECK(loudest(player, Player::kTickFrames) > 0);
}

/**
 * A sound limited to one at a time keeps a second out, or gives way to it when age decides.
 */
void test_instance_limit() {
    BankFile file;
    Player player;

    file.add_sound(127, 0, Sfx::kLimitInstances, 1);
    file.add_tone(0, BankFile::Sample::Looping);
    file.add_sound(127, 0, Sfx::kLimitInstances | Sfx::kLimitByAge, 1);
    file.add_tone(0, BankFile::Sample::Looping);
    CHECK(load(player, file));

    // No rule to pick a loser: the newcomer is kept out.
    CHECK(start(player, kHandle, 0));
    CHECK(!start(player, kHandle + 1, 0));

    // By age: the newcomer gets in and the older one is stopped.
    CHECK(start(player, kHandle + 2, 1));
    CHECK(start(player, kHandle + 3, 1));
    loudest(player, 2 * Player::kTickFrames);
    CHECK(!player.is_playing(kHandle + 2));
    CHECK(player.is_playing(kHandle + 3));
}

/**
 * A register test that fails skips the step after it; one that passes runs it.
 */
void test_register_test_skips_a_step() {
    BankFile file;
    Player player;

    // Kind 1 passes when the register equals the value: 5 is not 4, so the tone is skipped.
    file.add_sound(127, 0, 0, 0);
    file.add_control(GrainType::SetRegister, 0, 0, 5);
    file.add_control(GrainType::TestRegister, 0, 0, 1, 4);
    file.add_tone(0, BankFile::Sample::Looping);
    file.add_control(GrainType::Stop, 0);

    // The same with the register at 4: the tone plays.
    file.add_sound(127, 0, 0, 0);
    file.add_control(GrainType::SetRegister, 0, 0, 4);
    file.add_control(GrainType::TestRegister, 0, 0, 1, 4);
    file.add_tone(0, BankFile::Sample::Looping);
    file.add_control(GrainType::Stop, 0);

    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));
    CHECK_EQ(loudest(player, Player::kTickFrames), s32{0});

    CHECK(start(player, kHandle + 1, 1));
    CHECK(loudest(player, Player::kTickFrames) > 0);
}

/**
 * A script that loops with no delay in it is ended instead of running for ever.
 */
void test_endless_script_is_ended() {
    BankFile file;
    Player player;

    file.add_sound(127, 0, 0, 0);
    file.add_control(GrainType::LoopStart, 0);
    file.add_control(GrainType::LoopEnd, 0);
    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));

    // Two ticks: one to give up on the script, one to forget the sound.
    loudest(player, 2 * Player::kTickFrames);
    CHECK(!player.is_playing(kHandle));
}

/**
 * Unloading a bank ends the sounds that play from it.
 */
void test_unload_ends_the_banks_sounds() {
    BankFile file;
    Player player;

    file.add_sound(127, 0, 0, 0);
    file.add_tone(0, BankFile::Sample::Looping);
    CHECK(load(player, file));
    CHECK(start(player, kHandle, 0));

    player.unload_bank(kBank);
    CHECK(!player.is_playing(kHandle));
    CHECK_EQ(loudest(player, Player::kTickFrames), s32{0});
    CHECK(!start(player, kHandle, 0));
}

}  // namespace

/**
 * Runs every test in the order of the functions above.
 *
 * @return 1 when any check failed, else 0.
 */
int main() {
    const TestCase tests[] = {
        {"an envelope rises, holds and releases", test_envelope_rises_holds_and_releases},
        {"a voice scales samples by envelope and level",
         test_voice_scales_samples_by_envelope_and_level},
        {"a voice ends with its sample", test_voice_ends_with_its_sample},
        {"an octave doubles the pitch", test_octave_doubles_the_pitch},
        {"a centre pan is even", test_centre_pan_is_even},
        {"a bank is read back", test_bank_is_read_back},
        {"a bad bank is refused", test_bad_bank_is_refused},
        {"a sound plays and ends", test_sound_plays_and_ends},
        {"stop releases the voices", test_stop_releases_the_voices},
        {"group volume and pause", test_group_volume_and_pause},
        {"an instance limit", test_instance_limit},
        {"a register test skips a step", test_register_test_skips_a_step},
        {"an endless script is ended", test_endless_script_is_ended},
        {"unloading ends the bank's sounds", test_unload_ends_the_banks_sounds},
    };

    return run_tests(tests);
}
