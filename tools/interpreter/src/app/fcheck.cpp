// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-fcheck: does a function written in C do what the retail function does?
 *
 *   openrac-fcheck --code FILE@ADDRESS [--code FILE@ADDRESS ...] --entry ADDRESS
 *                  [--returns void|int|long|float] STATE...
 *
 * Each STATE is the machine at the start of a real call of the retail function, written by
 * `openrac-boot --capture`. From it the retail function runs in the interpreter, then, from the
 * same state again, the candidate: the code files (the candidate compiled for the console and
 * placed by tools/fcheck.py in the decompilation) are put into memory and the call starts at
 * `--entry`. What the two leave is compared: main memory and the scratchpad (but for the dead
 * stack below the caller's stack pointer and the candidate's own code), the result, the registers
 * a function must keep, and the order of what they did outside memory (devices, system calls).
 *
 * Exit status 0 when every state gives the same, 1 when one differs, 2 on a usage error.
 * It is a tool for writing functions, not part of the game.
 */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "sys/machine.h"
#include "sys/snapshot.h"

using namespace ps2;

namespace {

/** The most instructions a call may take before it counts as not returning. */
constexpr u64 kLimit = 200'000'000;

/** A piece of candidate code and where it goes. */
struct Code {
    u32 address;
    std::vector<u8> bytes;
};

/** One thing a call did outside memory, in order. */
struct Outside {
    char what;  // 'r' read, 'w' write, 'q' quadword write, 's' system call
    u32 address;
    u64 value, value2;

    bool operator==(const Outside& o) const {
        return what == o.what && address == o.address && value == o.value && value2 == o.value2;
    }
};

/** What a call left. */
struct Result {
    bool returned = false;
    u64 instructions = 0;
    std::array<Ee::Reg, 32> gpr{};
    std::array<u32, 32> fpr{};
    std::vector<u8> ram, scratchpad;
    std::vector<Outside> outside;
};

const char* const kGpr[32] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2",
                              "t3",   "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5",
                              "s6",   "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

}  // namespace

/** Two words that hold floats of the same sign and differ by at most `ulps` in the last place. */
bool near_float(u32 a, u32 b, int ulps) {
    if (ulps <= 0 || ((a ^ b) & 0x80000000u)) {
        return false;
    }
    u32 ea = (a >> 23) & 0xFF, eb = (b >> 23) & 0xFF;
    if (ea == 0 || eb == 0) {
        return false;
    }
    long long d = static_cast<long long>(a & 0x7FFFFFFF) - static_cast<long long>(b & 0x7FFFFFFF);
    return d <= ulps && d >= -ulps;
}

int main(int argc, char** argv) {
    std::vector<Code> code;
    std::vector<std::string> states;
    u32 entry = 0;
    std::string returns = "void";
    bool have_entry = false;
    int ulps = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--code" && i + 1 < argc) {
            std::string spec = argv[++i];
            std::size_t at = spec.rfind('@');

            if (at == std::string::npos) {
                std::fprintf(stderr, "--code wants FILE@ADDRESS\n");
                return 2;
            }

            std::ifstream f(spec.substr(0, at), std::ios::binary);
            Code c{static_cast<u32>(std::stoul(spec.substr(at + 1), nullptr, 16)),
                   std::vector<u8>(std::istreambuf_iterator<char>(f), {})};

            if (!f.good() && !f.eof()) {
                std::fprintf(stderr, "cannot read %s\n", spec.c_str());
                return 2;
            }

            code.push_back(std::move(c));
        } else if (arg == "--entry" && i + 1 < argc) {
            entry = static_cast<u32>(std::stoul(argv[++i], nullptr, 16));
            have_entry = true;
        } else if (arg == "--ulps" && i + 1 < argc) {
            // Floats that differ by this many units in the last place count as the same (VU0 and
            // the FPU round differently; a hand-written VU0 routine in C cannot always agree).
            ulps = std::stoi(argv[++i]);
        } else if (arg == "--returns" && i + 1 < argc) {
            returns = argv[++i];
        } else if (!arg.empty() && arg[0] != '-') {
            states.push_back(arg);
        } else {
            std::fprintf(stderr,
                         "usage: openrac-fcheck --code FILE@ADDRESS [...] --entry ADDRESS "
                         "[--returns void|int|long|float] [--ulps N] STATE...\n");
            return 2;
        }
    }

    if (!have_entry || code.empty() || states.empty()) {
        std::fprintf(stderr, "openrac-fcheck needs --code, --entry and at least one state\n");
        return 2;
    }

    auto machine = std::make_unique<sys::Machine>();
    Ee& ee = machine->ee;
    std::vector<Outside>* log = nullptr;

    // Everything that leaves memory is written down, then done as the machine does it.
    auto syscall = ee.on_syscall;
    auto read = ee.on_read;
    auto write = ee.on_write;
    auto write128 = ee.on_write128;

    ee.on_syscall = [&](u32 c) {
        if (log) {
            log->push_back({'s', c, ee.gpr[3].lo, ee.gpr[4].lo});
        }
        if (syscall) {
            syscall(c);
        }
    };
    ee.on_read = [&](u32 a, unsigned n) -> u64 {
        if (log) {
            log->push_back({'r', a, n, 0});
        }
        return read ? read(a, n) : 0;
    };
    ee.on_write = [&](u32 a, u64 v, unsigned n) {
        if (log) {
            log->push_back({'w', a, v, n});
        }
        if (write) {
            write(a, v, n);
        }
    };
    ee.on_write128 = [&](u32 a, u64 lo, u64 hi) {
        if (log) {
            log->push_back({'q', a, lo, hi});
        }
        if (write128) {
            write128(a, lo, hi);
        }
    };

    u8* micro = machine->vif0.micro.data();
    u8* data = machine->vif0.data.data();

    // Runs one call from a state, the retail function or the candidate.
    auto run = [&](const sys::CallState& state, u32 start, bool candidate) {
        Result r;
        state.put(ee, machine->vu0, machine->memory, micro, data);

        if (candidate) {
            for (const Code& c : code) {
                std::memcpy(machine->memory.ram(c.address), c.bytes.data(), c.bytes.size());
            }
        }

        ee.event_at = ~u64{0};
        ee.pc = start;
        ee.next_pc = start + 4;
        log = &r.outside;

        Ee::Call call = ee.begin_call(start);
        while (!r.returned && r.instructions < kLimit) {
            r.returned = ee.run_call(1'000'000);
            r.instructions += 1'000'000;
        }
        ee.end_call(call);

        log = nullptr;
        r.gpr = ee.gpr;
        r.fpr = ee.fpr;
        r.ram.assign(machine->memory.ram(0), machine->memory.ram(0) + GuestMemory::kRamBytes);
        r.scratchpad.assign(machine->memory.scratchpad(0),
                            machine->memory.scratchpad(0) + GuestMemory::kScratchpadBytes);
        return r;
    };

    int differing = 0;

    for (const std::string& path : states) {
        sys::CallState state;
        std::string error;

        if (!state.load(path, &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 2;
        }

        Result retail = run(state, state.address, false);
        Result ours = run(state, entry, true);
        std::vector<std::string> notes;
        char text[256];

        if (!retail.returned) {
            std::printf("%s: SKIPPED (the retail function did not return within %llu instructions)\n",
                        path.c_str(), static_cast<unsigned long long>(kLimit));
            continue;
        }

        if (!ours.returned) {
            notes.push_back("the candidate did not return (an endless loop, or it jumped away)");
        }

        // The result.
        if ((returns == "int" && static_cast<u32>(ours.gpr[2].lo) != static_cast<u32>(retail.gpr[2].lo))
            || (returns == "long" && ours.gpr[2].lo != retail.gpr[2].lo)) {
            std::snprintf(text, sizeof(text), "returns %llx, retail %llx",
                          static_cast<unsigned long long>(ours.gpr[2].lo),
                          static_cast<unsigned long long>(retail.gpr[2].lo));
            notes.push_back(text);
        }
        if (returns == "float" && ours.fpr[0] != retail.fpr[0] && !near_float(ours.fpr[0], retail.fpr[0], ulps)) {
            std::snprintf(text, sizeof(text), "returns float bits %08x, retail %08x", ours.fpr[0],
                          retail.fpr[0]);
            notes.push_back(text);
        }

        // The registers a function keeps for its caller.
        for (int g : {16, 17, 18, 19, 20, 21, 22, 23, 28, 29, 30}) {
            if (ours.gpr[g].lo != state.gpr[g].lo) {
                std::snprintf(text, sizeof(text), "does not keep $%s (%llx, was %llx)", kGpr[g],
                              static_cast<unsigned long long>(ours.gpr[g].lo),
                              static_cast<unsigned long long>(state.gpr[g].lo));
                notes.push_back(text);
            }
        }
        for (int f = 20; f < 32; f++) {
            if (ours.fpr[f] != state.fpr[f]) {
                std::snprintf(text, sizeof(text), "does not keep $f%d", f);
                notes.push_back(text);
            }
        }

        // Memory, but for the dead stack and the candidate's own code.
        u32 sp = static_cast<u32>(state.gpr[29].lo) & (GuestMemory::kRamBytes - 1);
        auto skipped = [&](u32 at) {
            if (at < sp && sp - at <= 0x100000) {
                return true;
            }
            for (const Code& c : code) {
                u32 base = c.address & (GuestMemory::kRamBytes - 1);
                if (at + 4 > base && at < base + c.bytes.size()) {
                    return true;
                }
            }
            return false;
        };
        int shown = 0, words = 0;
        for (u32 at = 0; at < GuestMemory::kRamBytes; at += 4) {
            if (std::memcmp(ours.ram.data() + at, retail.ram.data() + at, 4) == 0 || skipped(at)) {
                continue;
            }
            u32 wa, wb;
            std::memcpy(&wa, ours.ram.data() + at, 4);
            std::memcpy(&wb, retail.ram.data() + at, 4);
            if (near_float(wa, wb, ulps)) {
                continue;
            }
            words++;
            if (shown < 16) {
                u32 a, b, was;
                std::memcpy(&a, ours.ram.data() + at, 4);
                std::memcpy(&b, retail.ram.data() + at, 4);
                std::memcpy(&was, state.ram.data() + at, 4);
                std::snprintf(text, sizeof(text), "word at %08x: ours %08x, retail %08x (before %08x)",
                              at, a, b, was);
                notes.push_back(text);
                shown++;
            }
        }
        if (words > shown) {
            std::snprintf(text, sizeof(text), "... %d words of memory differ in all", words);
            notes.push_back(text);
        }
        if (ours.scratchpad != retail.scratchpad) {
            for (u32 at = 0; at < GuestMemory::kScratchpadBytes; at += 4) {
                if (std::memcmp(ours.scratchpad.data() + at, retail.scratchpad.data() + at, 4) != 0) {
                    std::snprintf(text, sizeof(text), "scratchpad differs, first at offset %04x", at);
                    notes.push_back(text);
                    break;
                }
            }
        }

        /*
         * What they did outside memory, in order. An address in the dead stack (a packet built in
         * a local and handed to the DMA controller) is the compiler's choice of frame: it compares
         * as "a stack address", whatever its offset.
         */
        auto stack_value = [&](u64 v) -> u64 {
            u64 seg = v & ~u64{0x01FFFFFF};
            u32 a = static_cast<u32>(v & 0x01FFFFFF);
            bool mirror = seg == 0 || seg == 0x20000000 || seg == 0x30000000 || seg == 0x80000000;
            return mirror && a < sp && sp - a <= 0x100000 ? 0x57AC0000'00000000ull : v;
        };
        for (std::vector<Outside>* v : {&ours.outside, &retail.outside}) {
            for (Outside& o : *v) {
                if (o.what == 'w' || o.what == 'q') {
                    o.value = stack_value(o.value);
                }
                if (o.what == 'q') {
                    o.value2 = stack_value(o.value2);
                }
            }
        }
        if (ours.outside != retail.outside) {
            std::size_t k = 0;
            while (k < ours.outside.size() && k < retail.outside.size() && ours.outside[k] == retail.outside[k]) {
                k++;
            }
            auto describe = [](const std::vector<Outside>& v, std::size_t k) {
                char t[96];
                if (k >= v.size()) {
                    return std::string("nothing more");
                }
                const Outside& o = v[k];
                std::snprintf(t, sizeof(t), "%c %08x %llx %llx", o.what, o.address,
                              static_cast<unsigned long long>(o.value),
                              static_cast<unsigned long long>(o.value2));
                return std::string(t);
            };
            std::snprintf(text, sizeof(text),
                          "outside memory, step %zu: ours [%s], retail [%s] (r read, w write, s system call)",
                          k, describe(ours.outside, k).c_str(), describe(retail.outside, k).c_str());
            notes.push_back(text);
        }

        if (notes.empty()) {
            std::printf("%s: SAME (retail %llu, ours %llu million instructions or less)\n",
                        path.c_str(), static_cast<unsigned long long>(retail.instructions / 1'000'000),
                        static_cast<unsigned long long>(ours.instructions / 1'000'000));
        } else {
            differing++;
            std::printf("%s: DIFFERS\n", path.c_str());
            for (const std::string& n : notes) {
                std::printf("    %s\n", n.c_str());
            }
        }
    }

    return differing ? 1 : 0;
}
