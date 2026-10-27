// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-vubench: how fast, and whether still the same.
 *
 * Takes one frame's display list as `openrac-boot --dump-vif` wrote it and
 * gives it to VIF1 again and again, with VU1 running the game's own
 * microprograms on it. It times the vector unit alone and prints a number
 * worked out from everything VU1 sends to the GIF and from its registers at
 * the end: a change to the interpreter that changes that number changed what
 * the programs compute.
 *
 * The file holds a game's programs and data. It is made on your machine from
 * your disc and stays there.
 *
 * The file is the 8 bytes "ORVIF1" and two zeros, VU1's program memory, VU1's data memory as they
 * were before the frame, and then the bytes VIF1 was given. It leaves out the GS: what VU1 sends
 * to it is summed and dropped.
 */

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "ps2/fp_quad.h"
#include "ps2/gif.h"
#include "ps2/gs.h"
#include "ps2/vif.h"
#include "ps2/vu.h"

using namespace ps2;

/**
 * Replays a recorded display list through VIF1 and VU1 and prints timings and a checksum.
 *
 * @param argc Number of command line arguments.
 * @param argv The program name, the file `openrac-boot --dump-vif` wrote, and optionally how many
 *     times to play the frame (20 when left out).
 * @return 0 when the run completed, 1 when the file cannot be read or is not a dump, 2 when the
 *     command line has no file.
 */
int main(int argc, char** argv) {
    // The dump file is the one required argument.
    if (argc < 2) {
        std::fprintf(stderr, "usage: openrac-vubench FILE [TIMES]\n");
        return 2;
    }

    // The frame is played this many times; 20 is enough for a stable timing.
    int times = argc > 2 ? std::atoi(argv[2]) : 20;
    std::FILE* f = std::fopen(argv[1], "rb");

    // The file cannot be opened.
    if (!f) {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    std::vector<u8> file;
    u8 buffer[65536];

    // Read the whole file in 64 KiB pieces; the loop ends when a read returns nothing.
    for (std::size_t n; (n = std::fread(buffer, 1, sizeof(buffer), f)) > 0;) {
        file.insert(file.end(), buffer, buffer + n);
    }
    std::fclose(f);

    // The header is the 8-byte magic and the two memories; VIF1's bytes follow it.
    const std::size_t head = 8 + 2 * Vif1::kMemoryBytes;

    // Too short for the header, or the magic is wrong: not a dump written by openrac-boot.
    if (file.size() < head || std::memcmp(file.data(), "ORVIF1\0", 8) != 0) {
        std::fprintf(stderr, "%s is not a file written by openrac-boot --dump-vif\n", argv[1]);
        return 1;
    }

    // The GS only takes what VIF1 passes straight on (texture data, mostly);
    // what VU1 kicks is summed up and dropped.
    Gs gs;
    Gif gif(gs);
    Vif1 vif(gif);
    Vu vu1(Vu::Memory{vif.micro.data(), Vif1::kMemoryBytes, vif.data.data(), Vif1::kMemoryBytes});

    // Put VU1's two memories back as they were before the recorded frame.
    std::memcpy(vif.micro.data(), file.data() + 8, Vif1::kMemoryBytes);
    std::memcpy(vif.data.data(), file.data() + 8 + Vif1::kMemoryBytes, Vif1::kMemoryBytes);

    // The checksum starts at the FNV-1a 64-bit offset basis; the rest count what happened.
    u64 sum = 0xCBF29CE484222325ull, kicks = 0, kicked = 0, instructions = 0, starts = 0;
    std::chrono::steady_clock::duration in_vu{};

    // Folds bytes into `sum`, eight at a time, with the FNV-1a prime 0x100000001B3. Called below.
    auto mix = [&sum](const u8* data, std::size_t bytes) {
        for (std::size_t n = 0; n + 8 <= bytes; n += 8) {
            sum = (sum ^ load<u64>(data + n)) * 0x100000001B3ull;
        }
    };

    // As the machine does for VU1: nothing outside it reads its flags.
    vu1.skip_unread_flags = true;

    // XTOP and XITOP read the VIF's registers; called by VU1 while it runs.
    vu1.on_top = [&] {
        return vif.top;
    };
    vu1.on_itop = [&] {
        return vif.itop;
    };

    // Called by VU1 at XGKICK: sums the packet it names instead of sending it to the GIF.
    vu1.on_kick = [&](u32 quadword) {
        // Data memory is a power of two of 16-byte quadwords, so this masks an index into it.
        const u32 mask = Vif1::kMemoryBytes / 16 - 1;
        u32 at = quadword & mask;
        std::size_t count = Gif::packet_quadwords(vif.data.data(), at, mask);

        // A packet wraps at the end of data memory, so each quadword is masked on its own.
        for (std::size_t n = 0; n < count; n++) {
            mix(&vif.data[((at + n) & mask) * 16], 16);
        }

        kicks++;
        kicked += count;
    };

    u64 loads = 0;

    // Called by VIF1 when MPG has written program memory.
    vif.on_program = [&] {
        vu1.program_changed();
        loads++;
    };

    // Called by VIF1 to start a microprogram, or to continue one (MSCNT): runs VU1 and times it.
    vif.on_start = [&](u32 address, bool resume) {
        auto before = std::chrono::steady_clock::now();

        // Cut a program off after 4 million instructions, the limit the machine uses too.
        instructions += resume ? vu1.resume(4'000'000) : vu1.run(address, 4'000'000);
        in_vu += std::chrono::steady_clock::now() - before;

        // The program left the host rounding towards zero; the rest of the code wants nearest.
        fp::want_nearest();
        starts++;
    };

    auto begin = std::chrono::steady_clock::now();
    u64 first_sum = 0;

    // Play the recorded bytes into VIF1 once per round.
    for (int n = 0; n < times; n++) {
        vif.write(file.data() + head, file.size() - head);

        // After the first round, also sum the registers, so a wrong result shows in the number.
        if (n == 0) {
            // reinterpret_cast hands the register arrays to `mix` as bytes, only to read them.
            for (const auto& reg : vu1.vf) {
                // A float register is four 32-bit values: 16 bytes.
                mix(reinterpret_cast<const u8*>(reg.data()), 16);
            }

            // The sixteen integer registers are 16 bits each: 32 bytes.
            mix(reinterpret_cast<const u8*>(vu1.vi.data()), 32);
            first_sum = sum;
        }
    }

    double total = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
    double vu = std::chrono::duration<double>(in_vu).count();

    // Counts first, then the times per frame, then the checksums.
    std::printf(
        "%d times: %llu program loads, %llu program starts, %llu instruction pairs, %llu packets "
        "of %llu quadwords\n",
        times,
        static_cast<unsigned long long>(loads),
        static_cast<unsigned long long>(starts),
        static_cast<unsigned long long>(instructions),
        static_cast<unsigned long long>(kicks),
        static_cast<unsigned long long>(kicked)
    );
    std::printf(
        "VU1: %.2f ms a frame, %.2f ns a pair; everything: %.2f ms a frame\n",
        vu * 1e3 / times,

        // No instruction ran: avoid dividing by zero.
        instructions ? vu * 1e9 / static_cast<double>(instructions) : 0.0,
        total * 1e3 / times
    );
    std::printf(
        "sum after the first: %016llx, after the last: %016llx\n",
        static_cast<unsigned long long>(first_sum),
        static_cast<unsigned long long>(sum)
    );

    // Report only what went wrong: instructions that decode to nothing, or a program cut off.
    if (vu1.unknown_ops || !vu1.stopped()) {
        std::printf(
            "%llu unknown instructions%s\n",
            static_cast<unsigned long long>(vu1.unknown_ops),
            vu1.stopped() ? "" : ", and the last program did not stop"
        );
    }

    return 0;
}
