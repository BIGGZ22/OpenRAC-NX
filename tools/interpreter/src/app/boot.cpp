// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-boot: runs the program on your own disc image and reports how far
 * it gets. A development tool: it shows what the machine model still lacks.
 *
 * It starts a `Machine` on a disc image, runs it field by field, and either shows the pictures
 * and plays the sound in a window or runs headless. Options script the pad, write the last
 * picture or all the sound to a file, record a frame's display list for `openrac-vubench`,
 * list what one frame is drawn with, and set the memory card directory. At the end it prints
 * what the model did not understand. It leaves out any menu or settings of its own.
 *
 * Sources: the command line is its own; the machine's sources are listed in `sys/machine.h`.
 */

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "ps2/vu_dis.h"
#include "sys/machine.h"
#include "sys/snapshot.h"

#include <zlib.h>

#ifndef OPENRAC_NO_WINDOW
#include "host/window.h"
#endif

using namespace ps2;

namespace {

/**
 * Prints how much each of a game's vector unit programs ran on one unit.
 *
 * @param unit The unit's name for the heading.
 * @param chunks The chunks of the game's programs, from its executable.
 * @param uses What the unit ran.
 */
void report_vu_programs(
    const char* unit, const std::vector<VuChunk>& chunks, const std::vector<Vu::Use>& uses
) {
    std::vector<VuProgramUse> programs = vu_program_use(chunks, uses);
    u64 all = 0;

    for (const VuProgramUse& program : programs) {
        all += program.pairs;
    }

    std::fprintf(stderr, "%s programs (program, runs started, instructions, share):\n", unit);

    for (const VuProgramUse& program : programs) {
        double share =
            all ? 100.0 * static_cast<double>(program.pairs) / static_cast<double>(all) : 0.0;

        std::fprintf(
            stderr,
            "  %8u  %10llu  %14llu  %5.1f%%%s\n",
            program.program,
            static_cast<unsigned long long>(program.starts),
            static_cast<unsigned long long>(program.pairs),
            share,
            program.program == 0 ? "  (not in the program's table)" : ""
        );
    }
}

/**
 * Writes a picture as a binary PPM file.
 *
 * @param path Host path of the file to make.
 * @param image The picture; each pixel has red in the low byte, then green and blue.
 * @return False if the file cannot be made.
 */
bool write_ppm(const std::string& path, const Image& image) {
    std::FILE* f = std::fopen(path.c_str(), "wb");

    // The host cannot make the file.
    if (!f) {
        return false;
    }

    // PPM header: format P6 (binary RGB), width, height, and 255 as the largest value of a channel.
    std::fprintf(f, "P6\n%d %d\n255\n", image.width, image.height);

    // One pixel at a time: three bytes, dropping the unused top byte.
    for (u32 p : image.pixels) {
        u8 rgb[3] = {static_cast<u8>(p), static_cast<u8>(p >> 8), static_cast<u8>(p >> 16)};
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
    return true;
}

}  // namespace

/**
 * Runs the program on a disc image.
 *
 * @param argc Number of command line arguments.
 * @param argv The program name, the disc image, and the options listed in the usage message.
 * @return 0 when the run ended, 1 when the disc, the program or the window cannot be opened, 2
 *     for a bad command line.
 */
int main(int argc, char** argv) {
    // Settings from the command line. A frame number of -1 means the option was not given.
    std::string iso, hooks, ppm, vif_file, card, wav, native, native_calls;
    bool card_wanted = true;
    int vif_frame = -1;
    int frames = -1, report = 60, states_frame = -1;

    /*
     * Threads that draw: by default most of the fast cores, leaving one for the program itself.
     * At least 2 cores are assumed, and no more than 12 threads are used.
     */
    int gs_threads =
        static_cast<int>(std::min(12u, std::max(2u, std::thread::hardware_concurrency()) - 1));
    bool window_wanted = false, drawing_thread = true, vu_programs = false;

    /** Scripted input: hold these buttons from one frame for some frames. */
    struct Press {
        /** The first field of the press and how many fields it lasts. */
        int frame, length;

        /** The buttons held, as the pad's bit mask. */
        unsigned buttons;
    };

    std::vector<Press> presses;

    /**
     * A scripted change of the program's memory: a word kept at a value for some fields. For
     * reaching a state that no script of presses reaches, such as starting in a later level.
     */
    struct Write {
        /** The first field of the change and how many fields it lasts. */
        int frame, length;

        /** The guest address of the 32-bit word and the value written at each field's start. */
        unsigned address, value;
    };

    std::vector<Write> writes;

    /**
     * For checking functions by what they do (`openrac-fcheck`): the machine's state is written at
     * the first calls of each listed function. The list has a line per function: its address,
     * how many of its first bytes to compare and their CRC-32 (so that a level's function is
     * taken only where that level's code is loaded), its name, and how many calls to take.
     */
    struct Capture {
        unsigned address, bytes, crc, wanted, taken;
        std::string name;
    };

    std::vector<Capture> captures;
    std::string capture_dir;
    sys::Machine machine;

    // One option per pass; a value-taking option reads the next argument as well.
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--capture" && i + 2 < argc) {
            // A list of functions and the directory their call states go to.
            std::ifstream list(argv[++i]);
            capture_dir = argv[++i];
            std::string line;

            while (std::getline(list, line)) {
                Capture c{};
                char name[128] = "";

                // Lines that do not parse (comments, blank lines) are skipped.
                if (std::sscanf(line.c_str(), "%x %u %x %127s %u", &c.address, &c.bytes, &c.crc, name, &c.wanted) >= 4) {
                    c.name = name;
                    c.wanted = c.wanted ? c.wanted : 3;
                    captures.push_back(c);
                }
            }
        } else if (arg == "--hooks" && i + 1 < argc) {
            // The table that says which addresses of the program are which library functions.
            hooks = argv[++i];
        } else if (arg == "--frames" && i + 1 < argc) {
            // Stop after this many fields.
            frames = std::atoi(argv[++i]);
        } else if (arg == "--report" && i + 1 < argc) {
            // Print counts and the speed every this many fields; 0 turns it off.
            report = std::atoi(argv[++i]);
        } else if (arg == "--verbose" && i + 1 < argc) {
            // The machine's log level (see `Machine::log`).
            machine.verbose = std::atoi(argv[++i]);
        } else if (arg == "--ppm" && i + 1 < argc) {
            // Where to write the last picture.
            ppm = argv[++i];
        } else if (arg == "--press" && i + 1 < argc) {
            // FRAME:BUTTONS[:FRAMES], buttons as a hexadecimal mask (cross is 4000, start 8)
            // The length defaults to 4 fields.
            Press p{0, 4, 0};
            std::sscanf(argv[++i], "%d:%x:%d", &p.frame, &p.buttons, &p.length);
            presses.push_back(p);
        } else if (arg == "--write" && i + 1 < argc) {
            // FRAME:ADDRESS:VALUE[:FRAMES], address and value in hexadecimal; one field by default.
            Write w{0, 1, 0, 0};
            std::sscanf(argv[++i], "%d:%x:%x:%d", &w.frame, &w.address, &w.value, &w.length);
            writes.push_back(w);
        } else if (arg == "--gs-states" && i + 1 < argc) {
            // The field whose drawing states are listed.
            states_frame = std::atoi(argv[++i]);
        } else if (arg == "--gs-threads" && i + 1 < argc) {
            // How many threads the GS draws with (see `Gs::set_threads`).
            gs_threads = std::atoi(argv[++i]);
        } else if (arg == "--dump-vif" && i + 2 < argc) {
            /*
             * For openrac-vubench: one frame's display list as VIF1 gets it, with
             * VU1's memories as they were before it. The file is game data: it is
             * for your own machine.
             */
            vif_frame = std::atoi(argv[++i]);
            vif_file = argv[++i];
        } else if (arg == "--wav" && i + 1 < argc) {
            // Everything heard, as a sound file.
            wav = argv[++i];
        } else if (arg == "--native" && i + 1 < argc) {
            // A library of host code built from the game's decompiled C (runtime/port).
            native = argv[++i];
        } else if (arg == "--native-range" && i + 1 < argc) {
            // Use only the library's functions FIRST up to LAST of its table: FIRST:LAST.
            const char* range = argv[++i];
            const char* colon = std::strchr(range, ':');

            machine.native.first = static_cast<std::size_t>(std::strtoull(range, nullptr, 10));

            // Without a colon the range is open at the top.
            if (colon) {
                machine.native.last =
                    static_cast<std::size_t>(std::strtoull(colon + 1, nullptr, 10));
            }
        } else if (arg == "--native-name" && i + 1 < argc) {
            // Print the name of the function at a place of the library's table.
            machine.native.name_at = std::atol(argv[++i]);
        } else if (arg == "--native-calls" && i + 1 < argc) {
            // Count calls by level and address and write them to this file at the end.
            native_calls = argv[++i];
            machine.native.count_calls = true;
        } else if (arg == "--vu-programs") {
            // At the end, list how much each of the game's vector unit programs ran.
            vu_programs = true;
        } else if (arg == "--native-skip" && i + 1 < argc) {
            // A file of function names, one a line, that the interpreter keeps running.
            std::ifstream names(argv[++i]);
            std::string name;

            while (std::getline(names, name)) {
                machine.native.skip.insert(name);
            }
        } else if (arg == "--native-check" && i + 1 < argc) {
            // Compare this many of each host function's first calls with the retail code.
            machine.native.check = static_cast<unsigned>(std::atoi(argv[++i]));
        } else if (arg == "--card" && i + 1 < argc) {
            // The directory that is the memory card.
            card = argv[++i];
        } else if (arg == "--no-card") {
            // The slot stays empty.
            card_wanted = false;
        } else if (arg == "--one-thread") {
            // The drawing path on the program's own thread.
            drawing_thread = false;
        } else if (arg == "--ntsc") {
            // 60 Hz display: 59.94 fields a second.
            machine.hz = 59.94;
        } else if (arg == "--window") {
            // Show the pictures in a window, play the sound and read the keyboard.
            window_wanted = true;
        } else if (arg[0] != '-' && iso.empty()) {
            // The first argument that is not an option is the disc image.
            iso = arg;
        } else {
            // Anything else is a mistake: say what the options are.
            std::fprintf(
                stderr,
                "usage: openrac-boot DISC.iso [--hooks FILE] [--frames N] [--report N] [--verbose "
                "N] "
                "[--ntsc] [--window] [--ppm FILE] [--press FRAME:BUTTONS[:FRAMES]] [--gs-states "
                "FRAME] "
                "[--gs-threads N] [--one-thread] [--dump-vif FRAME FILE] [--card DIRECTORY | "
                "--no-card] [--wav FILE] [--native LIBRARY] [--native-check CALLS] [--native-range "
                "FIRST:LAST] [--native-skip FILE] [--native-calls FILE] [--write "
                "FRAME:ADDRESS:VALUE[:FRAMES]] [--vu-programs] [--capture LIST DIRECTORY]\n"
            );
            return 2;
        }
    }

    // No frame count given: a window runs until it is closed, a headless run for 600 fields.
    if (frames < 0) {
        frames = window_wanted ? INT_MAX : 600;
    }

    // No disc image was named, or it cannot be opened.
    if (iso.empty() || !machine.disc.open(iso)) {
        std::fprintf(stderr, "cannot open the disc image %s\n", iso.c_str());
        return 1;
    }

    // A negative thread count counts as none: the caller draws.
    machine.graphics.gs.set_threads(static_cast<unsigned>(std::max(gs_threads, 0)));

    // Without `--one-thread` the drawing path gets its own thread before the program starts.
    if (drawing_thread) {
        machine.drawing.start();
    }

    std::string error;

    // The disc has no bootable program, or the hooks table is bad.
    if (!machine.boot(&error) || (!hooks.empty() && !machine.load_hooks(hooks, &error))) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    // Host code in place of guest functions was asked for and cannot be loaded.
    if (!native.empty() && !machine.native.load(native, &error)) {
        std::fprintf(stderr, "%s: %s\n", native.c_str(), error.c_str());
        return 1;
    }

    std::vector<u8> capture_marks;

    // Call states were asked for: mark the functions' first instructions for the interpreter.
    if (!captures.empty()) {
        // Host code uses the same marks; the two do not go together.
        if (!native.empty()) {
            std::fprintf(stderr, "--capture and --native do not go together\n");
            return 2;
        }

        std::filesystem::create_directories(capture_dir);
        capture_marks.assign(GuestMemory::kRamBytes / 4, 0);

        for (const Capture& c : captures) {
            capture_marks[(c.address & (GuestMemory::kRamBytes - 1)) >> 2] = 1;
        }

        machine.ee.native_marks = capture_marks.data();

        // Reached a marked word: take the state if it is one of the listed functions' first calls.
        machine.ee.on_native = [&](u32 address) {
            for (Capture& c : captures) {
                bool here = (c.address & (GuestMemory::kRamBytes - 1)) == (address & (GuestMemory::kRamBytes - 1));

                if (!here || c.taken >= c.wanted) {
                    continue;
                }

                // Another program's code is at this address now (a level's).
                if (crc32(0, machine.memory.ram(c.address), c.bytes) != c.crc) {
                    continue;
                }

                sys::CallState state;
                state.take(machine.ee, machine.vu0, machine.memory, machine.vif0.micro.data(), machine.vif0.data.data());
                state.address = address;
                std::string path = capture_dir + "/" + c.name + "." + std::to_string(c.taken) + ".snap";

                // The file is counted only when it was written.
                if (state.save(path)) {
                    c.taken++;
                }
            }

            return false;
        };
    }

    // The memory card: a directory, by default one per disc among the user's
    // own files.
    if (card_wanted) {
        // No directory was named: use the one under the user's data folder.
        if (card.empty()) {
            std::filesystem::path home = std::getenv("HOME") ? std::getenv("HOME") : ".";
#if defined(__APPLE__)
            std::filesystem::path data = home / "Library" / "Application Support" / "OpenRAC";
#else
            std::filesystem::path data =
                std::getenv("XDG_DATA_HOME")
                    ? std::filesystem::path(std::getenv("XDG_DATA_HOME")) / "openrac"
                    : home / ".local" / "share" / "openrac";
#endif
            card = (data / "memcard" / machine.program_name).string();
        }

        std::error_code problem;
        std::filesystem::create_directories(card, problem);

        // The directory cannot be made: the program runs without a card.
        if (problem) {
            std::fprintf(
                stderr,
                "no memory card: cannot make %s (%s)\n",
                card.c_str(),
                problem.message().c_str()
            );
        } else {
            machine.card.directory = card;
            std::fprintf(stderr, "memory card: %s\n", card.c_str());
        }
    }

#ifndef OPENRAC_NO_WINDOW
    host::Window window;

    // A 960 by 720 window (4:3); the program ends if SDL cannot make it.
    if (window_wanted && !window.open("OpenRAC", 960, 720)) {
        return 1;
    }
#else
    // A build without SDL has no window; the flag is read only to say it is not used.
    (void)window_wanted;
#endif

    std::vector<s16> heard;

    // Called by the machine at each vertical blank, on this thread, with that field's sound.
    machine.on_sound = [&](const s16* samples, std::size_t count) {
        // Keep the samples (left and right, so count * 2) only if a sound file was asked for.
        if (!wav.empty()) {
            heard.insert(heard.end(), samples, samples + count * 2);
        }
#ifndef OPENRAC_NO_WINDOW
        // In a window the sound is played as well.
        if (window_wanted) {
            window.play(samples, count, sys::Sound::kRate);
        }
#endif
    };

    Image image;
    auto reported_at = std::chrono::steady_clock::now();
    auto next_frame_at = reported_at;

    /**
     * What one field needs from here: `before_field` sets the pad and any recording up,
     * `after_field` shows the picture and writes what was asked for, `finish` is the closing
     * report. The machine calls them at each vertical blank through `on_frame`, wherever the
     * program is at that moment: also inside host code that stands in for a guest function,
     * which cannot return here first.
     */
    int frame = 0;
    bool stop_run = false;

    /**
     * For working on the GS: what one frame is drawn with, by state, with
     * how many primitives and the area they span.
     *
     * The area and level of detail start at values no primitive can have, so the first one
     * replaces them.
     */
    struct Use {
        /**
         * Primitives drawn with the state, the box they span as least and greatest
         * coordinates, and the rank of the first among all primitives.
         */
        int count = 0, x0 = 1 << 30, y0 = 1 << 30, x1 = -(1 << 30), y1 = -(1 << 30), first = 0;

        /** The least and greatest level of detail among them. */
        float lod0 = 1e9f, lod1 = -1e9f;
    };

    std::map<std::string, Use> states;
    int order = 0;
    std::vector<u8> vif_bytes, vif_micro, vif_data;

    // Before a field runs: the window, the pad, and what a tool asked to record.
    auto before_field = [&] {
#ifndef OPENRAC_NO_WINDOW
        // The user closed the window (or pressed Escape).
        if (window_wanted && !window.pump()) {
            stop_run = true;
            return;
        }
#endif

        states.clear();
        order = 0;

        // This is the field whose states were asked for: collect them while it is drawn.
        if (frame == states_frame) {
            // The drawing side must be idle before a callback is installed on the GS.
            machine.drawing.sync();

            // Called by the GS for every primitive drawn, on the drawing thread: sums up per state.
            machine.graphics.gs.on_primitive = [&](const std::string& state,
                                                   int x0,
                                                   int y0,
                                                   int x1,
                                                   int y1,
                                                   float lod0,
                                                   float lod1) {
                Use& u = states[state];

                // The first primitive with this state fixes where the state ranks in the listing.
                if (u.count++ == 0) {
                    u.first = order;
                }

                order++;
                u.x0 = std::min(u.x0, x0);
                u.y0 = std::min(u.y0, y0);
                u.x1 = std::max(u.x1, x1);
                u.y1 = std::max(u.y1, y1);
                u.lod0 = std::min(u.lod0, lod0);
                u.lod1 = std::max(u.lod1, lod1);
            };
        }

        // The pad for this field: the scripted presses that cover it.
        machine.pad.buttons = 0;
        for (const Press& p : presses) {
            // The press covers fields from its start for its length.
            if (frame >= p.frame && frame < p.frame + p.length) {
                machine.pad.buttons |= static_cast<u16>(p.buttons);
            }
        }

        for (const Write& w : writes) {
            bool covered = frame >= w.frame && frame < w.frame + w.length;
            bool inside = w.address + 4 <= GuestMemory::kRamBytes;

            // The change covers this field and the word is in main memory.
            if (covered && inside) {
                std::memcpy(machine.memory.ram(w.address), &w.value, 4);
            }
        }

#ifndef OPENRAC_NO_WINDOW
        // In a window the keyboard and controller add to the scripted presses.
        if (window_wanted) {
            host::Controls c = window.controls();
            machine.pad.buttons |= static_cast<u16>(c.buttons);
            machine.pad.left_x = c.left_x;
            machine.pad.left_y = c.left_y;
            machine.pad.right_x = c.right_x;
            machine.pad.right_y = c.right_y;
        }
#endif
        vif_bytes.clear();
        vif_micro.clear();
        vif_data.clear();

        // This is the field to record: keep VU1's memories from before it, then its VIF1 bytes.
        if (frame == vif_frame) {
            // The memories are read directly, so the drawing side must be idle.
            machine.drawing.sync();
            vif_micro.assign(machine.graphics.vif.micro.begin(), machine.graphics.vif.micro.end());
            vif_data.assign(machine.graphics.vif.data.begin(), machine.graphics.vif.data.end());
            machine.drawing.vif_copy = &vif_bytes;
        }
    };

    // After a field has run: recordings, listings, the picture, the pace, the periodic report.
    auto after_field = [&] {
        // Write the recording: the magic, the two memories before the field, then the VIF1 bytes.
        if (frame == vif_frame) {
            machine.drawing.vif_copy = nullptr;

            // If the file cannot be made nothing is written and no message is given.
            if (std::FILE* f = std::fopen(vif_file.c_str(), "wb")) {
                // The magic is "ORVIF1" and two zero bytes: 8 bytes.
                std::fwrite("ORVIF1\0", 1, 8, f);
                std::fwrite(vif_micro.data(), 1, vif_micro.size(), f);
                std::fwrite(vif_data.data(), 1, vif_data.size(), f);
                std::fwrite(vif_bytes.data(), 1, vif_bytes.size(), f);
                std::fclose(f);
                std::fprintf(
                    stderr,
                    "frame %d: %zu bytes for VIF1 written to %s\n",
                    frame,
                    vif_bytes.size(),
                    vif_file.c_str()
                );
            }
        }

        // A VU0 microprogram did not stop: further fields would show nothing useful.
        if (machine.ee.vu0_runaways) {
            stop_run = true;
            return;
        }

        // The states of this field are complete: print them in the order they first appeared.
        if (frame == states_frame) {
            machine.drawing.sync();
            machine.graphics.gs.on_primitive = nullptr;
            std::vector<std::pair<int, std::string>> lines;

            // One line per state: first rank, count, the area, then the state's own text.
            for (const auto& [state, u] : states) {
                // Room for the numbers: fixed-width columns of at most 96 characters.
                char head[96];
                std::snprintf(
                    head,
                    sizeof(head),
                    "%6d x%-6d [%4d,%4d - %4d,%4d] ",
                    u.first,
                    u.count,
                    u.x0,
                    u.y0,
                    u.x1,
                    u.y1
                );
                char tail[48] = "";

                // Show the level of detail only if some primitive set it and it was not all zero.
                if (u.lod1 >= u.lod0 && (u.lod0 != 0 || u.lod1 != 0)) {
                    std::snprintf(
                        tail,
                        sizeof(tail),
                        " | lod %.2f..%.2f",
                        static_cast<double>(u.lod0),
                        static_cast<double>(u.lod1)
                    );
                }
                lines.push_back({u.first, head + state + tail});
            }

            std::sort(lines.begin(), lines.end());
            for (const auto& line : lines) {
                std::printf("%s\n", line.second.c_str());
            }
        }

        bool shown = machine.drawing.picture(image);

#ifndef OPENRAC_NO_WINDOW
        // The picture is shown at the games' 4:3 shape, if the display was on.
        if (window_wanted && shown) {
            window.present(image, 4.0f / 3.0f);
        }

        if (window_wanted) {
            // No faster than the console: one frame per field.
            next_frame_at += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(1.0 / machine.hz)
            );
            auto now = std::chrono::steady_clock::now();

            // Ahead of the schedule: wait. Behind it: start the next schedule from now.
            if (next_frame_at > now) {
                std::this_thread::sleep_until(next_frame_at);
            } else {
                next_frame_at = now;
            }
        }
#else
        // A build without SDL does not show the picture.
        (void)shown;
#endif

        // Every `report` fields print counts and the speed.
        if (report > 0 && (frame + 1) % report == 0) {
            // The counts come from the drawing side, so it must have caught up.
            machine.drawing.sync();
            auto now = std::chrono::steady_clock::now();
            double seconds = std::chrono::duration<double>(now - reported_at).count();
            reported_at = now;

            // The speed is `report` fields over the time taken; the floor avoids dividing by zero.
            std::fprintf(
                stderr,
                "frame %d: pc %08x ra %08x, %llu primitives, %llu pixels, %llu VU1 instructions, "
                "%.1f frames a second\n",
                frame + 1,
                machine.ee.pc,
                static_cast<u32>(machine.ee.gpr[31].lo),
                static_cast<unsigned long long>(machine.graphics.gs.stats.primitives),
                static_cast<unsigned long long>(machine.graphics.gs.stats.pixels),
                static_cast<unsigned long long>(machine.graphics.vu1_instructions),
                report / std::max(seconds, 1e-9)
            );
        }
    };

    // When the run is over: the reports and the files asked for. Returns the exit code.
    auto finish = [&]() -> int {
        // The closing report: the drawing side must finish first so its counts are complete.
        machine.drawing.sync();
        std::fprintf(
            stderr,
            "gs: %llu texture levels decoded (%llu texels), %llu transfers, %llu batches drawn\n",
            static_cast<unsigned long long>(machine.graphics.gs.stats.texture_decodes),
            static_cast<unsigned long long>(machine.graphics.gs.stats.texels_decoded),
            static_cast<unsigned long long>(machine.graphics.gs.stats.transfers),
            static_cast<unsigned long long>(machine.graphics.gs.stats.flushes)
        );
        std::fprintf(
            stderr,
            "stopped after %llu frames at pc %08x (ra %08x)\n",
            static_cast<unsigned long long>(machine.frames),
            machine.ee.pc,
            static_cast<u32>(machine.ee.gpr[31].lo)
        );

        // Everything the model met and does not do, once each with how often.
        for (const auto& [what, count] : machine.notes) {
            std::fprintf(
                stderr,
                "  not modelled: %s (%llu times)\n",
                what.c_str(),
                static_cast<unsigned long long>(count)
            );
        }

        // The EE met instructions it does not know: say how many, and the last one.
        if (machine.ee.unknown) {
            std::fprintf(
                stderr,
                "  %llu unknown EE instructions, the last %08x at %08x\n",
                static_cast<unsigned long long>(machine.ee.unknown),
                machine.ee.last_unknown,
                machine.ee.last_unknown_pc
            );
        }

        // Either vector unit or VIF1 met something it cannot decode.
        if (machine.vu0.unknown_ops || machine.graphics.vu1.unknown_ops
            || machine.graphics.vif.unknown_codes) {
            std::fprintf(
                stderr,
                "  unknown: %llu VU0, %llu VU1 instructions, %llu VIF1 codes\n",
                static_cast<unsigned long long>(machine.vu0.unknown_ops),
                static_cast<unsigned long long>(machine.graphics.vu1.unknown_ops),
                static_cast<unsigned long long>(machine.graphics.vif.unknown_codes)
            );
        }

        // A VU0 microprogram did not stop: say where it started and who started it.
        if (machine.ee.vu0_runaways) {
            std::fprintf(
                stderr,
                "  %llu VU0 microprograms did not stop (the first started at %u by the EE at "
                "%08x)\n",
                static_cast<unsigned long long>(machine.ee.vu0_runaways),
                machine.ee.vu0_runaway_start,
                machine.ee.vu0_runaway_from
            );
            u32 at = machine.ee.vu0_runaway_start;

            // The listing is long, so it is printed only when asked for in the environment.
            if (std::getenv("OPENRAC_VU0_LISTING")) {
                // The program as it sits in VU0, and where it was: 4 KB of memory is 512 pairs.
                for (u32 n = 0; n < 512; n++) {
                    u32 up = load<u32>(&machine.vif0.micro[n * 8 + 4]),
                        low = load<u32>(&machine.vif0.micro[n * 8]);

                    // Empty slots are not listed.
                    if (up || low) {
                        std::fprintf(stderr, "%s\n", vudis::pair(n, up, low).c_str());
                    }
                }

                // The program counter and the sixteen integer registers.
                std::fprintf(stderr, "pc %u; vi:", machine.vu0.pc);
                for (unsigned n = 0; n < 16; n++) {
                    std::fprintf(stderr, " %04x", machine.vu0.vi[n]);
                }
                std::fprintf(stderr, "\n");
            }

            // `at` is kept for working in a debugger; reading it here quiets the unused warning.
            (void)at;
        }

        // VU1 programs that were cut off at the instruction limit.
        if (machine.graphics.vu1_runaways) {
            std::fprintf(
                stderr,
                "  %llu of %llu VU1 program starts did not stop (the last from %u, at %u when cut "
                "off)\n",
                static_cast<unsigned long long>(machine.graphics.vu1_runaways),
                static_cast<unsigned long long>(machine.graphics.vu1_starts),
                machine.graphics.vu1_runaway_start,
                machine.graphics.vu1_runaway_pc
            );
        }

        // Asked for: how much each of the game's vector unit programs ran, by unit.
        if (vu_programs) {
            report_vu_programs("VU1", machine.vu_chunks, machine.graphics.vu1.uses());
            report_vu_programs("VU0", machine.vu_chunks, machine.vu0.uses());
        }

        // What host code ran, the 20 busiest functions first.
        if (!native.empty()) {
            machine.native.report(stderr, 20);
        }

        // The counted calls were asked for.
        if (!native_calls.empty()) {
            std::FILE* out = std::fopen(native_calls.c_str(), "w");

            // If the file cannot be made nothing is written and no message is given.
            if (out) {
                machine.native.write_calls(out);
                std::fclose(out);
            }
        }

        // A sound file was asked for.
        if (!wav.empty()) {
            // If the file cannot be made nothing is written and no message is given.
            if (std::FILE* f = std::fopen(wav.c_str(), "wb")) {
                // A plain sound file: 16-bit, two channels.
                u32 bytes = static_cast<u32>(heard.size() * 2), rate = sys::Sound::kRate;
                u8 head[44] = {'R', 'I', 'F', 'F', 0,   0,   0,   0, 'W', 'A', 'V',
                               'E', 'f', 'm', 't', ' ', 16,  0,   0, 0,   1,   0,
                               2,   0,   0,   0,   0,   0,   0,   0, 0,   0,   4,
                               0,   16,  0,   'd', 'a', 't', 'a', 0, 0,   0,   0};

                /*
                 * RIFF size at byte 4 (the data plus the 36 bytes after it), sample rate at 24,
                 * bytes a second at 28 (rate times 4 bytes a frame), data size at 40 (documented).
                 */
                store<u32>(head + 4, bytes + 36);
                store<u32>(head + 24, rate);
                store<u32>(head + 28, rate * 4);
                store<u32>(head + 40, bytes);
                std::fwrite(head, 1, sizeof(head), f);
                std::fwrite(heard.data(), 2, heard.size(), f);
                std::fclose(f);
                std::fprintf(
                    stderr,
                    "%.1f seconds of sound written to %s\n",
                    static_cast<double>(heard.size()) / 2 / rate,
                    wav.c_str()
                );
            }
        }

        // The last picture was asked for and the display was on.
        if (!ppm.empty() && machine.drawing.picture(image)) {
            write_ppm(ppm, image);
        }

        return 0;
    };

    /*
     * Called by the machine when a field is complete. The run ends from here, not from the
     * loop below: at that moment the program may be inside host code many calls deep.
     */
    machine.on_frame = [&] {
        after_field();
        frame++;

        // The frame limit, a halt, a closed window or a runaway VU0 program.
        if (stop_run || frame >= frames || machine.halted) {
            std::exit(finish());
        }

        before_field();

        // The window was closed.
        if (stop_run) {
            std::exit(finish());
        }
    };

    before_field();

    // Runs until `on_frame` ends the process; it comes back here only when the program halts.
    while (!stop_run && !machine.halted && frames > 0) {
        machine.run_frame();
    }

    return finish();
}
