// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-vuscan: finds the VU microprograms stored in a file (a game's
 * executable, read from your own disc) and reports whether the interpreter
 * knows every instruction in them. It prints counts and instruction words it
 * could not decode, never the programs themselves.
 *
 * A stored program is looked for as the games store them for VIF1: a run of
 * VIF codes that begins with FLUSH and MPG and goes on with MPG blocks.
 *
 * Each run is loaded through a VIF1 as the game would load it, then every instruction pair that
 * landed in program memory is run alone on a vector unit to see whether it decodes. It leaves out
 * running the programs: decoding is necessary, not sufficient.
 *
 * Sources: the VIF1 codes and the vector unit's instruction layout, as publicly documented.
 */

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "ps2/gif.h"
#include "ps2/gs.h"
#include "ps2/vif.h"
#include "ps2/vu.h"

using namespace ps2;

namespace {

/** Fills program memory before a load; a pair still holding it afterwards was never written. */
constexpr u32 kEmpty = 0xDEADC0DE;

/**
 * Reads a whole file into memory.
 *
 * @param path Host path of the file.
 * @return The file's bytes, or an empty vector when it cannot be read.
 */
std::vector<u8> read_file(const char* path) {
    std::vector<u8> bytes;

    // The file opened; otherwise the result stays empty.
    if (std::FILE* f = std::fopen(path, "rb")) {
        // Find the size by seeking to the end, then go back to read.
        std::fseek(f, 0, SEEK_END);
        long size = std::ftell(f);
        std::fseek(f, 0, SEEK_SET);
        bytes.resize(static_cast<std::size_t>(size));

        // A short read leaves a file with a hole in it: give back nothing instead.
        if (std::fread(bytes.data(), 1, bytes.size(), f) != bytes.size()) {
            bytes.clear();
        }
        std::fclose(f);
    }

    return bytes;
}

/**
 * Does the interpreter decode this pair? Run it alone and see.
 *
 * @param upper The upper instruction word of the pair.
 * @param lower The lower instruction word of the pair.
 * @return True if running the pair counted no unknown instruction.
 */
bool known(u32 upper, u32 lower) {
    // Program and data memory of 4 KB each, the size of VU0's, for a unit of one pair.
    static std::array<u8, 4096> micro, data;

    // The lower word comes first in memory, the upper word after it.
    store<u32>(&micro[0], lower);
    store<u32>(&micro[4], upper);
    Vu vu(Vu::Memory{micro.data(), 4096, data.data(), 4096});
    vu.run(0, 1);

    return vu.unknown_ops == 0;
}

}  // namespace

/**
 * Scans a file for stored VU microprograms and prints a line for each, and a total.
 *
 * @param argc Number of command line arguments.
 * @param argv The program name, the file to scan, and optionally the first and last byte offset
 *     to look at (decimal, or hexadecimal with 0x).
 * @return 0 when every instruction found decodes, 1 when one does not or the file cannot be read,
 *     2 when the command line has no file.
 */
int main(int argc, char** argv) {
    // The file is the one required argument.
    if (argc < 2) {
        std::fprintf(stderr, "usage: openrac-vuscan FILE [FIRST_OFFSET LAST_OFFSET]\n");
        return 2;
    }

    std::vector<u8> file = read_file(argv[1]);

    // Unreadable or empty: there is nothing to scan.
    if (file.empty()) {
        std::fprintf(stderr, "cannot read %s\n", argv[1]);
        return 1;
    }

    std::size_t first = argc > 2 ? std::strtoul(argv[2], nullptr, 0) : 0;
    std::size_t last = argc > 3 ? std::strtoul(argv[3], nullptr, 0) : file.size();
    last = std::min(last, file.size());

    int programs = 0;
    u64 total = 0, total_unknown = 0;

    // Scan one word at a time; after a program, resume at its end.
    for (std::size_t at = first & ~std::size_t{3}; at + 8 <= last; at += 4) {
        // VIF codes are CMD in bits 24-30, so a program starts with FLUSH (0x11) and then MPG
        // (0x4A) (documented).
        if (load<u32>(&file[at]) != 0x11000000u || (load<u32>(&file[at + 4]) >> 24) != 0x4A) {
            continue;
        }

        // The run: FLUSH, then MPG blocks (a count of instructions, eight bytes
        // each), with NOPs between them allowed.
        std::size_t end = at + 4;

        // Extend the run block by block; it ends at the first word that is neither.
        while (end + 4 <= last) {
            u32 code = load<u32>(&file[end]);

            // An MPG: NUM in bits 16-23 gives the instruction count, and 0 means 256 (documented).
            if ((code >> 24) == 0x4A) {
                u32 count = (code >> 16) & 0xFF;
                std::size_t next = end + 4 + std::size_t{count ? count : 256} * 8;

                // The block runs past the end of the file: it cannot be a whole program.
                if (next > last) {
                    break;
                }

                end = next;
            } else if (code == 0 && end + 8 <= last && (load<u32>(&file[end + 4]) >> 24) == 0x4A) {
                // A NOP between two MPG blocks is part of the run.
                end += 4;
            } else {
                // Anything else ends the program.
                break;
            }
        }

        // The run's length in 16-byte quadwords, rounded up.
        u32 quadwords = static_cast<u32>((end - at + 15) / 16);

        // Load it the way the game does, through VIF1, and see what landed.
        Gs gs;
        Gif gif(gs);
        Vif1 vif(gif);

        // Mark every word, so that a pair the load did not write can be told from one it did.
        for (std::size_t i = 0; i < vif.micro.size(); i += 4) {
            store<u32>(&vif.micro[i], kEmpty);
        }

        vif.write(&file[at], end - at);

        // `lowest` starts at the largest value so the first instruction found replaces it.
        u32 instructions = 0, immediates = 0, unknown = 0, lowest = 0xFFFFFFFF, highest = 0;
        u32 mac_readers = 0, status_readers = 0, clip_ops = 0, clip_readers = 0;
        std::map<u64, u32> unknown_words;

        // Visit every instruction slot of program memory, 8 bytes each.
        for (u32 n = 0; n < Vif1::kMemoryBytes / 8; n++) {
            u32 lower = load<u32>(&vif.micro[n * 8]), upper = load<u32>(&vif.micro[n * 8 + 4]);

            // Neither half was written by the load: an empty slot.
            if (lower == kEmpty && upper == kEmpty) {
                continue;
            }

            instructions++;
            lowest = std::min(lowest, n);
            highest = std::max(highest, n);

            // Bit 31 of the upper word is the I bit: the lower word is a number for I, not code.
            if (upper & 0x80000000u) {
                immediates++;
            } else {
                /*
                 * Lower opcode in bits 25-31. The flag instructions are 0x10-0x1C: FMEQ, FMAND
                 * and FMOR read MAC; FSEQ to FSOR read status; FCEQ, FCAND, FCOR and FCGET read
                 * clip (documented).
                 */
                u32 op = lower >> 25;
                mac_readers += op == 0x18 || op == 0x1A || op == 0x1B;
                status_readers += op >= 0x14 && op <= 0x17;
                clip_readers += op == 0x10 || op == 0x12 || op == 0x13 || op == 0x1C;
            }

            // The low 11 bits of an upper word being 0x1FF is CLIP (documented).
            clip_ops += (upper & 0x7FF) == 0x1FF;

            // The interpreter counts this pair as unknown; remember the words, by number of times.
            if (!known(upper, lower)) {
                unknown++;
                unknown_words[(u64{upper} << 32) | lower]++;
            }
        }

        // One line per program: where it is and what it holds.
        std::printf(
            "program at 0x%zx: %u quadwords, %u instructions at %u-%u, %u with a number for I, %u "
            "not decoded, "
            "%llu codes VIF1 did not know; flag readers: %u MAC, %u status, %u clip (%u CLIPs)\n",
            at,
            quadwords,
            instructions,
            lowest,
            highest,
            immediates,
            unknown,
            static_cast<unsigned long long>(vif.unknown_codes),
            mac_readers,
            status_readers,
            clip_readers,
            clip_ops
        );
        int shown = 0;

        // List the undecoded words, upper and lower, with how often each occurs.
        for (const auto& [word, count] : unknown_words) {
            // Twelve are enough to see what is missing; the rest would only be noise.
            if (shown++ == 12) {
                std::printf("    ...\n");
                break;
            }
            std::printf(
                "    upper %08x lower %08x  x%u\n",
                static_cast<u32>(word >> 32),
                static_cast<u32>(word),
                count
            );
        }

        programs++;
        total += instructions;
        total_unknown += unknown;

        // Go on after the program; the loop's own step adds the 4 bytes taken off here.
        at = end - 4;
    }

    std::printf(
        "%d programs, %llu instructions, %llu not decoded\n",
        programs,
        static_cast<unsigned long long>(total),
        static_cast<unsigned long long>(total_unknown)
    );

    // A nonzero exit tells a script that some instruction did not decode.
    return total_unknown ? 1 : 0;
}
