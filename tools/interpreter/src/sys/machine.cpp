// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The machine: booting a program, time and interrupts, the DMA controller, the hardware
 * registers and the kernel's calls.
 *
 * Time is counted in EE cycles. The EE runs until `event_at`; `event` then raises the vertical
 * blank, delivers interrupts and calls back the program, and sets the next stop. The kernel is not
 * run: a SYSCALL lands in `syscall`, which answers the kernel call or runs a replaced library
 * function. Calls it does not know are counted with `note`.
 *
 * Sources: the EE's memory map, DMA controller, timers and interrupt controller as publicly
 * documented, and the kernel calls the games' own code makes.
 */

#include "machine.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

#include "ps2/fp_quad.h"

namespace sys {

using namespace ps2;

namespace {

/**
 * The field of SYSCALL that marks one put at a replaced function's entry.
 *
 * The code field is 20 bits wide (bits 6-25 of the instruction); this is its top bit, which the
 * program's own SYSCALLs do not use (assumed).
 */
constexpr u32 kHookCode = 0x80000;

/**
 * The address of each DMA channel's registers, by channel number: VIF0, VIF1, GIF, fromIPU,
 * toIPU, SIF0, SIF1, SIF2, fromSPR, toSPR (documented).
 */
constexpr u32 kDmaBase[10] =
    {0x10008000,
     0x10009000,
     0x1000A000,
     0x1000B000,
     0x1000B400,
     0x1000C000,
     0x1000C400,
     0x1000C800,
     0x1000D000,
     0x1000D400};

/** The numbers of the DMA channels this model carries out transfers for (documented). */
enum : unsigned {
    kVif0 = 0,
    kVif1 = 1,
    kGifChannel = 2,
    kFromSpr = 8,
    kToSpr = 9
};

}  // namespace

Machine::Machine() {
    // The EE hands everything outside memory to the machine: hardware registers, SYSCALLs, events.
    // A device register is outside the guest's memory: host code's checker must know.
    ee.on_read = [this](u32 address, unsigned bytes) {
        native.outside++;
        return hw_read(address, bytes);
    };
    ee.on_write = [this](u32 address, u64 value, unsigned bytes) {
        native.outside++;
        hw_write(address, value, bytes);
    };
    ee.on_write128 = [this](u32 address, u64 lo, u64 hi) {
        native.outside++;
        hw_write128(address, lo, hi);
    };
    ee.on_syscall = [this](u32 code) {
        syscall(code);
    };
    ee.on_event = [this] {
        event();
    };
    vif0.on_program = [this] {
        vu0.program_changed();
    };

    // Until the program reads VU0's status flags (these games never do), skip flags nothing reads.
    vu0.skip_unread_flags = true;

    add_default_services();
}

void Machine::log(int level, const char* format, ...) {
    // The message is more detailed than the verbosity asked for.
    if (level > verbose) {
        return;
    }

    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    // The format carries no newline; every message is one line.
    std::fputc('\n', stderr);
}

void Machine::note(const std::string& what) {
    // Only the first time is reported; later ones are counted for the totals a tool prints.
    if (notes[what]++ == 0) {
        log(1, "not modelled: %s (pc %08x)", what.c_str(), ee.pc - 4);
    }
}

std::string Machine::string_at(u32 address, std::size_t limit) {
    std::string out;

    // Read up to the limit, so a string that never ends cannot run on through memory.
    while (out.size() < limit) {
        char c = static_cast<char>(ee.read8(address++));

        // A zero byte ends the string.
        if (!c) {
            break;
        }

        out.push_back(c);
    }

    return out;
}

std::string Machine::format(u32 format_address, unsigned first_arg) {
    std::string f = string_at(format_address), out;
    unsigned next = first_arg;

    // Called for each conversion: the next argument register, or 0 once all eight are used.
    auto take = [&]() -> u64 {
        return next < 8 ? ee.gpr[4 + next++].lo : 0;
    };

    // Copy the format character by character; a '%' starts a conversion.
    for (std::size_t n = 0; n < f.size(); n++) {
        // Text outside a conversion is copied as it is.
        if (f[n] != '%') {
            out.push_back(f[n]);
            continue;
        }

        std::string spec = "%";
        n++;

        // Collect flags, width and precision; 'l' is dropped because arguments are 32 bits here.
        while (n < f.size() && std::strchr("-+ #0123456789.l", f[n])) {
            if (f[n] != 'l') {
                spec.push_back(f[n]);
            }
            n++;
        }

        // The format ends inside a conversion: stop.
        if (n >= f.size()) {
            break;
        }

        // Room for one converted number.
        char buffer[128];

        switch (f[n]) {
            case '%':
                out.push_back('%');
                break;

            case 's':
                // The argument is the guest address of a string.
                out += string_at(static_cast<u32>(take()));
                break;

            case 'c':
                out.push_back(static_cast<char>(take()));
                break;

            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'p':
                // A pointer prints as hexadecimal; every integer conversion gets 32 bits.
                spec.push_back(f[n] == 'p' ? 'x' : f[n]);
                std::snprintf(buffer, sizeof(buffer), spec.c_str(), static_cast<unsigned>(take()));
                out += buffer;
                break;

            case 'f':
            case 'g':
            case 'e':
                // Floats are passed in FPU registers, which this does not read: take the slot only.
                take();
                out += "<float>";
                break;

            default:
                // A conversion this does not know is copied as written.
                out += spec;
                out.push_back(f[n]);
                break;
        }
    }

    return out;
}

// --- replaced functions ---

bool Machine::hook(u32 address, const std::string& service) {
    auto found = services_.find(service);

    // No service has that name, so there is nothing to put at the address.
    if (found == services_.end()) {
        return false;
    }

    u32 index = static_cast<u32>(hooked_.size());
    hooked_.push_back(found->second);
    hooked_names_.push_back(service);

    // Keep the first instruction only the first time, so a second hook does not save a SYSCALL.
    if (!hooked_original_.count(address)) {
        hooked_original_[address] = ee.read32(address);
    }

    /*
     * SYSCALL with the hook mark and the service's number in its code field.
     * The code field is bits 6-25 of the instruction and the function code of SYSCALL is 0x0C
     * (documented).
     */
    ee.write32(address, ((kHookCode | index) << 6) | 0x0C);
    return true;
}

bool Machine::load_hooks(const std::string& path, std::string* error) {
    std::ifstream in(path);

    // The table cannot be read.
    if (!in) {
        if (error) {
            *error = "cannot read " + path;
        }
        return false;
    }

    std::string line;
    int number = 0;

    // One line at a time; the loop ends at the end of the file or at the first bad line.
    while (std::getline(in, line)) {
        number++;

        // A '#' starts a comment that runs to the end of the line.
        std::size_t hash = line.find('#');
        if (hash != std::string::npos) {
            line.erase(hash);
        }

        std::istringstream words(line);
        std::string address, service;

        // A blank line or a comment alone has no address and name; skip it.
        if (!(words >> address >> service)) {
            continue;
        }

        // The address is hexadecimal. A name no service has stops the load.
        if (!hook(static_cast<u32>(std::stoul(address, nullptr, 16)), service)) {
            if (error) {
                *error = path + ":" + std::to_string(number) + ": no service named " + service;
            }
            return false;
        }
    }

    return true;
}

void Machine::syscall(u32 code) {
    // A kernel call or a replaced library function: something outside the guest's memory.
    native.outside++;

    // The mark of a replaced function: the rest of the code is the service's number.
    if (code & kHookCode) {
        u32 index = code & (kHookCode - 1);

        // A number outside the table is ignored.
        if (index < hooked_.size()) {
            log(3,
                "%s(%x, %x, %x, %x) from %08x",
                hooked_names_[index].c_str(),
                arg(0),
                arg(1),
                arg(2),
                arg(3),
                static_cast<u32>(ee.gpr[31].lo));
            u32 entry = ee.pc - 4;
            hooked_[index](*this);

            // Unless the service sent the program somewhere itself, return to the caller.
            if (ee.pc == entry + 4) {
                ee.leave();
            }
        }
        return;
    }

    // The kernel call's number is in v1, negative for the forms meant for
    // interrupt handlers.
    int number = static_cast<s32>(ee.gpr[3].lo);
    kernel(number < 0 ? -number : number);
}

// --- the kernel ---

void Machine::kernel(int number) {
    u32 a0 = arg(0), a1 = arg(1), a2 = arg(2), a3 = arg(3);
    log(4, "syscall %02x(%x, %x, %x, %x) at %08x", number, a0, a1, a2, a3, ee.pc - 4);

    /*
     * The call numbers and names follow the kernel's public call table (documented). Only the
     * calls the games make are answered; any other is counted by `note` and returns 0.
     */
    switch (number) {
        case 0x02:  // SetGsCrt(interlace, mode, field)
            // Only logged: the field rate comes from `hz`.
            log(1, "SetGsCrt: interlace %u, mode %u, field %u", a0, a1, a2);
            break;

        case 0x04:  // Exit
            // The program has ended: say so and stop the run.
            log(0, "the program called Exit(%d)", static_cast<s32>(a0));
            halted = true;
            ee.stop();
            break;

        case 0x10:    // AddIntcHandler(cause, handler, next, arg)
        case 0x12: {  // AddDmacHandler(channel, handler, next, arg)
            // The INTC cause or the DMA channel picks the list; both are numbers below 16.
            auto& list = (number == 0x10 ? intc_handlers : dmac_handlers)[a0 & 15];
            Handler h{a1, a3, next_id_++, static_cast<u32>(ee.gpr[28].lo)};

            if (a2 == 0) {
                // "next 0" puts it at the head.
                list.insert(list.begin(), h);
            } else {
                list.push_back(h);
            }

            // The handler's id is what RemoveIntcHandler and RemoveDmacHandler take later.
            result(static_cast<u64>(h.id));
            break;
        }

        case 0x11:    // RemoveIntcHandler(cause, id)
        case 0x13: {  // RemoveDmacHandler(channel, id)
            auto& list = (number == 0x11 ? intc_handlers : dmac_handlers)[a0 & 15];
            std::erase_if(list, [&](const Handler& h) { return h.id == static_cast<int>(a1); });

            // The result is how many handlers are left on the list.
            result(list.size());
            break;
        }

        case 0x14:
        case 0x1A:  // _EnableIntc(cause)
            // The result is 1 when the cause was off, that is when the call changed the mask.
            result((intc_mask >> a0) & 1 ? 0 : 1);
            intc_mask |= 1u << (a0 & 15);
            break;

        case 0x15:
        case 0x1B:  // _DisableIntc(cause)
            // The result is 1 when the cause was on.
            result((intc_mask >> a0) & 1);
            intc_mask &= ~(1u << (a0 & 15));
            break;

        case 0x16:
        case 0x1C:  // _EnableDmac(channel)
            // The channels' enable bits are D_STAT bits 16-25 (documented).
            result((d_stat >> (16 + a0)) & 1 ? 0 : 1);
            d_stat |= 1u << (16 + (a0 & 15));

            // And the controller's own line to the CPU: INTC cause 1 is the DMAC.
            intc_mask |= 2;
            break;

        case 0x17:
        case 0x1D:  // _DisableDmac(channel)
            result((d_stat >> (16 + a0)) & 1);
            d_stat &= ~(1u << (16 + (a0 & 15)));
            break;

        case 0x20:  // CreateThread
            // Threads are not run; the program is told one exists.
            note("CreateThread (threads are not run)");
            result(static_cast<u64>(next_id_++));
            break;

        // Threads are not run, so these thread calls report success and do nothing.
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:  // Delete, Start, Exit, ExitDelete, Terminate
        case 0x29:
        case 0x2A:  // ChangeThreadPriority
        case 0x2B:
        case 0x2C:  // RotateThreadReadyQueue
        case 0x32:
        case 0x33:
        case 0x34:  // Sleep, Wakeup
            result(0);
            break;

        case 0x2F:  // GetThreadId
            // There is one thread, numbered 1.
            result(1);
            break;

        case 0x3C: {  // SetupThread(gp, stack, stack_size, args, root): the initial stack pointer
            // A stack address of -1 puts the stack at the top of memory (assumed).
            u32 stack = a1 == 0xFFFFFFFFu ? GuestMemory::kRamBytes - a2 : a1;
            stack_top_ = stack + a2;
            result(stack_top_);
            break;
        }

        case 0x3D:  // SetupHeap(start, size): the end of the heap
            // A size of -1 means up to the stack, less a margin of 0x4000 bytes (assumed).
            result(a1 == 0xFFFFFFFFu ? stack_top_ - 0x4000 : a0 + a1);
            break;

        case 0x3E:  // EndOfHeap
            // The same margin below the stack as SetupHeap leaves.
            result(stack_top_ - 0x4000);
            break;

        case 0x4B:  // GetOsdConfigParam(config)
            /*
             * The console's settings as one word: a non-Japanese console (bit 4), the settings
             * layout with the full list of languages (2 in bits 13-15), English (1 in bits
             * 16-20), everything else zero (documented in the open ps2sdk; values assumed).
             */
            ee.write32(a0, 0x00014010);
            break;

        case 0x40: {  // CreateSema(param): the initial count is the third word
            int id = next_id_++;
            semaphores[id] = static_cast<int>(ee.read32(a0 + 8));
            result(static_cast<u64>(id));
            break;
        }

        case 0x41:  // DeleteSema
            semaphores.erase(static_cast<int>(a0));
            result(a0);
            break;

        case 0x42:
        case 0x43:  // SignalSema
            semaphores[static_cast<int>(a0)]++;
            result(a0);
            break;

        // WaitSema: with one thread, a wait that would block cannot be satisfied.
        case 0x44:
            // A positive count lets the wait through and takes one off.
            if (semaphores[static_cast<int>(a0)] > 0) {
                semaphores[static_cast<int>(a0)]--;
            } else {
                // Nothing else runs to signal it, so the wait could never end: report it.
                note("WaitSema on an empty semaphore");
            }

            result(a0);
            break;

        case 0x45:
        case 0x46:  // PollSema
            // A positive count is taken at once; otherwise the answer is -1 and nothing waits.
            if (semaphores[static_cast<int>(a0)] > 0) {
                semaphores[static_cast<int>(a0)]--;
                result(a0);
            } else {
                result(static_cast<u64>(-1));
            }

            break;

        // There are no caches here, so these calls succeed and do nothing.
        case 0x61:  // EnableCache
        case 0x62:  // DisableCache
        case 0x64:  // FlushCache
        case 0x68:  // iFlushCache
            // A program flushes the cache after it has loaded code: host code is bound anew.
            native.code_changed();
            result(0);
            break;

        case 0x70:  // GsGetIMR
            result(gs_imr);
            break;

        case 0x71:  // GsPutIMR
            // The result is the old value.
            result(gs_imr);
            gs_imr = a0;
            break;

        case 0x73:  // SetVSyncFlag
            // The flags are not kept; the vertical blank is delivered through its interrupt.
            result(0);
            break;

        // SetSyscall: a program's own kernel calls; the ones that patch the kernel are replaced.
        case 0x74:
            result(0);
            break;

        // SifDmaStat: every transfer is over.
        case 0x76:
            result(static_cast<u64>(-1));
            break;

        case 0x77: {
            /*
             * SifSetDma(records, count): each record copies bytes of this
             * processor's memory into the second one's (source, destination,
             * size, attributes). Done at once. A record is four words, 16 bytes; 32 records
             * is more than a game gives at a time (assumed).
             */
            for (u32 n = 0; n < a1 && n < 32; n++) {
                u32 record = a0 + n * 16;
                u32 from = ee.read32(record), to = ee.read32(record + 4),
                    bytes = ee.read32(record + 8);

                // The copy stops at the size of the second processor's memory, which wraps.
                for (u32 b = 0; b < bytes && b < kIopBytes; b++) {
                    iop_memory[(to + b) & (kIopBytes - 1)] = ee.read8(from + b);
                }
            }

            // The result is a fresh id for the transfer, which SifDmaStat then calls over.
            result(static_cast<u64>(next_id_++));
            break;
        }

        case 0x78:  // SifSetDChain
            result(0);
            break;

        case 0x79:  // SifSetReg
            sif_regs_[a0] = a1;
            result(0);
            break;

        case 0x7A:  // SifGetReg
            // A register that was never set reads 0.
            result(sif_regs_.count(a0) ? sif_regs_[a0] : 0);
            break;

        case 0x7F:  // GetMemorySize
            result(GuestMemory::kRamBytes);
            break;

        default:
            // A call this model does not answer: count it by number and return 0.
            note("kernel call 0x" + [&] {
                char b[8];
                std::snprintf(b, sizeof(b), "%02x", number);
                return std::string(b);
            }());
            result(0);
            break;
    }
}

// --- starting ---

bool Machine::boot(std::string* error) {
    // Called below at each way out: gives the reason to the caller, if it asked, and returns false.
    auto fail = [&](const std::string& why) {
        if (error) {
            *error = why;
        }
        return false;
    };
    auto cnf = disc.find("SYSTEM.CNF");

    // No boot configuration file: the disc is not one this can start.
    if (!cnf) {
        return fail("no SYSTEM.CNF on the disc");
    }

    std::vector<u8> text = disc.read_file(*cnf);
    std::string config(text.begin(), text.end());
    std::size_t at = config.find("cdrom0:");

    // The BOOT2 line names the program as cdrom0:NAME.
    if (at == std::string::npos) {
        return fail("SYSTEM.CNF names no program");
    }

    // The name ends at the version suffix ";1" or the end of the line; "cdrom0:" is 7 characters.
    std::size_t end = config.find_first_of(";\r\n", at);
    std::string name = config.substr(at + 7, end - at - 7);

    // The name may begin with a backslash or a slash after the colon.
    while (!name.empty() && (name[0] == '\\' || name[0] == '/')) {
        name.erase(0, 1);
    }

    program_name = name;
    auto program = disc.find(name);

    // The file the configuration names is not on the disc.
    if (!program) {
        return fail("the disc has no " + name);
    }

    std::vector<u8> elf = disc.read_file(*program);

    // Too short for an ELF header (52 bytes), or no "\x7FELF" magic at the start.
    if (elf.size() < 52
        || std::memcmp(
               elf.data(),
               "\x7F"
               "ELF",
               4
           ) != 0) {
        return fail(name + " is not an ELF program");
    }

    // The program's own table of its vector unit programs, for telling them apart when they run.
    vu_chunks = ps2::vu_program_chunks(elf);

    // ELF32 header: entry point at byte 24, program header table at 28, entry size 42, count 44.
    u32 entry = load<u32>(&elf[24]), phoff = load<u32>(&elf[28]);
    unsigned phentsize = load<u16>(&elf[42]), phnum = load<u16>(&elf[44]);

    // One program header per pass; only the loadable segments are copied in.
    for (unsigned n = 0; n < phnum; n++) {
        const u8* ph = &elf[phoff + n * phentsize];

        // Not a loadable segment: the type at byte 0 of the header is 1 for PT_LOAD.
        if (load<u32>(ph) != 1) {
            continue;
        }

        // Program header: file offset at byte 4, address 8, size in file 16, size in memory 20.
        u32 offset = load<u32>(ph + 4), address = load<u32>(ph + 8), filesz = load<u32>(ph + 16),
            memsz = load<u32>(ph + 20);

        // The segment reaches past the file, or past main memory once the address is made physical.
        if (offset + filesz > elf.size()
            || (address & 0x1FFFFFFF) + memsz > GuestMemory::kRamBytes) {
            return fail("a segment of " + name + " does not fit");
        }

        std::memcpy(memory.ram(address), &elf[offset], filesz);

        // The part of the segment past the file's data (the program's zeroed data) starts as zeros.
        std::memset(memory.ram(address + filesz), 0, memsz - filesz);
        log(1, "loaded %s: %08x-%08x", name.c_str(), address, address + memsz);
    }

    ee.reset();
    ee.pc = entry;
    ee.next_pc = entry + 4;

    // The initial stack pointer, 4 KB under the top of the 32 MB of main memory (assumed).
    ee.gpr[29].lo = 0x01FFF000;

    // The first vertical blank is one field after the start.
    frame_cycles_ = static_cast<u64>(static_cast<double>(kEeHz) / hz);
    vblank_at_ = frame_cycles_;
    ee.event_at = vblank_at_;
    halted = false;
    frames = 0;

    return true;
}

// --- time and interrupts ---

void Machine::run_frame() {
    frame_done_ = false;

    // Run until `vblank` ends the frame, the program exits, or a VU0 program does not stop.
    while (!frame_done_ && !halted && !ee.vu0_runaways) {
        // No time limit: the EE stops itself, at `vblank`, at Exit, or by `event_at`.
        ee.run(~u64{0});

        if (ee.lost) {
            // The program counter left memory. Say how it got there, and stop:
            // nothing sensible follows.
            std::string trail;
            for (const auto& jump : ee.recent_jumps()) {
                char text[32];
                std::snprintf(text, sizeof(text), " %08x>%08x", jump[0], jump[1]);
                trail += text;
            }
            log(0,
                "the program is lost at %08x (ra %08x); the last jumps through a register:%s",
                ee.last_unknown_pc,
                static_cast<u32>(ee.gpr[31].lo),
                trail.c_str());
            halted = true;
            break;
        }

        // Report an unknown instruction once per new count, not at every pass.
        if (ee.unknown && verbose >= 0 && last_reported_unknown_ != ee.unknown) {
            last_reported_unknown_ = ee.unknown;
            log(0,
                "the EE met an instruction it does not know: %08x at %08x (%llu so far)",
                ee.last_unknown,
                ee.last_unknown_pc,
                static_cast<unsigned long long>(ee.unknown));
        }
    }

    // The vector units leave the host rounding towards zero.
    ps2::fp::want_nearest();
}

void Machine::skip_to_vblank() {
    // Only move time forward; a vertical blank already due needs no jump.
    if (ee.cycles < vblank_at_) {
        ee.cycles = vblank_at_;
    }
}

void Machine::raise(unsigned cause) {
    intc_stat |= 1u << cause;
    pending_ |= 1u << cause;

    // Look at it before the next instruction.
    ee.event_at = ee.cycles;
}

void Machine::event() {
    // The vertical blank has ended: INTC cause 3, raised once.
    if (ee.cycles >= vblank_end_at_) {
        vblank_end_at_ = ~u64{0};
        raise(3);
    }

    // The next vertical blank has begun.
    if (ee.cycles >= vblank_at_) {
        vblank();
    }

    deliver();

    // The disc callback is due and the program can take it: COP0 Status IE (bit 0), EIE (16) set.
    if (ee.cycles >= cd_callback_at_ && !in_handler_ && (ee.cop0[12] & 0x10001) == 0x10001) {
        cd_callback_at_ = ~u64{0};

        // The program may have asked for no callback.
        if (cd_callback) {
            in_handler_ = true;

            // The callback's argument 1 says that a read finished.
            call_handler(cd_callback, cd_callback_gp, 1);
            in_handler_ = false;
        }
    }

    update_event();
}

void Machine::update_event() {
    // The nearest of the blank, its end and the disc callback; one that is held back is retried.
    u64 next = std::min(
        {vblank_at_,
         vblank_end_at_,
         cd_callback_at_ > ee.cycles ? cd_callback_at_ : ee.cycles + 4000}
    );

    // Interrupts the program has switched off stay pending; look again soon.
    if (pending_ & intc_mask) {
        next = std::min(next, ee.cycles + 4000);
    }
    ee.event_at = next;
}

void Machine::vblank() {
    // The blank lasts about a twelfth of a field (assumed); the next one starts a field later.
    vblank_end_at_ = vblank_at_ + frame_cycles_ / 12;
    vblank_at_ += frame_cycles_;
    frames++;

    // The drawing side learns of the blank in order with the data given before it.
    drawing.vblank(odd_field());

    // INTC cause 2 is the start of the vertical blank.
    raise(2);
    deliver();

    // What the program's handler sent is drawn, then the picture is taken.
    drawing.present();
    // One field's worth of sound, whether or not anybody listens: the
    // streams run their course either way.
    sound_owed_ += Sound::kRate / hz;
    std::size_t count = static_cast<std::size_t>(sound_owed_);

    // Keep the fraction of a frame that did not make a whole one, for the next field.
    sound_owed_ -= static_cast<double>(count);
    sound_out_.clear();
    sound.mix(count, sound_out_);

    // The sound effects go on top of the streams; the sum is clipped to 16 bits.
    effect_sums_.assign(count * 2, 0);
    effects.mix(count, effect_sums_.data());

    for (std::size_t n = 0; n < effect_sums_.size(); n++) {
        s32 sum = sound_out_[n] + effect_sums_[n];

        sound_out_[n] = static_cast<ps2::s16>(std::clamp(sum, -32768, 32767));
    }

    // A listener, if any, takes the field's sound.
    if (on_sound) {
        on_sound(sound_out_.data(), count);
    }

    // The frame is complete: let the host show it.
    if (on_vblank) {
        on_vblank();
    }

    // Stop the EE so that `run_frame` returns.
    frame_done_ = true;
    ee.stop();

    // The owner's work between two fields.
    if (on_frame) {
        on_frame();
    }
}

void Machine::call_handler(u32 function, u32 gp, u64 a0, u64 a1) {
    u64 interrupted_gp = ee.gpr[28].lo;

    // `call` puts back every register but takes the global pointer as it finds it.
    ee.gpr[28].lo = static_cast<u64>(static_cast<s64>(static_cast<s32>(gp)));
    ee.call_stack = kHandlerStack;
    ee.call(function, a0, a1);
    ee.gpr[28].lo = interrupted_gp;
}

void Machine::deliver() {
    // Handlers run with interrupts off, one cause at a time, and only when the
    // program has interrupts on.
    if (in_handler_ || (ee.cop0[12] & 0x10001) != 0x10001) {
        return;
    }

    in_handler_ = true;

    // The sixteen INTC causes in order, lowest first.
    for (unsigned cause = 0; cause < 16; cause++) {
        u32 bit = 1u << cause;

        // Nothing is waiting on this cause.
        if (!(pending_ & bit)) {
            continue;
        }

        // The program has not enabled this cause.
        if (!(intc_mask & bit)) {
            // Not asked for: forget it, the status bit stays for anyone polling.
            if (cause != 1) {
                pending_ &= ~bit;
            }

            continue;
        }

        pending_ &= ~bit;
        intc_stat &= ~bit;

        // The DMA controller's cause is shared by its channels.
        if (cause == 1) {
            // The DMA controller: one handler list per channel that finished and
            // whose interrupt the program enabled.
            for (unsigned channel = 0; channel < 10; channel++) {
                // D_STAT bit n: channel n finished; bit 16+n unmasks its interrupt (documented).
                u32 done = 1u << channel, enabled = 1u << (16 + channel);

                if ((d_stat & done) && (d_stat & enabled)) {
                    d_stat &= ~done;

                    // The list is copied: a handler may add or remove handlers while it runs.
                    for (const Handler& h : std::vector<Handler>(dmac_handlers[channel])) {
                        call_handler(h.function, h.gp, channel, h.argument);
                    }
                }
            }
        } else {
            // Any other cause: its handlers, each called with the cause and its own argument.
            for (const Handler& h : std::vector<Handler>(intc_handlers[cause])) {
                call_handler(h.function, h.gp, cause, h.argument);
            }
        }
    }

    in_handler_ = false;
}

// --- DMA ---

void Machine::dma_done(unsigned channel) {
    // A finished transfer clears CHCR bit 8 (STR) and sets D_STAT bit n for channel n (documented).
    dma[channel].chcr &= ~0x100u;
    d_stat |= 1u << channel;

    // Only a channel whose interrupt is unmasked (D_STAT bit 16+n) raises the DMAC cause.
    if (d_stat & (1u << (16 + channel))) {
        raise(1);
    }
}

void Machine::dma_start(unsigned channel) {
    DmaChannel& ch = dma[channel];

    // CHCR bit 0 is the direction, bits 2-3 the mode: 0 normal, 1 source chain (documented).
    bool from_memory = (ch.chcr & 1) != 0;
    unsigned mode = (ch.chcr >> 2) & 3;
    log(3,
        "DMA channel %u: chcr %08x madr %08x qwc %x tadr %08x",
        channel,
        ch.chcr,
        ch.madr,
        ch.qwc,
        ch.tadr);

    switch (channel) {
        case kVif0:
        case kVif1: {
            /*
             * VIF0 is the EE's own; what goes to VIF1 is copied for the drawing side. The sink is
             * called by the chain walker, on the EE's thread, with each piece of data.
             */
            DmaSink sink = channel == kVif0 ? DmaSink([this](const u8* data, std::size_t bytes) {
                vif0.write(data, bytes);
            })
                                            : DmaSink([this](const u8* data, std::size_t bytes) {
                                                  drawing.vif(data, bytes);
                                              });
            // To memory: what the GS sends back from a local-to-host transfer.
            if (!from_memory) {
                // The GS must have drawn everything given before its memory is read.
                drawing.sync();

                // QWC counts 16-byte quadwords; MADR ends past the data and QWC at 0.
                graphics.gs.transfer_out(memory.dma(ch.madr), std::size_t{ch.qwc} * 16);
                ch.madr += ch.qwc * 16;
                ch.qwc = 0;
            } else if (mode == 1) {
                // Source chain: the tags in memory say what to send; a runaway list is reported.
                if (run_source_chain(memory, ch, sink) == DmaStop::Runaway) {
                    note("a VIF display list that does not end");
                }
            } else {
                // Normal mode: one block of QWC quadwords from MADR.
                sink(memory.dma(ch.madr), std::size_t{ch.qwc} * 16);
                ch.madr += ch.qwc * 16;
                ch.qwc = 0;
            }

            // Whatever VIF1 was given goes to the drawing side as one command.
            if (channel == kVif1) {
                drawing.send();
            }

            break;
        }

        case kGifChannel: {
            // The sink is called by the chain walker, on the EE's thread, with each piece of data.
            DmaSink sink = [this](const u8* data, std::size_t bytes) {
                drawing.gif(data, bytes);
            };

            if (mode == 1) {
                // GIF chains carry no data in their tags: CHCR bit 6 (TTE) is cleared.
                ch.chcr &= ~0x40u;
                run_source_chain(memory, ch, sink);
            } else {
                // Normal mode: one block of QWC quadwords from MADR.
                sink(memory.dma(ch.madr), std::size_t{ch.qwc} * 16);
                ch.madr += ch.qwc * 16;
                ch.qwc = 0;
            }

            drawing.send();
            break;
        }

        case kFromSpr: {
            // Scratchpad to memory.
            u32& sadr = dma_sadr[channel];

            if (mode == 0) {
                // The scratchpad is 16 KB, so SADR wraps at 0x4000 (documented).
                std::memcpy(memory.dma(ch.madr), memory.scratchpad(sadr), std::size_t{ch.qwc} * 16);
                ch.madr += ch.qwc * 16;
                sadr = (sadr + ch.qwc * 16) & 0x3FFF;
                ch.qwc = 0;
            } else {
                /*
                 * Destination chain: each tag in the scratchpad says where its data goes. At most
                 * 4096 tags, so a list that never ends cannot hang the machine.
                 */
                for (int guard = 0; guard < 4096; guard++) {
                    // Tag: QWC bits 0-15, ID bits 28-30, IRQ bit 31, ADDR bits 32-62 (documented).
                    u64 tag = load<u64>(memory.scratchpad(sadr));
                    u32 qwc = static_cast<u32>(tag & 0xFFFF),
                        id = static_cast<u32>((tag >> 28) & 7);
                    u32 to = static_cast<u32>(tag >> 32);

                    // The tag takes one quadword; its data follows, to the address the tag names.
                    sadr = (sadr + 16) & 0x3FFF;
                    std::memcpy(memory.dma(to), memory.scratchpad(sadr), std::size_t{qwc} * 16);
                    sadr = (sadr + qwc * 16) & 0x3FFF;

                    // The tag's bits 16-31 are kept in CHCR's TAG field, bits 16-31 (documented).
                    ch.chcr = (ch.chcr & 0xFFFF) | (static_cast<u32>(tag) & 0xFFFF0000u);

                    // ID 7 is END; IRQ stops the chain when the channel's TIE bit (7) is set.
                    if (id == 7 || ((tag >> 31) & 1 && (ch.chcr & 0x80))) {
                        break;
                    }
                }
            }

            break;
        }

        case kToSpr: {
            // Memory to scratchpad.
            u32& sadr = dma_sadr[channel];

            // The sink is called by the chain walker, on the EE's thread, with each piece of data.
            DmaSink sink = [this, &sadr](const u8* data, std::size_t bytes) {
                // Byte by byte, wrapping at the end of the 16 KB scratchpad.
                for (std::size_t n = 0; n < bytes; n++) {
                    *memory.scratchpad(sadr) = data[n];
                    sadr = (sadr + 1) & 0x3FFF;
                }
            };

            if (mode == 1) {
                run_source_chain(memory, ch, sink);
            } else {
                // Normal mode: one block of QWC quadwords from MADR.
                sink(memory.dma(ch.madr), std::size_t{ch.qwc} * 16);
                ch.madr += ch.qwc * 16;
                ch.qwc = 0;
            }

            break;
        }

        default:
            // A channel this model does not carry out (IPU, SIF): counted, and finished at once.
            note("DMA on channel " + std::to_string(channel));
            break;
    }

    dma_done(channel);
}

// --- hardware registers ---

u32 Machine::timer_count(const Timer& t) const {
    // MODE bit 7 (CUE) is the count enable; a stopped timer holds its count.
    if (!(t.mode & 0x80)) {
        return t.base_count & 0xFFFF;
    }

    // The bus clock is half the CPU's; then /1, /16, /256, or one count a scan line.
    static constexpr u64 divider[3] = {2, 32, 512};

    // MODE bits 0-1 pick the clock (documented).
    unsigned clock = t.mode & 3;

    // The scan-line clock counts lines per field: about 312 at 50 Hz, 262 at 60 Hz (assumed).
    u64 per_count = clock < 3 ? divider[clock] : frame_cycles_ / (hz < 55 ? 312 : 262);

    // The count has advanced by whole counts since it was last set, and is 16 bits wide.
    return static_cast<u32>(t.base_count + (ee.cycles - t.base_cycles) / per_count) & 0xFFFF;
}

u64 Machine::hw_read(u32 address, unsigned bytes) {
    (void)bytes;

    // The GS's privileged registers (0x12000000-0x12001FFF); the drawing side must catch up first.
    if (address >= 0x12000000 && address < 0x12002000) {
        drawing.sync();
        return graphics.gs.read_privileged(address & ~0xFu);
    }

    // The timers (0x10000000-0x10001FFF): 0x800 apart, four registers each at 0x10 intervals.
    if (address >= 0x10000000 && address < 0x10002000) {
        const Timer& t = timers[(address >> 11) & 3];

        switch (address & 0x7F0) {
            case 0x00:  // COUNT
                return timer_count(t);

            case 0x10:  // MODE
                return t.mode;

            case 0x20:  // COMP
                return t.compare;

            default:  // HOLD
                return t.hold;
        }
    }

    // The ten DMA channels' registers (0x10008000-0x1000DFFF).
    if (address >= 0x10008000 && address < 0x1000E000) {
        // Find the channel whose 0x100-byte block holds the address.
        for (unsigned n = 0; n < 10; n++) {
            if ((address & ~0xFFu) == kDmaBase[n]) {
                switch (address & 0xF0) {
                    case 0x00:  // CHCR
                        return dma[n].chcr;

                    case 0x10:  // MADR
                        return dma[n].madr;

                    case 0x20:  // QWC
                        return dma[n].qwc;

                    case 0x30:  // TADR
                        return dma[n].tadr;

                    case 0x40:  // ASR0
                        return dma[n].asr[0];

                    case 0x50:  // ASR1
                        return dma[n].asr[1];

                    case 0x80:  // SADR
                        return dma_sadr[n];

                    default:
                        // A register of the block that is not modelled reads as zero.
                        return 0;
                }
            }
        }
    }

    switch (address) {
        case 0x1000E000:  // D_CTRL
            return d_ctrl;

        case 0x1000E010:  // D_STAT
            return d_stat;

        case 0x1000E020:  // D_PCR
            return d_pcr;

        case 0x1000E030:  // D_SQWC
            return d_sqwc;

        case 0x1000E040:  // D_RBSR
            return d_rbsr;

        case 0x1000E050:  // D_RBOR
            return d_rbor;

        case 0x1000F000:  // INTC_STAT
            return intc_stat;

        case 0x1000F010:  // INTC_MASK
            return intc_mask;

        case 0x1000F520:  // D_ENABLER
            return d_enable;

        case 0x10003020:  // GIF_STAT
        case 0x10003800:  // VIF0_STAT
        case 0x10003C00:  // VIF1_STAT: idle, FIFOs empty
        case 0x1000F130:  // serial status
            return 0;

        default: {
            // A register this model does not have: reported once, reads as zero.
            char text[48];
            std::snprintf(text, sizeof(text), "a read of hardware register %08x", address);
            note(text);
            return 0;
        }
    }
}

void Machine::hw_write(u32 address, u64 value, unsigned bytes) {
    (void)bytes;
    u32 v = static_cast<u32>(value);

    // The GS's privileged registers: queued behind the data already given to the drawing side.
    if (address >= 0x12000000 && address < 0x12002000) {
        drawing.privileged(address & ~0xFu, value);
        return;
    }

    // The timers: the same layout as for reads (see `hw_read`).
    if (address >= 0x10000000 && address < 0x10002000) {
        Timer& t = timers[(address >> 11) & 3];

        switch (address & 0x7F0) {
            case 0x00:  // COUNT
                t.base_count = v & 0xFFFF;
                t.base_cycles = ee.cycles;
                break;

            case 0x10:  // MODE
                // Count from the current value under the new mode.
                t.base_count = timer_count(t);
                t.base_cycles = ee.cycles;

                // Bits 10 and 11 are flags, cleared by writing them.
                t.mode = v & 0x3FF;
                break;

            case 0x20:  // COMP
                t.compare = v & 0xFFFF;
                break;

            default:  // HOLD
                t.hold = v & 0xFFFF;
                break;
        }

        return;
    }

    // The ten DMA channels' registers.
    if (address >= 0x10008000 && address < 0x1000E000) {
        for (unsigned n = 0; n < 10; n++) {
            // Not this channel's block.
            if ((address & ~0xFFu) != kDmaBase[n]) {
                continue;
            }

            switch (address & 0xF0) {
                case 0x00:  // CHCR
                    dma[n].chcr = v;

                    // Bit 8 (STR) starts the transfer, which is carried out at once.
                    if (v & 0x100) {
                        dma_start(n);
                    }

                    break;

                case 0x10:  // MADR
                    dma[n].madr = v;
                    break;

                case 0x20:  // QWC, 16 bits
                    dma[n].qwc = v & 0xFFFF;
                    break;

                case 0x30:  // TADR
                    dma[n].tadr = v;
                    break;

                case 0x40:  // ASR0
                    dma[n].asr[0] = v;
                    break;

                case 0x50:  // ASR1
                    dma[n].asr[1] = v;
                    break;

                case 0x80:  // SADR, within the 16 KB scratchpad
                    dma_sadr[n] = v & 0x3FFF;
                    break;

                default:
                    // A register of the block that is not modelled ignores the write.
                    break;
            }

            return;
        }
    }

    switch (address) {
        case 0x1000E000:  // D_CTRL
            d_ctrl = v;
            break;

        case 0x1000E010:  // D_STAT
            // Writing 1 clears a channel's status bit and flips its enable bit.
            d_stat &= ~(v & 0x3FF);
            d_stat ^= v & 0x03FF0000;
            break;

        case 0x1000E020:  // D_PCR
            d_pcr = v;
            break;

        case 0x1000E030:  // D_SQWC
            d_sqwc = v;
            break;

        case 0x1000E040:  // D_RBSR
            d_rbsr = v;
            break;

        case 0x1000E050:  // D_RBOR
            d_rbor = v;
            break;

        case 0x1000F590:  // D_ENABLEW
            d_enable = v;
            break;

        case 0x1000F000:  // INTC_STAT
            // Writing 1 clears a status bit (documented).
            intc_stat &= ~v;
            break;

        case 0x1000F010:  // INTC_MASK
            // Writing 1 flips a mask bit; the register has 15 bits (documented).
            intc_mask ^= v & 0x7FFF;
            break;

        case 0x1000F180:  // the serial port: the kernel's console
            // A newline ends a line of text, which is printed; a carriage return is dropped.
            if (v == '\n') {
                log(0, "[tty] %s", tty_.c_str());
                tty_.clear();
            } else if (v != '\r') {
                // Any other character is added to the line.
                tty_.push_back(static_cast<char>(v));
            }

            break;

        /*
         * Writes to these registers are accepted and ignored. VIF1_STAT: only its direction bit
         * can be written, and the DMA channel says the same.
         */
        case 0x10003000:  // GIF_CTRL
        case 0x10003810:  // VIF0_FBRST
        case 0x10003820:  // VIF0_ERR
        case 0x10003C00:  // VIF1_STAT
        case 0x10003C10:  // VIF1_FBRST
        case 0x10003C20:  // VIF1_ERR
        case 0x1000F100:  // serial control
            break;

        default: {
            // A register this model does not have: reported once, the write is lost.
            char text[64];
            std::snprintf(text, sizeof(text), "a write to hardware register %08x", address);
            note(text);
            break;
        }
    }
}

void Machine::hw_write128(u32 address, u64 lo, u64 hi) {
    u8 quad[16];
    store<u64>(quad, lo);
    store<u64>(quad + 8, hi);
    switch (address) {
        case 0x10004000:  // VIF0 FIFO
            vif0.write(quad, 16);
            break;

        case 0x10005000:  // VIF1 FIFO
            drawing.vif(quad, 16);
            break;

        case 0x10006000:  // GIF FIFO
            drawing.gif(quad, 16);
            break;

        default:
            // Any other address takes the two halves as separate 64-bit writes.
            hw_write(address, lo, 8);
            hw_write(address + 8, hi, 8);
            break;
    }
}

void Machine::add_default_services() {
    // Services that return a fixed value or nothing, for the library functions that need no more.
    add_service("return0", [](Machine& m) { m.result(0); });
    add_service("return1", [](Machine& m) { m.result(1); });
    add_service("return2", [](Machine& m) { m.result(2); });
    add_service("return_minus1", [](Machine& m) { m.result(static_cast<u64>(-1)); });
    add_service("nothing", [](Machine&) {});

    // The services that need to know what the library does are in services.cpp.
    add_library_services(*this);
}

}  // namespace sys
