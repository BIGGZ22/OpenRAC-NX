// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The Emotion Engine's CPU core as an interpreter over guest memory.
 *
 * This file declares `Ee`. The decoding of the MIPS III instructions, the FPU and the vector
 * instructions on VU0 is in `ee.cpp`; the multimedia instructions and the second multiply and
 * divide unit are in `ee_mmi.cpp`.
 *
 * Sources: the MIPS III instruction set and the Emotion Engine's additions as publicly documented.
 */

#pragma once

#include <array>
#include <functional>

#include "memory.h"
#include "types.h"
#include "vu.h"

namespace ps2 {

/**
 * The Emotion Engine's CPU core as the games' code needs it: the MIPS III instruction set with the
 * EE's additions (128-bit registers and the multimedia instructions, a second multiply unit,
 * three-operand multiply), the single-precision FPU, and the vector instructions that run on VU0.
 *
 * It is an interpreter over guest memory. It has no kernel, no TLB and no exception vectors: a
 * SYSCALL goes to `on_syscall`, an access outside main memory and the scratchpad goes to the
 * hardware callbacks, and interrupts are delivered by whoever owns the core calling `call()` on a
 * handler.
 *
 * One thread runs the core. Every callback is called on that thread, from inside `run()` or
 * `call()`.
 */
class Ee {
public:
    /** A 128-bit general register, as two 64-bit halves. */
    struct Reg {
        /** The low and the high 64 bits of the register. */
        u64 lo = 0, hi = 0;
    };

    /**
     * Creates a core in its power-on state.
     *
     * @param memory The guest memory the core reads and writes. The caller keeps it alive.
     * @param vu0 The vector unit that the COP2 instructions drive. The caller keeps it alive.
     */
    Ee(GuestMemory& memory, Vu& vu0);

    /** Puts every register in the state a program finds at start-up and clears the counters. */
    void reset();

    /**
     * Runs until `cycles` reaches `until`, a stop is asked for, or the program returns to the
     * address `call()` set up.
     *
     * @param until The value of `cycles` at which to stop.
     */
    void run(u64 until);

    /**
     * Calls a guest function with up to four integer arguments and runs it to its return, leaving
     * every register as it was. For interrupt handlers and callbacks.
     *
     * @param function Address of the first instruction of the function.
     * @param a0 First argument.
     * @param a1 Second argument.
     * @param a2 Third argument.
     * @param a3 Fourth argument.
     * @return The function's result, from register v0. Counted in `unknown` when it never returns.
     */
    u64 call(u32 function, u64 a0 = 0, u64 a1 = 0, u64 a2 = 0, u64 a3 = 0);

    /**
     * Where the stack of the next `call` starts, or 0 for one below the caller's own.
     *
     * An interrupt handler or a callback that breaks into running code is given a stack of its
     * own, as the console's kernel does: the code it interrupts may be using the stack pointer
     * for something else at that moment (seen in game code). Calls made inside such a call go
     * below the caller's stack again.
     */
    u32 call_stack = 0;

    /** Asks `run()` to return after the instruction that is running. */
    void stop() { stop_ = true; }

    /**
     * Runs the guest function at an address with the registers as they are, until it returns.
     *
     * For host code that stands in for a guest function and calls another one: nothing is saved
     * or put back but the return address and the place to go on at, so the callee's result and
     * everything else it changed are the caller's to see. Timed events are handled as in `run()`;
     * a stop asked for meanwhile is kept for the caller's `run()`.
     *
     * @param function Address of the function's first instruction.
     */
    void run_function(u32 function);

    /** What `begin_call` set aside of the caller, for `end_call` to put back. */
    struct Call {
        u64 ra = 0;             // The caller's return address register.
        u32 pc = 0;             // Where the caller goes on.
        u32 next_pc = 0;        // And the instruction after that.
        bool returned = false;  // The caller's own "returned" flag.
    };

    /**
     * The three steps of `run_function`, for a caller that wants to stop part of the way:
     * `begin_call` points the core at the function, `run_call` runs it for a number of
     * instructions at most and says whether it returned, `end_call` puts the caller back.
     *
     * @param function Address of the function's first instruction.
     * @return What `end_call` needs.
     */
    Call begin_call(u32 function);

    /**
     * Runs the function `begin_call` started.
     *
     * @param instructions The most instructions to run now.
     * @return True when the function has returned (or the program counter left memory).
     */
    bool run_call(u64 instructions);

    /**
     * Puts the caller back after the function returned.
     *
     * @param call What `begin_call` gave.
     */
    void end_call(const Call& call);

    /**
     * Where host code may stand in for guest functions: one byte for each word of main memory,
     * nonzero at a function's first instruction. Null when there is none.
     *
     * When the program counter reaches a marked word, `on_native` is called with the address; if
     * it answers true the function has been run by host code and the EE goes on at the return
     * address. Not owned.
     */
    const u8* native_marks = nullptr;

    /** Runs the host code for the function at an address; false leaves it to the interpreter. */
    std::function<bool(u32 address)> on_native;

    /**
     * Called with the target of every call instruction (JAL, JALR) the interpreter runs, when
     * set: for counting which guest functions a run uses. Empty by default.
     */
    std::function<void(u32 target)> on_call;

    // --- State ---

    /** The 32 general registers, 128 bits each (documented). */
    std::array<Reg, 32> gpr{};

    /** HI and LO of the first multiply unit, and HI1 and LO1 of the second one (documented). */
    u64 hi = 0, lo = 0, hi1 = 0, lo1 = 0;

    /**
     * The address of the instruction to run next, and the address that follows it.
     *
     * A branch changes `next_pc`, so the instruction in its delay slot still runs first
     * (documented).
     */
    u32 pc = 0, next_pc = 4;

    /** The shift amount register SA, used by QFSRV and set by MTSA, MTSAB, MTSAH (documented). */
    u32 sa = 0;

    /** The 32 FPU registers, as the bit patterns of single-precision floats (documented). */
    std::array<u32, 32> fpr{};

    /** The FPU accumulator, and the FPU control and status register FCR31 (documented). */
    u32 facc = 0, fcr31 = 0;

    /** The coprocessor 0 registers by number, for instance 12 Status and 14 EPC (documented). */
    std::array<u32, 32> cop0{};

    /** Time in core cycles; the interpreter adds a fixed number for every instruction. */
    u64 cycles = 0;

    // --- What the core hands out ---

    /**
     * Called for SYSCALL, with the instruction's 20-bit code field. A SYSCALL does nothing when
     * this is empty.
     */
    std::function<void(u32 code)> on_syscall;

    /**
     * Called for a load outside memory, with the physical address and the size of 1, 2, 4 or 8
     * bytes. Returns the value read. The load reads 0 when this is empty.
     */
    std::function<u64(u32 address, unsigned bytes)> on_read;

    /** Called for a store outside memory, by physical address, of 1, 2, 4 or 8 bytes. */
    std::function<void(u32 address, u64 value, unsigned bytes)> on_write;

    /** Called for a quadword written outside memory (the FIFOs), by physical address. */
    std::function<void(u32 address, u64 lo, u64 hi)> on_write128;

    /**
     * Called when `cycles` reaches `event_at`: timers, the display's timing, interrupts. The
     * callee sets the next `event_at`.
     */
    std::function<void()> on_event;

    /** The cycle at which `on_event` is called next. All ones means never. */
    u64 event_at = ~u64{0};

    // --- Memory, as the program sees it ---

    /**
     * Reads a byte of guest memory.
     *
     * @param address The address as the program uses it.
     * @return The byte, or what `on_read` gives for an address outside memory (0 without it).
     */
    u8 read8(u32 address);

    /**
     * Reads a halfword of guest memory.
     *
     * @param address The address as the program uses it.
     * @return The halfword, or what `on_read` gives for an address outside memory (0 without it).
     */
    u16 read16(u32 address);

    /**
     * Reads a word of guest memory.
     *
     * @param address The address as the program uses it.
     * @return The word, or what `on_read` gives for an address outside memory (0 without it).
     */
    u32 read32(u32 address);

    /**
     * Reads a doubleword of guest memory.
     *
     * @param address The address as the program uses it.
     * @return The doubleword, or what `on_read` gives for an address outside memory (0 without it).
     */
    u64 read64(u32 address);

    /**
     * Writes a byte of guest memory, or hands it to `on_write` for an address outside memory.
     *
     * @param address The address as the program uses it.
     * @param value The byte to store.
     */
    void write8(u32 address, u8 value);

    /**
     * Writes a halfword of guest memory, or hands it to `on_write` for an address outside memory.
     *
     * @param address The address as the program uses it.
     * @param value The halfword to store.
     */
    void write16(u32 address, u16 value);

    /**
     * Writes a word of guest memory, or hands it to `on_write` for an address outside memory.
     *
     * @param address The address as the program uses it.
     * @param value The word to store.
     */
    void write32(u32 address, u32 value);

    /**
     * Writes a doubleword of guest memory, or hands it to `on_write` for an address outside memory.
     *
     * @param address The address as the program uses it.
     * @param value The doubleword to store.
     */
    void write64(u32 address, u64 value);

    /**
     * Finds the host memory behind an address.
     *
     * @param address The address as the program uses it.
     * @return A pointer into main memory or the scratchpad, or null for anything else.
     */
    u8* pointer(u32 address);

    /**
     * Gives an integer argument register by its ABI name: a0 is register 4, a1 is register 5
     * (documented).
     *
     * @param n Which argument, 0 for the first.
     * @return The low 64 bits of that register.
     */
    u64& a(unsigned n) { return gpr[4 + n].lo; }

    /** Gives the integer result register v0, which is register 2 (documented). */
    u64& v0() { return gpr[2].lo; }

    /** Gives the low word of the stack pointer, which is register 29 (documented). */
    u32& sp32() { return *reinterpret_cast<u32*>(&gpr[29].lo); }

    /**
     * Sets the integer result register v0.
     *
     * @param value The result, sign-extended to 64 bits.
     */
    void set_result(s64 value) { gpr[2].lo = static_cast<u64>(value); }

    /** Returns from the function the program is at the entry of, to the address in ra. */
    void leave() {
        pc = static_cast<u32>(gpr[31].lo);
        next_pc = pc + 4;
    }

    /** True once the program counter left memory: the program is lost. */
    bool lost = false;

    /**
     * Gives the last jumps (from, to), oldest first, to see how the program got lost.
     *
     * @return The 16 most recent jumps, J, JAL, JR and JALR alike.
     */
    std::array<std::array<u32, 2>, 16> recent_jumps() const;

    /** How many instructions the core does not know. */
    u64 unknown = 0;

    /** The address and the word of the last instruction the core did not know. */
    u32 last_unknown_pc = 0, last_unknown = 0;

    /** How many VU0 microprograms did not stop. */
    u64 vu0_runaways = 0;

    /**
     * Where the first runaway VU0 microprogram started, and the address of the EE instruction that
     * was waiting for it.
     */
    u32 vu0_runaway_start = 0, vu0_runaway_from = 0;

    /** The address a `call()` returns to. Nothing is mapped there. */
    static constexpr u32 kReturnAddress = 0x1FC00FF0;

private:
    /** Runs the instruction at `pc`, or ends the run when `pc` is the return address or lost. */
    void step();

    /**
     * Runs an instruction of the SPECIAL group (opcode 0), chosen by its function field
     * (documented).
     *
     * @param op The instruction word.
     */
    void special(u32 op);

    /**
     * Runs an instruction of the REGIMM group (opcode 1), chosen by its rt field (documented).
     *
     * @param op The instruction word.
     * @param at The address of the instruction.
     */
    void regimm(u32 op, u32 at);

    /**
     * Runs an instruction of the MMI group (opcode 0x1C), chosen by its function field.
     *
     * @param op The instruction word.
     */
    void mmi(u32 op);

    /**
     * Runs an MMI0 instruction (MMI function 0x08), chosen by its shift field (documented).
     *
     * @param op The instruction word.
     */
    void mmi0(u32 op);

    /**
     * Runs an MMI1 instruction (MMI function 0x28), chosen by its shift field (documented).
     *
     * @param op The instruction word.
     */
    void mmi1(u32 op);

    /**
     * Runs an MMI2 instruction (MMI function 0x09), chosen by its shift field (documented).
     *
     * @param op The instruction word.
     */
    void mmi2(u32 op);

    /**
     * Runs an MMI3 instruction (MMI function 0x29), chosen by its shift field (documented).
     *
     * @param op The instruction word.
     */
    void mmi3(u32 op);

    /**
     * Runs an instruction of coprocessor 0 (opcode 0x10), chosen by its rs field (documented).
     *
     * @param op The instruction word.
     */
    void cop0_op(u32 op);

    /**
     * Runs an instruction of coprocessor 1, the FPU (opcode 0x11), chosen by its rs field
     * (documented).
     *
     * @param op The instruction word.
     * @param at The address of the instruction.
     */
    void cop1_op(u32 op, u32 at);

    /**
     * Runs an instruction of coprocessor 2, which is VU0 (opcode 0x12).
     *
     * @param op The instruction word.
     * @param at The address of the instruction.
     */
    void cop2_op(u32 op, u32 at);

    /**
     * Brings VU0 up to date with the EE.
     *
     * VU0 runs beside the EE once a microprogram is started. It is advanced when the EE next
     * touches it: by as many instructions as the EE has run since (one VU instruction to an EE
     * instruction), or further when the EE's instruction is one that waits.
     */
    void vu0_sync();

    /**
     * Lets a running VU0 microprogram run to its end, up to a limit.
     *
     * A microprogram that does not stop within the limit is counted in `vu0_runaways`.
     *
     * @param at The address of the EE instruction that waits, noted for the first runaway.
     */
    void vu0_finish(u32 at);

    /**
     * Sets the FCR31 overflow and underflow flags and their sticky copies from an operation's
     * result (documented).
     *
     * @param problems The `fp::` flags the operation reported.
     */
    void fpu_flags(u32 problems);

    /**
     * Counts an instruction the core does not know and notes which one.
     *
     * @param op The instruction word.
     * @param at The address of the instruction.
     */
    void not_known(u32 op, u32 at);

    /**
     * Carries out a conditional branch.
     *
     * @param taken Whether the condition holds.
     * @param at The address of the branch instruction.
     * @param offset The instruction's immediate, a signed distance in instructions from the delay
     * slot.
     * @param likely True for the "likely" forms, which skip the delay slot when not taken
     * (documented).
     */
    void branch(bool taken, u32 at, s32 offset, bool likely);

    /**
     * Writes a 32-bit result to a register, sign-extended to 64 bits. Register 0 stays zero
     * (documented).
     *
     * @param reg The register number.
     * @param value The result.
     */
    void set32(unsigned reg, u32 value) {
        // A write to register 0 is dropped: it always reads zero (documented).
        if (reg) {
            gpr[reg].lo = static_cast<u64>(static_cast<s64>(static_cast<s32>(value)));
        }
    }

    /**
     * Writes a 64-bit result to a register. Register 0 stays zero (documented).
     *
     * @param reg The register number.
     * @param value The result.
     */
    void set64(unsigned reg, u64 value) {
        // A write to register 0 is dropped: it always reads zero (documented).
        if (reg) {
            gpr[reg].lo = value;
        }
    }

    /** The guest memory the core reads and writes. Owned by the caller. */
    GuestMemory& memory_;

    /** The vector unit the COP2 instructions drive. Owned by the caller. */
    Vu& vu0_;

    /**
     * Whether `stop()` was called, and whether the program returned to `kReturnAddress`. Both end
     * `run()`.
     */
    bool stop_ = false, returned_ = false;

    /** The last 16 jumps as (from, to), a ring written at `jump_next_`. */
    std::array<std::array<u32, 2>, 16> jumps_{};

    /** The next slot of `jumps_` to write, counting without a limit. */
    unsigned jump_next_ = 0;

    /**
     * Records a jump in the ring of recent ones.
     *
     * @param from The address of the jump instruction.
     * @param to The address it jumps to.
     */
    void note_jump(u32 from, u32 to) { jumps_[jump_next_++ & 15] = {from, to}; }

    /** The value `cycles` had when VU0 was last brought up to date. */
    u64 vu0_cycles_ = 0;

    /** The program address of the last VU0 microprogram that was started. */
    u32 vu0_started_at_ = 0;
};

}  // namespace ps2
