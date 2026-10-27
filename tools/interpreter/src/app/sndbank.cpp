// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-sndbank: lists a sound effect bank from a disc and plays its sounds into a file.
 *
 * A game's sound effect banks lie on its disc at sectors the game names when it loads them. This
 * tool reads one, prints every sound with its steps, and plays each sound in turn through the
 * same player the runtime uses, so that a bank can be heard and checked without running the
 * game. The bank is read from your disc and stays on your machine.
 */

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ps2/types.h"
#include "snd/bank.h"
#include "snd/player.h"
#include "sys/disc.h"

using namespace snd;

namespace {

using ps2::store;

/** The bank number and sound number the tool plays under; any would do. */
constexpr u32 kBank = 1;
constexpr u32 kHandle = 1;

/** The longest a sound is given before the tool stops it: four seconds. */
constexpr std::size_t kLongestFrames = 4 * static_cast<std::size_t>(Player::kRate);

/** The silence left after each sound: a quarter of a second. */
constexpr std::size_t kGapFrames = static_cast<std::size_t>(Player::kRate) / 4;

/** What the command line asked for. */
struct Options {
    std::string disc;  // Path of the disc image.
    u32 sector = 0;    // First sector of the bank.
    int sound = -1;    // The one sound to play, or -1 for all of them.
    std::string wav;   // Where to write what was played; empty for nowhere.
};

/**
 * Reads the command line.
 *
 * @param argc Number of arguments.
 * @param argv The arguments.
 * @param[out] options What was asked for.
 * @return False when the disc or the sector is missing.
 */
bool read_options(int argc, char** argv, Options& options) {
    std::vector<std::string> plain;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // The two options take a value; everything else is the disc and the sector.
        if (arg == "--sound" && i + 1 < argc) {
            options.sound = std::atoi(argv[++i]);
        } else if (arg == "--wav" && i + 1 < argc) {
            options.wav = argv[++i];
        } else {
            plain.push_back(arg);
        }
    }

    // Both are needed.
    if (plain.size() != 2) {
        return false;
    }

    options.disc = plain[0];

    // Base 0: the sector may be written in hexadecimal with 0x, as the runtime's trace prints it.
    options.sector = static_cast<u32>(std::strtoul(plain[1].c_str(), nullptr, 0));

    return true;
}

/**
 * Reads a whole bank file from a disc, finding its size in its own table.
 *
 * @param disc The disc.
 * @param sector The file's first sector.
 * @return The file, or nothing when it cannot be read or is not a two-part bank.
 */
std::vector<u8> read_bank_file(sys::Disc& disc, u32 sector) {
    std::vector<u8> first(sys::Disc::kSector);

    // The first sector holds the table.
    if (!disc.read(sector, 1, first.data())) {
        return {};
    }

    std::size_t bytes = Bank::file_bytes(first.data());

    // Not a bank's table.
    if (bytes == 0) {
        return {};
    }

    u32 sectors = static_cast<u32>((bytes + sys::Disc::kSector - 1) / sys::Disc::kSector);
    std::vector<u8> file(std::size_t{sectors} * sys::Disc::kSector);

    // The disc ends before the bank does.
    if (!disc.read(sector, sectors, file.data())) {
        return {};
    }

    file.resize(bytes);

    return file;
}

/**
 * Prints one line for each sound of a bank and one for each of its steps.
 *
 * @param bank The bank.
 */
void list_sounds(const Bank& bank) {
    for (std::size_t n = 0; n < bank.sounds.size(); n++) {
        const Sfx& sound = bank.sounds[n];

        std::printf(
            "sound %zu: volume %d, group %d, pan %d, limit %d, flags %x, %zu steps\n",
            n,
            sound.volume,
            sound.group,
            sound.pan,
            sound.instance_limit,
            sound.flags,
            sound.grains.size()
        );

        for (const Grain& grain : sound.grains) {
            std::printf(
                "    step type %u after %d ticks, parameters %d %d %d\n",
                static_cast<u32>(grain.type),
                grain.delay,
                grain.parameters[0],
                grain.parameters[1],
                grain.parameters[2]
            );
        }
    }
}

/**
 * Plays one sound until it is over or has had its four seconds, then a gap.
 *
 * @param player The player, with the bank loaded under `kBank`.
 * @param index The sound's index in the bank.
 * @param[out] heard Receives the samples, left and right in turn.
 */
void play_sound(Player& player, u32 index, std::vector<s16>& heard) {
    SoundStart how;
    std::size_t frames = 0;
    s32 loudest = 0;

    how.index = index;

    // The player refused it.
    if (!player.play(kHandle, kBank, how)) {
        std::printf("sound %u: did not start\n", index);
        return;
    }

    // A tick at a time, so that the end is found to the tick.
    while (player.is_playing(kHandle) && frames < kLongestFrames) {
        s32 sums[2 * Player::kTickFrames] = {};

        player.mix(Player::kTickFrames, sums);
        frames += Player::kTickFrames;

        for (s32 sum : sums) {
            s32 sample = std::clamp(sum, -32768, 32767);

            loudest = std::max(loudest, std::abs(sample));
            heard.push_back(static_cast<s16>(sample));
        }
    }

    bool cut = player.is_playing(kHandle);

    // A sound that loops for as long as the game wants it is cut off here.
    player.stop_all();
    heard.insert(heard.end(), 2 * kGapFrames, s16{0});

    std::printf(
        "sound %u: %.2f seconds%s, loudest sample %d\n",
        index,
        static_cast<double>(frames) / Player::kRate,
        cut ? " (stopped by the tool)" : "",
        loudest
    );
}

/**
 * Writes samples as a sound file: 16 bits, two channels, the player's rate.
 *
 * @param path Where to write.
 * @param heard The samples, left and right in turn.
 * @return False when the file cannot be made.
 */
bool write_wav(const std::string& path, const std::vector<s16>& heard) {
    std::FILE* file = std::fopen(path.c_str(), "wb");

    // The path cannot be written.
    if (!file) {
        return false;
    }

    u32 bytes = static_cast<u32>(heard.size() * 2);
    u32 rate = Player::kRate;
    u8 head[44] = {'R', 'I', 'F', 'F', 0,  0, 0,   0,   'W', 'A', 'V', 'E', 'f', 'm', 't',
                   ' ', 16,  0,   0,   0,  1, 0,   2,   0,   0,   0,   0,   0,   0,   0,
                   0,   0,   4,   0,   16, 0, 'd', 'a', 't', 'a', 0,   0,   0,   0};

    /*
     * RIFF size at byte 4 (the data plus the 36 bytes after it), sample rate at 24, bytes a
     * second at 28 (rate times 4 bytes a frame), data size at 40 (documented).
     */
    store<u32>(head + 4, bytes + 36);
    store<u32>(head + 24, rate);
    store<u32>(head + 28, rate * 4);
    store<u32>(head + 40, bytes);
    std::fwrite(head, 1, sizeof(head), file);
    std::fwrite(heard.data(), 2, heard.size(), file);
    std::fclose(file);

    return true;
}

}  // namespace

/**
 * Lists a bank and plays its sounds.
 *
 * @param argc Number of command line arguments.
 * @param argv The program name, the disc image, the bank's sector and the options.
 * @return 0 when the bank was read, 1 when the command line or the bank is bad.
 */
int main(int argc, char** argv) {
    Options options;
    sys::Disc disc;

    // The command line is incomplete.
    if (!read_options(argc, argv, options)) {
        std::fprintf(stderr, "usage: openrac-sndbank DISC.iso SECTOR [--sound N] [--wav FILE]\n");
        return 1;
    }

    // No such disc image.
    if (!disc.open(options.disc)) {
        std::fprintf(stderr, "openrac-sndbank: cannot open %s\n", options.disc.c_str());
        return 1;
    }

    std::vector<u8> file = read_bank_file(disc, options.sector);
    std::unique_ptr<Bank> bank = Bank::parse(file.data(), file.size());
    Player player;

    // Something else lies at that sector.
    if (!bank || !player.load_bank(kBank, file.data(), file.size())) {
        std::fprintf(
            stderr, "openrac-sndbank: no sound effect bank at sector %u\n", options.sector
        );
        return 1;
    }

    std::printf(
        "version %u, %zu sounds, %zu bytes of samples\n",
        bank->version,
        bank->sounds.size(),
        bank->samples.size()
    );
    list_sounds(*bank);

    std::vector<s16> heard;

    for (std::size_t n = 0; n < bank->sounds.size(); n++) {
        // Only one sound was asked for, and it is another.
        if (options.sound >= 0 && static_cast<std::size_t>(options.sound) != n) {
            continue;
        }

        play_sound(player, static_cast<u32>(n), heard);
    }

    // A sound file was asked for and cannot be written.
    if (!options.wav.empty() && !write_wav(options.wav, heard)) {
        std::fprintf(stderr, "openrac-sndbank: cannot write %s\n", options.wav.c_str());
        return 1;
    }

    return 0;
}
