// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The console as a game program needs it: the machine that connects the parts and answers the
 * kernel.
 *
 * Its definitions are in machine.cpp (boot, time, interrupts, DMA, hardware registers, kernel
 * calls), services.cpp (the replaced library functions) and memcard.cpp (the memory card
 * library). It leaves out the second processor and the devices behind it.
 *
 * Sources: the EE's memory map, DMA controller, timers and interrupt controller as publicly
 * documented, and what the games' own code asks of the kernel.
 */

#pragma once

#include <array>
#include <cstdio>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "disc.h"
#include "drawing.h"
#include "ps2/dma.h"
#include "ps2/ee.h"
#include "ps2/graphics.h"
#include "ps2/memory.h"
#include "ps2/vu_programs.h"
#include "snd/player.h"
#include "sound.h"
#include "sys/native.h"

namespace sys {

using ps2::u16;
using ps2::u32;
using ps2::u64;
using ps2::u8;

class Machine;

/**
 * A function of the game's program that the runtime answers itself. It
 * reads its arguments from the EE's registers and sets the result; the
 * machine then returns to the caller.
 */
using Service = std::function<void(Machine&)>;

/**
 * The whole console as a game program needs it, without the parts a game
 * only reaches through Sony's libraries: the EE with its memory, the vector
 * units, the drawing path, the DMA controller, the timers, the interrupt
 * controller and the kernel's services. The second processor and everything
 * behind it (disc drive, memory cards, pads, sound) are not modelled; the
 * library functions that talk to them are replaced by name (see `hook`).
 *
 * Threads: everything here runs on the thread that calls `run_frame`, the EE's thread, including
 * the callbacks and the services. The only other thread belongs to `drawing`, which says what it
 * takes from this one. The public data members are plain state of the console and are not
 * guarded.
 *
 * The machine owns files (the memory card's) and a thread (the drawing path's), so its copy
 * operations are deleted.
 *
 * Sources: the EE's memory map, the DMA controller, timers and interrupt controller as publicly
 * documented, and the kernel calls the games' own code makes.
 */
class Machine {
public:
    /** Connects the EE's hardware callbacks to this machine and registers the default services. */
    Machine();
    Machine(const Machine&) = delete;
    Machine& operator=(const Machine&) = delete;

    /** Guest memory: main memory and the scratchpad. */
    ps2::GuestMemory memory;

    /** VIF1, VU1, GIF, GS. */
    ps2::Graphics graphics;

    /**
     * How the machine reaches them: in order, and on a thread of their own
     * once `drawing.start()` has been called. Reading `graphics` directly is
     * for after `drawing.sync()`.
     */
    Drawing drawing{graphics};

    /** The GIF that `vif0` is built on; the machine uses it for nothing else. */
    ps2::Gif unused_gif{graphics.gs};

    /** VIF0, which loads and starts VU0's programs; it shares the class of VIF1. */
    ps2::Vif1 vif0{unused_gif};

    /** VU0, running on the memories of `vif0`, 4 KB of program and 4 KB of data (documented). */
    ps2::Vu vu0{ps2::Vu::Memory{vif0.micro.data(), 4096, vif0.data.data(), 4096}};

    /** The EE's CPU core, over `memory` and `vu0`. */
    ps2::Ee ee{memory, vu0};

    /** The disc image the program is loaded from and reads. */
    Disc disc;

    /** The sound library's streams, read from `disc`. */
    Sound sound{disc};

    /** The sound library's sound effects, played from the banks a program loads. */
    snd::Player effects;

    /** Host code that stands in for guest functions, when a library of it is loaded. */
    Native native{ee, memory};

    /**
     * Loads the program the disc boots (SYSTEM.CNF's BOOT2) and gets ready to run it.
     *
     * `hz` is the display's field rate and must be set before the call.
     *
     * @param[out] error Receives the reason when the call fails; may be null.
     * @return True when the program is loaded and ready for `run_frame`.
     * @pre `disc` is open.
     */
    bool boot(std::string* error = nullptr);

    /** The display's field rate, 50 for PAL; the length of a frame is worked out from it. */
    double hz = 50.0;

    /** The program's file on the disc, which is the disc's code (SCES_509.16). */
    std::string program_name;

    /**
     * The chunks of the program's vector unit programs, from its own table (ps2/vu_programs.h);
     * empty if it has none. For telling which program a unit runs.
     */
    std::vector<ps2::VuChunk> vu_chunks;

    /**
     * Runs until the next vertical blank has been announced to the program.
     *
     * Stops early when the program ends, is lost, or runs a VU0 microprogram that does not stop.
     */
    void run_frame();

    /** Called at each vertical blank, after the game's handlers: show a frame. */
    std::function<void()> on_vblank;

    /**
     * Called when a field is complete, as the last thing of the vertical blank.
     *
     * The program may be anywhere at that moment, also inside host code that stands in for a
     * guest function, so an owner that does its per-field work here does it at the same field
     * however the program is run. It may end the process; it must not call `run_frame`.
     */
    std::function<void()> on_frame;

    /**
     * Called at each vertical blank with that field's sound: pairs of left
     * and right at Sound::kRate.
     */
    std::function<void(const ps2::s16* samples, std::size_t frames)> on_sound;

    /** True once the program has called Exit or got lost; `run_frame` does nothing more. */
    bool halted = false;

    /** Vertical blanks since the program was loaded. */
    u64 frames = 0;

    /**
     * Makes calls to `address` go to the named service.
     *
     * @param address Entry address of the program's function.
     * @param service Name given to `add_service`.
     * @return False when no service has that name.
     */
    bool hook(u32 address, const std::string& service);

    /**
     * Reads a table of `address service` lines; '#' starts a comment.
     *
     * @param path Host path of the table.
     * @param[out] error Receives the reason when the call fails; may be null.
     * @return True when every line was applied.
     */
    bool load_hooks(const std::string& path, std::string* error = nullptr);

    /**
     * Registers a service under a name, so that `hook` and a hooks table can refer to it.
     *
     * @param name The name; a second service with the same name replaces the first.
     * @param service What to run in place of the program's function.
     */
    void add_service(const std::string& name, Service service) {
        services_[name] = std::move(service);
    }

    /** True if a service has been registered under this name. */
    bool has_service(const std::string& name) const { return services_.count(name) != 0; }

    /**
     * For services: the integer argument registers, a0 onward.
     *
     * @param n Argument number: 0-3 are a0-a3 and 4-7 t0-t3 (documented).
     * @return The low 32 bits of that register.
     */
    u32 arg(unsigned n) { return static_cast<u32>(ee.gpr[4 + n].lo); }

    /**
     * For services: sets the result, which the program reads from v0.
     *
     * @param value The result.
     */
    void result(u64 value) { ee.gpr[2].lo = value; }

    /**
     * For services: reads a zero-terminated string out of guest memory.
     *
     * @param address Guest address of the first character.
     * @param limit Most characters to read.
     * @return The characters up to the terminator or the limit.
     */
    std::string string_at(u32 address, std::size_t limit = 1024);

    /**
     * For services: a printf format with its arguments taken from the argument registers
     * (integers a0-a3, t0-t3). Enough for the messages games print.
     *
     * Floating-point conversions print a placeholder.
     *
     * @param format_address Guest address of the format string.
     * @param first_arg Number of the first argument register, as for `arg`.
     * @return The formatted text.
     */
    std::string format(u32 format_address, unsigned first_arg);

    /** Lets guest time jump to the next vertical blank (the program is waiting). */
    void skip_to_vblank();

    /** True when the number of vertical blanks so far is odd. */
    bool odd_field() const { return (frames & 1) != 0; }

    /**
     * The pad as libpad reports it: buttons as a bit mask (bit set = pressed,
     * in the order select, L3, R3, start, up, right, down, left, L2, R2, L1,
     * R1, triangle, circle, cross, square) and the two sticks, 0x80 centred.
     */
    struct Pad {
        /** The pressed buttons, one bit each, in the order above. */
        u16 buttons = 0;

        /** The stick axes, 0x80 when centred. */
        u8 right_x = 0x80, right_y = 0x80, left_x = 0x80, left_y = 0x80;
    } pad;

    /** The function the program asked to be told when a disc read finishes. */
    u32 cd_callback = 0;

    /** The global pointer (register 28) of the code that named `cd_callback`. */
    u32 cd_callback_gp = 0;

    /**
     * Tells the program soon that a disc read finished, as the drive would: not before the caller
     * has returned.
     *
     * It is 200,000 cycles later (about 0.7 ms at `kEeHz`).
     */
    void cd_read_finished() {
        cd_callback_at_ = ee.cycles + 200000;
        update_event();
    }

    /**
     * Which server each remote-call client was bound to, by the client
     * record's address.
     */
    std::unordered_map<u32, u32> rpc_servers;

    /** Size of the second processor's memory, 2 MB (documented). */
    static constexpr u32 kIopBytes = 2 * 1024 * 1024;

    /**
     * Where the stack of interrupt handlers and callbacks starts: the top of the first megabyte
     * of main memory. That megabyte is the kernel's on the console (documented); no program is
     * loaded there. Host code's data and stack are below it (`Native::kStackTop`).
     */
    static constexpr u32 kHandlerStack = 0x000FFFF0;

    /**
     * The second processor's memory, as far as a program uses it itself: the
     * games park data there (through SifSetDma) and fetch it back.
     */
    std::vector<u8> iop_memory = std::vector<u8>(kIopBytes);

    /** The sound server's next handle for a bank, stream or sound. */
    u32 sound_next_handle = 0x100;

    /**
     * The memory card in the first slot: a directory of the host (none when
     * the name is empty), the files the program has open on it, and whether
     * the program has been told of it yet.
     */
    struct Card {
        /** The host directory that stands for the card; empty for no card. */
        std::string directory;

        /** The host files the program has open, by the handle it was given. */
        std::unordered_map<int, std::FILE*> open;

        /** True once the program has been told that a card is present. */
        bool seen = false;

        /** Closes the files still open. */
        ~Card();
    } card;

    /** The memory card library's "last function and its result". */
    int mc_function = 0, mc_result = 0;

    /**
     * Raises an interrupt in the interrupt controller.
     *
     * @param cause An INTC cause: 1 DMAC, 2 vertical blank start, 3 its end, ...
     */
    void raise(unsigned cause);

    /** How much `log` prints: 0 failures only, 1 notable events, 2 and up a trace of calls. */
    int verbose = 0;

    /**
     * Writes a line to `stderr` when `level` is at most `verbose`.
     *
     * @param level 0 for a failure the user needs to see, 1 for a notable event, 2 and up for
     *     traces.
     * @param format A printf format; the compiler checks it against the arguments.
     */
    void log(int level, const char* format, ...) __attribute__((format(printf, 3, 4)));

    /** Counts of things met that are not modelled, by a short description. */
    std::map<std::string, u64> notes;

    /**
     * Counts a thing that is not modelled and reports it the first time.
     *
     * @param what A short description; the same text counts as the same thing.
     */
    void note(const std::string& what);

    /** The kernel's memory of what the program asked for. */
    struct Handler {
        /** Guest address of the handler function and the argument it is called with. */
        u32 function = 0, argument = 0;

        /** The number the program uses to remove the handler. */
        int id = 0;

        /** The global pointer (register 28) of the code that added the handler. */
        u32 gp = 0;
    };

    /** The interrupt handlers the program added, per INTC cause, in the order they are called. */
    std::array<std::vector<Handler>, 16> intc_handlers;

    /** The DMA handlers the program added, per channel, in the order they are called. */
    std::array<std::vector<Handler>, 16> dmac_handlers;

    /** The semaphores the program created: their ids, and the count each holds. */
    std::unordered_map<int, int> semaphores;

    /** The ten DMA channels' registers, which the EE reads and writes directly. */
    std::array<ps2::DmaChannel, 10> dma{};

    /** The scratchpad address register of each channel (SADR), for the two scratchpad channels. */
    std::array<u32, 10> dma_sadr{};

    /**
     * The DMA controller's global registers: D_CTRL, D_STAT, D_PCR, D_SQWC, D_RBSR, D_RBOR and
     * D_ENABLE (documented). D_ENABLE starts at 0x1201 (assumed).
     */
    u32 d_ctrl = 0, d_stat = 0, d_pcr = 0, d_sqwc = 0, d_rbsr = 0, d_rbor = 0, d_enable = 0x1201;

    /** The interrupt controller's status (INTC_STAT) and mask (INTC_MASK) registers. */
    u32 intc_stat = 0, intc_mask = 0;

    /** One of the EE's four timers: its registers and when the count was last set. */
    struct Timer {
        /** The mode, compare and hold registers, as the program wrote them. */
        u32 mode = 0, compare = 0, hold = 0;

        /** The count at `base_cycles`; the count since then is worked out from the time. */
        u32 base_count = 0;

        /** The EE cycle at which the count was `base_count`. */
        u64 base_cycles = 0;
    };

    /** The EE's four timers. */
    std::array<Timer, 4> timers{};

    /** The GS's interrupt mask register (IMR), kept for the kernel calls that read and write it. */
    u32 gs_imr = 0;

    /** The EE's clock rate in Hz (documented). */
    static constexpr u64 kEeHz = 294912000;

private:
    /**
     * Answers a read of a hardware register outside memory.
     *
     * @param address Physical address.
     * @param bytes Width of the access; not used.
     * @return The register's value, or 0 for one that is not modelled.
     */
    u64 hw_read(u32 address, unsigned bytes);

    /**
     * Applies a write to a hardware register outside memory.
     *
     * @param address Physical address.
     * @param value The value written.
     * @param bytes Width of the access; not used.
     */
    void hw_write(u32 address, u64 value, unsigned bytes);

    /**
     * Applies a 128-bit write: a FIFO takes it whole, any other address takes two 64-bit writes.
     *
     * @param address Physical address.
     * @param lo Low 64 bits.
     * @param hi High 64 bits.
     */
    void hw_write128(u32 address, u64 lo, u64 hi);

    /** Handles whatever is due at the current cycle: blanks, interrupts, the disc callback. */
    void event();

    /** Starts a vertical blank: raises it, draws what was sent, makes the sound, ends the frame. */
    void vblank();

    /** Runs the handlers of the interrupts that are pending, enabled, and allowed by the EE. */
    void deliver();

    /**
     * Calls a handler or callback of the program that breaks into whatever code is running.
     *
     * It runs on the handlers' stack with the global pointer of the code that registered it, as
     * under the console's kernel: the code it interrupts may be using both registers for
     * something else (seen in game code).
     *
     * @param function Address of the function.
     * @param gp The global pointer to give it.
     * @param a0 Its first argument.
     * @param a1 Its second argument.
     */
    void call_handler(u32 function, u32 gp, u64 a0, u64 a1 = 0);

    /**
     * Handles a SYSCALL: a replaced library function or a kernel call.
     *
     * @param code The 20-bit code field of the instruction.
     */
    void syscall(u32 code);

    /**
     * Answers a kernel call.
     *
     * @param number The call's number, from v1 with the sign dropped.
     */
    void kernel(int number);

    /**
     * Carries out the transfer of a DMA channel whose STR bit has been set.
     *
     * @param channel Channel number, 0-9.
     */
    void dma_start(unsigned channel);

    /**
     * Marks a channel's transfer as finished and raises the DMAC interrupt if it is enabled.
     *
     * @param channel Channel number, 0-9.
     */
    void dma_done(unsigned channel);

    /**
     * Works out the value of a timer's count register now.
     *
     * @param t The timer.
     * @return The 16-bit count.
     */
    u32 timer_count(const Timer& t) const;

    /** Registers the services every game shares. */
    void add_default_services();

    /** Works out when the EE next needs to look at the machine, and sets `ee.event_at`. */
    void update_event();

    /** The services by name. */
    std::unordered_map<std::string, Service> services_;

    /** The service each hooked function runs, indexed by the number in its SYSCALL. */
    std::vector<Service> hooked_;

    /** The names of `hooked_`, for the trace. */
    std::vector<std::string> hooked_names_;

    /** The first instruction of each hooked function before it was replaced, by address. */
    std::unordered_map<u32, u32> hooked_original_;

    /**
     * Cycles in one field; the cycle of the next vertical blank start; the cycle its end is due,
     * or all ones when none is.
     */
    u64 frame_cycles_ = 0, vblank_at_ = 0, vblank_end_at_ = ~u64{0};

    /** Interrupt causes raised and not yet delivered, one bit per cause. */
    u32 pending_ = 0;

    /** True while a handler or callback runs; set so that none starts inside another. */
    bool in_handler_ = false;

    /** Set by `vblank` to end the current `run_frame`. */
    bool frame_done_ = false;

    /** The next id given to a handler, thread, semaphore or transfer. */
    int next_id_ = 1;

    /** The characters the program has written to the serial port since the last newline. */
    std::string tty_;

    /** The values the program set in the second processor's interface registers (SifSetReg). */
    std::unordered_map<u32, u32> sif_regs_;

    /** The top of the stack the program set up; the heap ends 0x4000 below it. */
    u32 stack_top_ = ps2::GuestMemory::kRamBytes;

    /** The count of unknown instructions as last reported, so each new one is told once. */
    u64 last_reported_unknown_ = 0;

    /** The cycle the disc callback is due, or all ones when none is. */
    u64 cd_callback_at_ = ~u64{0};

    /** The sound made at the latest vertical blank. */
    std::vector<ps2::s16> sound_out_;

    /** The sound effects' part of it, as sums before clipping; kept to use the buffer again. */
    std::vector<ps2::s32> effect_sums_;

    /** Frames of sound not yet made, in fractions. */
    double sound_owed_ = 0;
};

/**
 * Services every game shares, registered by `add_default_services` and
 * defined in services.cpp.
 *
 * @param machine The machine to register them with.
 */
void add_library_services(Machine& machine);

/**
 * The memory card library (memcard.cpp).
 *
 * @param machine The machine to register the services with.
 */
void add_memory_card_services(Machine& machine);

}  // namespace sys
