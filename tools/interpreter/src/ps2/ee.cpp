// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The Emotion Engine's CPU core: fetch, decode and run of the MIPS III instructions, COP0, the FPU
 * and the COP2 instructions that drive VU0.
 *
 * This file implements `Ee` (declared in `ee.h`) except the multimedia instructions, which are in
 * `ee_mmi.cpp`. There is no pipeline, cache or TLB: each instruction runs to its end before the
 * next one, and a delay slot is modelled by `next_pc`.
 *
 * Sources: the MIPS III instruction set and the Emotion Engine's additions as publicly documented.
 */

#include "ee.h"

#include "fp.h"

namespace ps2 {
namespace {

/** Cycles the interpreter charges for every instruction, whatever it is. */
constexpr u64 kCyclesPerInstruction = 2;

/**
 * Extracts the rs field, the first register operand, of an instruction word (documented).
 *
 * @param op The instruction word.
 * @return The register number.
 */
inline unsigned rs_of(u32 op) {
    return (op >> 21) & 31;  // RS, bits 21-25
}

/**
 * Extracts the rt field, the second register operand, of an instruction word (documented).
 *
 * @param op The instruction word.
 * @return The register number.
 */
inline unsigned rt_of(u32 op) {
    return (op >> 16) & 31;  // RT, bits 16-20
}

/**
 * Extracts the rd field, the destination register, of an instruction word (documented).
 *
 * @param op The instruction word.
 * @return The register number.
 */
inline unsigned rd_of(u32 op) {
    return (op >> 11) & 31;  // RD, bits 11-15
}

/**
 * Extracts the shift amount field of an instruction word (documented).
 *
 * @param op The instruction word.
 * @return The shift amount.
 */
inline unsigned sa_of(u32 op) {
    return (op >> 6) & 31;  // SA, bits 6-10
}

/**
 * Extracts the immediate field of an instruction word, sign-extended (documented).
 *
 * @param op The instruction word.
 * @return The immediate as a signed number.
 */
inline s32 imm_of(u32 op) {
    return static_cast<s16>(op & 0xFFFF);  // IMMEDIATE, bits 0-15
}

/**
 * Sign-extends a 32-bit word to 64 bits, as the EE does when it keeps a word result.
 *
 * @param v The word.
 * @return The word's value as a signed 64-bit number, as bits.
 */
inline u64 sext32(u32 v) {
    return static_cast<u64>(static_cast<s64>(static_cast<s32>(v)));
}

/**
 * The FCR31 condition bit C, and the flag bits I, D, O and U: invalid, divide by zero, overflow,
 * underflow (documented).
 */
constexpr u32 kC = 1u << 23, kI = 1u << 17, kD = 1u << 16, kO = 1u << 15, kU = 1u << 14;

/**
 * The sticky copies of the FCR31 flags I, D, O and U, which stay set until a program clears them
 * (documented).
 */
constexpr u32 kSI = 1u << 6, kSD = 1u << 5, kSO = 1u << 4, kSU = 1u << 3;

}  // namespace

Ee::Ee(GuestMemory& memory, Vu& vu0) : memory_(memory), vu0_(vu0) {
    reset();
}

void Ee::reset() {
    // Power-on state: everything zero, with the next instruction at address 0.
    gpr = {};
    hi = lo = hi1 = lo1 = 0;
    pc = 0;
    next_pc = 4;
    sa = 0;
    fpr = {};
    facc = 0;

    // FCR31 reads with bits 24 and 0 set, and CTC1 never clears them (assumed).
    fcr31 = 0x01000001;
    cop0 = {};

    // Status as a program finds it: interrupts on.
    cop0[12] = 0x70030C13;
    cop0[15] = 0x2E20;  // PRId
    cycles = 0;
    stop_ = returned_ = false;
}

// --- Memory ---

u8* Ee::pointer(u32 address) {
    // The scratchpad is the region whose top four address bits are 7 (documented).
    if ((address >> 28) == 7) {
        return memory_.scratchpad(address);
    }

    // The top three bits only choose how the same physical memory is cached (documented).
    u32 physical = address & 0x1FFFFFFFu;

    // Main memory starts at physical address 0.
    if (physical < GuestMemory::kRamBytes) {
        return memory_.ram(physical);
    }

    return nullptr;
}

u8 Ee::read8(u32 address) {
    // Memory answers directly.
    if (u8* p = pointer(address)) {
        return *p;
    }

    // Anything else is hardware: its callback gets the physical address and the size in bytes.
    return on_read ? static_cast<u8>(on_read(address & 0x1FFFFFFFu, 1)) : 0;
}

u16 Ee::read16(u32 address) {
    // Memory answers directly.
    if (u8* p = pointer(address)) {
        return load<u16>(p);
    }

    // Anything else is hardware: its callback gets the physical address and the size in bytes.
    return on_read ? static_cast<u16>(on_read(address & 0x1FFFFFFFu, 2)) : 0;
}

u32 Ee::read32(u32 address) {
    // Memory answers directly.
    if (u8* p = pointer(address)) {
        return load<u32>(p);
    }

    // Anything else is hardware: its callback gets the physical address and the size in bytes.
    return on_read ? static_cast<u32>(on_read(address & 0x1FFFFFFFu, 4)) : 0;
}

u64 Ee::read64(u32 address) {
    // Memory answers directly.
    if (u8* p = pointer(address)) {
        return load<u64>(p);
    }

    // Anything else is hardware: its callback gets the physical address and the size in bytes.
    return on_read ? on_read(address & 0x1FFFFFFFu, 8) : 0;
}

void Ee::write8(u32 address, u8 value) {
    // Memory takes the store directly.
    if (u8* p = pointer(address)) {
        *p = value;
    } else if (on_write) {
        // Anything else is hardware: its callback gets the physical address and the size in bytes.
        on_write(address & 0x1FFFFFFFu, value, 1);
    }
}

void Ee::write16(u32 address, u16 value) {
    // Memory takes the store directly.
    if (u8* p = pointer(address)) {
        store<u16>(p, value);
    } else if (on_write) {
        // Anything else is hardware: its callback gets the physical address and the size in bytes.
        on_write(address & 0x1FFFFFFFu, value, 2);
    }
}

void Ee::write32(u32 address, u32 value) {
    // Memory takes the store directly.
    if (u8* p = pointer(address)) {
        store<u32>(p, value);
    } else if (on_write) {
        // Anything else is hardware: its callback gets the physical address and the size in bytes.
        on_write(address & 0x1FFFFFFFu, value, 4);
    }
}

void Ee::write64(u32 address, u64 value) {
    // Memory takes the store directly.
    if (u8* p = pointer(address)) {
        store<u64>(p, value);
    } else if (on_write) {
        // Anything else is hardware: its callback gets the physical address and the size in bytes.
        on_write(address & 0x1FFFFFFFu, value, 8);
    }
}

// --- Running ---

void Ee::run(u64 until) {
    stop_ = false;

    // Ends when a stop is asked for, the call returns, or the cycle budget is spent.
    while (!stop_ && !returned_) {
        // A timed event is due: the owner handles it before any further instruction runs.
        if (cycles >= event_at) {
            // With no handler the event is dropped, so it does not fire again.
            if (on_event) {
                on_event();
            } else {
                event_at = ~u64{0};
            }

            continue;
        }

        // The budget is spent.
        if (cycles >= until) {
            break;
        }

        // Run no further than the budget or the next event, whichever comes first.
        u64 limit = until < event_at ? until : event_at;

        // One instruction per pass, until the limit, a stop request or the return to the caller.
        while (cycles < limit && !stop_ && !returned_) {
            step();
        }
    }
}

u64 Ee::call(u32 function, u64 a0, u64 a1, u64 a2, u64 a3) {
    /** Everything a call changes and puts back afterwards. */
    struct Saved {
        std::array<Reg, 32> gpr;           // The general registers.
        u64 hi, lo, hi1, lo1;              // The multiply results of both units.
        u32 pc, next_pc, sa, facc, fcr31;  // The program counter, SA and the FPU state.
        std::array<u32, 32> fpr;           // The FPU registers.
        bool stop, returned;               // The flags that end `run()`.
    } saved{gpr, hi, lo, hi1, lo1, pc, next_pc, sa, facc, fcr31, fpr, stop_, returned_};

    // The arguments go in a0 to a3, which are registers 4 to 7 (documented).
    gpr[4].lo = a0;
    gpr[5].lo = a1;
    gpr[6].lo = a2;
    gpr[7].lo = a3;

    // Returning to this address is how `step()` sees that the function is done.
    gpr[31].lo = kReturnAddress;

    // The call's stack: the one set aside for it, or 0x400 bytes below the caller's, 16-aligned.
    u32 own_stack = call_stack;
    u32 stack = own_stack ? own_stack : static_cast<u32>(gpr[29].lo) - 0x400;

    gpr[29].lo = sext32(stack & ~0xFu);

    // Calls made by the function itself must not start on the same stack.
    call_stack = 0;

    // The function's first instruction is next, and the one after it is 4 bytes on.
    pc = function;
    next_pc = function + 4;
    returned_ = false;

    // A handler that never returns is a bug in the model; allow it 2^28 instructions.
    u64 guard = u64{1} << 28;

    // One instruction per pass, until the handler returns or the guard runs out.
    while (!returned_ && guard--) {
        step();
    }

    // It never returned: count it as an unknown instruction at its entry, with a marker word.
    if (!returned_) {
        unknown++;
        last_unknown_pc = function;
        last_unknown = 0xCA11CA11;
    }

    // The result is in v0, register 2 (documented), and must be read before the registers go back.
    u64 result = gpr[2].lo;

    // Put every register back as the interrupted code left it.
    gpr = saved.gpr;
    hi = saved.hi;
    lo = saved.lo;
    hi1 = saved.hi1;
    lo1 = saved.lo1;
    pc = saved.pc;
    next_pc = saved.next_pc;
    sa = saved.sa;
    facc = saved.facc;
    fcr31 = saved.fcr31;
    fpr = saved.fpr;
    stop_ = saved.stop;
    returned_ = saved.returned;

    return result;
}

void Ee::run_function(u32 function) {
    Call call = begin_call(function);

    // No limit: until it returns.
    run_call(~u64{0});
    end_call(call);
}

Ee::Call Ee::begin_call(u32 function) {
    Call call;

    call.ra = gpr[31].lo;
    call.pc = pc;
    call.next_pc = next_pc;
    call.returned = returned_;

    // Returning to this address is how `step()` sees that the function is done.
    gpr[31].lo = kReturnAddress;
    pc = function;
    next_pc = function + 4;
    returned_ = false;

    return call;
}

bool Ee::run_call(u64 instructions) {
    // Ends when the function returns, the program counter leaves memory, or the count is spent.
    while (!returned_ && !lost && instructions != 0) {
        // A timed event is due: the owner handles it before any further instruction runs.
        if (cycles >= event_at) {
            // With no handler the event is dropped, so it does not fire again.
            if (on_event) {
                on_event();
            } else {
                event_at = ~u64{0};
            }

            continue;
        }

        step();
        instructions--;
    }

    return returned_ || lost;
}

void Ee::end_call(const Call& call) {
    gpr[31].lo = call.ra;
    pc = call.pc;
    next_pc = call.next_pc;
    returned_ = call.returned;
}

std::array<std::array<u32, 2>, 16> Ee::recent_jumps() const {
    std::array<std::array<u32, 2>, 16> out{};

    // The ring has 16 slots and `jump_next_` points at the oldest, so reading from there gives
    // the jumps oldest first.
    for (unsigned n = 0; n < 16; n++) {
        out[n] = jumps_[(jump_next_ + n) & 15];
    }

    return out;
}

void Ee::not_known(u32 op, u32 at) {
    unknown++;
    last_unknown = op;
    last_unknown_pc = at;
}

void Ee::branch(bool taken, u32 at, s32 offset, bool likely) {
    // The target is the delay slot's address plus the offset counted in 4-byte instructions
    // (documented).
    if (taken) {
        next_pc = at + 4 + static_cast<u32>(offset << 2);
    } else if (likely) {
        // The instruction in the delay slot is not run.
        pc = next_pc;
        next_pc += 4;
    }
}

void Ee::step() {
    u32 at = pc;

    // The call's return address is not code: reaching it ends the call.
    if (at == kReturnAddress) {
        returned_ = true;
        return;
    }

    // Host code stands in for the function that starts here: go on at its return address.
    if (native_marks && native_marks[(at & (GuestMemory::kRamBytes - 1)) >> 2] && on_native(at)) {
        pc = static_cast<u32>(gpr[31].lo);
        next_pc = pc + 4;
        return;
    }

    u32 op;

    // An instruction is fetched from memory only; anywhere else the program is lost.
    if (const u8* p = pointer(at)) {
        op = load<u32>(p);
    } else {
        not_known(0, at);
        lost = true;
        stop_ = true;
        return;
    }

    // Step past this 4-byte instruction first; a branch then changes next_pc and the delay slot
    // still runs (documented).
    pc = next_pc;
    next_pc += 4;
    cycles += kCyclesPerInstruction;

    // The fields most opcodes share, and rs plus the immediate, the address of loads and stores
    // (documented).
    unsigned rs = rs_of(op), rt = rt_of(op);
    s32 imm = imm_of(op);
    u32 address = static_cast<u32>(gpr[rs].lo) + static_cast<u32>(imm);

    // The opcode is bits 26-31 (documented).
    switch (op >> 26) {
        case 0x00:  // SPECIAL
            special(op);
            break;

        case 0x01:  // REGIMM
            regimm(op, at);
            break;

        case 0x02:  // J
            // The 26-bit target counts instructions and keeps the top four bits of the delay
            // slot's address (documented).
            next_pc = ((at + 4) & 0xF0000000u) | ((op & 0x03FFFFFFu) << 2);
            note_jump(at, next_pc);
            break;

        case 0x03:  // JAL
            // The link is the address after the delay slot, 8 bytes on (documented).
            gpr[31].lo = at + 8;
            next_pc = ((at + 4) & 0xF0000000u) | ((op & 0x03FFFFFFu) << 2);
            note_jump(at, next_pc);

            // Somebody counts calls.
            if (on_call) {
                on_call(next_pc);
            }

            break;

        case 0x04:  // BEQ
            branch(gpr[rs].lo == gpr[rt].lo, at, imm, false);
            break;

        case 0x05:  // BNE
            branch(gpr[rs].lo != gpr[rt].lo, at, imm, false);
            break;

        case 0x06:  // BLEZ
            branch(static_cast<s64>(gpr[rs].lo) <= 0, at, imm, false);
            break;

        case 0x07:  // BGTZ
            branch(static_cast<s64>(gpr[rs].lo) > 0, at, imm, false);
            break;

        case 0x08:  // ADDI
        case 0x09:  // ADDIU
            // ADDI runs as ADDIU: the core has no overflow exception.
            set32(rt, static_cast<u32>(gpr[rs].lo) + static_cast<u32>(imm));
            break;

        case 0x0A:  // SLTI
            set64(rt, static_cast<s64>(gpr[rs].lo) < imm);
            break;

        case 0x0B:  // SLTIU
            set64(rt, gpr[rs].lo < static_cast<u64>(static_cast<s64>(imm)));
            break;

        case 0x0C:  // ANDI
            // The immediate of the logical operations is zero-extended (documented).
            set64(rt, gpr[rs].lo & (op & 0xFFFF));
            break;

        case 0x0D:  // ORI
            // The immediate is zero-extended (documented).
            set64(rt, gpr[rs].lo | (op & 0xFFFF));
            break;

        case 0x0E:  // XORI
            // The immediate is zero-extended (documented).
            set64(rt, gpr[rs].lo ^ (op & 0xFFFF));
            break;

        case 0x0F:  // LUI
            // The immediate becomes the upper halfword of a word that is then sign-extended
            // (documented).
            set32(rt, (op & 0xFFFF) << 16);
            break;

        case 0x10:  // COP0
            cop0_op(op);
            break;

        case 0x11:  // COP1
            cop1_op(op, at);
            break;

        case 0x12:  // COP2
            cop2_op(op, at);
            break;

        case 0x14:  // BEQL
            branch(gpr[rs].lo == gpr[rt].lo, at, imm, true);
            break;

        case 0x15:  // BNEL
            branch(gpr[rs].lo != gpr[rt].lo, at, imm, true);
            break;

        case 0x16:  // BLEZL
            branch(static_cast<s64>(gpr[rs].lo) <= 0, at, imm, true);
            break;

        case 0x17:  // BGTZL
            branch(static_cast<s64>(gpr[rs].lo) > 0, at, imm, true);
            break;

        case 0x18:  // DADDI
        case 0x19:  // DADDIU
            // DADDI runs as DADDIU: the core has no overflow exception.
            set64(rt, gpr[rs].lo + static_cast<u64>(static_cast<s64>(imm)));
            break;

        case 0x1A: {  // LDL
            /*
             * Loads the bytes up to the address, within its aligned doubleword, into the top of rt.
             * The low n bytes of rt stay (documented).
             */
            unsigned n = 7 - (address & 7);
            u64 mem = read64(address & ~7u);
            set64(rt, n ? (mem << (8 * n)) | (gpr[rt].lo & ((u64{1} << (8 * n)) - 1)) : mem);
            break;
        }

        case 0x1B: {  // LDR
            /*
             * Loads the bytes from the address to the end of its aligned doubleword into the
             * bottom of rt. The top s bytes of rt stay (documented).
             */
            unsigned s = address & 7;
            u64 mem = read64(address & ~7u);
            set64(rt, s ? (mem >> (8 * s)) | (gpr[rt].lo & ~(~u64{0} >> (8 * s))) : mem);
            break;
        }

        case 0x1C:  // MMI
            mmi(op);
            break;

        case 0x1E: {  // LQ
            // A quadword access ignores the low four address bits (documented).
            u32 a = address & ~15u;

            // Register 0 is never written.
            if (rt) {
                gpr[rt].lo = read64(a);
                gpr[rt].hi = read64(a + 8);
            }

            break;
        }

        case 0x1F: {  // SQ
            // A quadword access ignores the low four address bits (documented).
            u32 a = address & ~15u;

            // Memory takes two doublewords; hardware that has a quadword callback gets it whole.
            if (pointer(a) || !on_write128) {
                write64(a, gpr[rt].lo);
                write64(a + 8, gpr[rt].hi);
            } else {
                on_write128(a & 0x1FFFFFFFu, gpr[rt].lo, gpr[rt].hi);
            }

            break;
        }

        case 0x20:  // LB
            set64(rt, static_cast<u64>(static_cast<s64>(static_cast<s8>(read8(address)))));
            break;

        case 0x21:  // LH
            set64(rt, static_cast<u64>(static_cast<s64>(static_cast<s16>(read16(address)))));
            break;

        case 0x22: {  // LWL
            /*
             * Loads the bytes up to the address, within its aligned word, into the top of the low
             * word of rt. The low n bytes of rt stay (documented).
             */
            unsigned n = 3 - (address & 3);
            u32 mem = read32(address & ~3u), old = static_cast<u32>(gpr[rt].lo);
            set32(rt, n ? (mem << (8 * n)) | (old & ((1u << (8 * n)) - 1)) : mem);
            break;
        }

        case 0x23:  // LW
            set32(rt, read32(address));
            break;

        case 0x24:  // LBU
            set64(rt, read8(address));
            break;

        case 0x25:  // LHU
            set64(rt, read16(address));
            break;

        case 0x26: {  // LWR
            /*
             * Loads the bytes from the address to the end of its aligned word into the bottom of
             * the low word of rt. The top s bytes of that word stay (documented).
             */
            unsigned s = address & 3;
            u32 mem = read32(address & ~3u), old = static_cast<u32>(gpr[rt].lo);

            // A word-aligned address loads the whole word, sign-extended (documented).
            if (s == 0) {
                set32(rt, mem);
            } else if (rt) {
                // Only the low word changes when the load is partial.
                u32 merged = (mem >> (8 * s)) | (old & ~(0xFFFFFFFFu >> (8 * s)));
                gpr[rt].lo = (gpr[rt].lo & 0xFFFFFFFF00000000ull) | merged;
            }

            break;
        }

        case 0x27:  // LWU
            set64(rt, read32(address));
            break;

        case 0x28:  // SB
            write8(address, static_cast<u8>(gpr[rt].lo));
            break;

        case 0x29:  // SH
            write16(address, static_cast<u16>(gpr[rt].lo));
            break;

        case 0x2A: {  // SWL
            /*
             * Stores the top bytes of rt into the bytes from the start of the aligned word up to
             * the address. The rest of the word stays (documented).
             */
            unsigned n = 3 - (address & 3);
            u32 mem = read32(address & ~3u), value = static_cast<u32>(gpr[rt].lo);
            write32(
                address & ~3u, n ? (value >> (8 * n)) | (mem & ~(0xFFFFFFFFu >> (8 * n))) : value
            );
            break;
        }

        case 0x2B:  // SW
            write32(address, static_cast<u32>(gpr[rt].lo));
            break;

        case 0x2C: {  // SDL
            /*
             * Stores the top bytes of rt into the bytes from the start of the aligned doubleword
             * up to the address. The rest stays (documented).
             */
            unsigned n = 7 - (address & 7);
            u64 mem = read64(address & ~7u);
            write64(
                address & ~7u,
                n ? (gpr[rt].lo >> (8 * n)) | (mem & ~(~u64{0} >> (8 * n))) : gpr[rt].lo
            );
            break;
        }

        case 0x2D: {  // SDR
            /*
             * Stores the low bytes of rt from the address to the end of its aligned doubleword.
             * The rest stays (documented).
             */
            unsigned s = address & 7;
            u64 mem = read64(address & ~7u);
            write64(
                address & ~7u,
                s ? (gpr[rt].lo << (8 * s)) | (mem & ((u64{1} << (8 * s)) - 1)) : gpr[rt].lo
            );
            break;
        }

        case 0x2E: {  // SWR
            /*
             * Stores the low bytes of rt from the address to the end of its aligned word. The rest
             * stays (documented).
             */
            unsigned s = address & 3;
            u32 mem = read32(address & ~3u), value = static_cast<u32>(gpr[rt].lo);
            write32(address & ~3u, s ? (value << (8 * s)) | (mem & ((1u << (8 * s)) - 1)) : value);
            break;
        }

        case 0x2F:  // CACHE
        case 0x33:  // PREF
            // No cache is modelled, so these do nothing.
            break;

        case 0x31:  // LWC1
            fpr[rt] = read32(address);
            break;

        case 0x39:  // SWC1
            write32(address, fpr[rt]);
            break;

        case 0x36: {  // LQC2
            // A quadword access ignores the low four address bits (documented).
            u32 a = address & ~15u;

            // VU0 must be up to date before its registers change.
            vu0_sync();

            // The four words of the quadword are 4 bytes apart; register 0 is never written.
            if (rt) {
                vu0_.vf[rt] = {read32(a), read32(a + 4), read32(a + 8), read32(a + 12)};
            }

            break;
        }

        case 0x3E: {  // SQC2
            // A quadword access ignores the low four address bits (documented).
            u32 a = address & ~15u;

            // VU0 must be up to date before its registers are read.
            vu0_sync();

            // The four words of the register go to the quadword, 4 bytes apart.
            for (unsigned f = 0; f < 4; f++) {
                write32(a + f * 4, vu0_.vf[rt][f]);
            }

            break;
        }

        case 0x37:  // LD
            set64(rt, read64(address));
            break;

        case 0x3F:  // SD
            write64(address, gpr[rt].lo);
            break;

        default:
            // An opcode the core does not know is counted.
            not_known(op, at);
            break;
    }
}

// --- SPECIAL and REGIMM ---

void Ee::special(u32 op) {
    // The register fields and the shift amount (documented).
    unsigned rs = rs_of(op), rt = rt_of(op), rd = rd_of(op), shift = sa_of(op);

    // The 64-bit values of rs and rt, and their low words.
    u64 s = gpr[rs].lo, t = gpr[rt].lo;
    u32 s32v = static_cast<u32>(s), t32v = static_cast<u32>(t);

    // This instruction's own address: pc already points past it.
    u32 at = pc - 4;

    // The function field, bits 0-5, chooses the instruction (documented).
    switch (op & 0x3F) {
        case 0x00:  // SLL
            set32(rd, t32v << shift);
            break;

        case 0x02:  // SRL
            set32(rd, t32v >> shift);
            break;

        case 0x03:  // SRA
            set32(rd, static_cast<u32>(static_cast<s32>(t32v) >> shift));
            break;

        case 0x04:  // SLLV
            // A word shift by a register uses the low five bits of that register (documented).
            set32(rd, t32v << (s32v & 31));
            break;

        case 0x06:  // SRLV
            // As SLLV, shifting right.
            set32(rd, t32v >> (s32v & 31));
            break;

        case 0x07:  // SRAV
            // As SLLV, shifting right and keeping the sign.
            set32(rd, static_cast<u32>(static_cast<s32>(t32v) >> (s32v & 31)));
            break;

        case 0x08:  // JR
            // The delay slot still runs before the jump lands (documented).
            next_pc = s32v;
            note_jump(at, s32v);
            break;

        case 0x09:  // JALR
            // The link is the address after the delay slot, 8 bytes on (documented).
            next_pc = s32v;
            note_jump(at, s32v);
            set64(rd, at + 8);

            // Somebody counts calls.
            if (on_call) {
                on_call(next_pc);
            }

            break;

        case 0x0A:  // MOVZ
            // The move happens only when rt is zero; otherwise rd stays (documented).
            if (t == 0) {
                set64(rd, s);
            }

            break;

        case 0x0B:  // MOVN
            // The move happens only when rt is not zero; otherwise rd stays (documented).
            if (t != 0) {
                set64(rd, s);
            }

            break;

        case 0x0C:  // SYSCALL
            // Without a handler the call does nothing. The code field is bits 6-25 (documented).
            if (on_syscall) {
                on_syscall((op >> 6) & 0xFFFFF);
            }

            break;

        case 0x0D:  // BREAK
            // The compiler's divide-by-zero trap; the game never survives one (assumed).
            not_known(op, at);
            break;

        case 0x0F:  // SYNC
            // Instructions run one at a time here, so there is nothing to order.
            break;

        case 0x10:  // MFHI
            set64(rd, hi);
            break;

        case 0x11:  // MTHI
            hi = s;
            break;

        case 0x12:  // MFLO
            set64(rd, lo);
            break;

        case 0x13:  // MTLO
            lo = s;
            break;

        case 0x14:  // DSLLV
            // A doubleword shift by a register uses the low six bits of that register (documented).
            set64(rd, t << (s & 63));
            break;

        case 0x16:  // DSRLV
            // As DSLLV, shifting right.
            set64(rd, t >> (s & 63));
            break;

        case 0x17:  // DSRAV
            // As DSLLV, shifting right and keeping the sign.
            set64(rd, static_cast<u64>(static_cast<s64>(t) >> (s & 63)));
            break;

        case 0x18: {  // MULT
            // The product's two words go to HI and LO, and the EE also writes LO to rd
            // (documented).
            s64 product = static_cast<s64>(static_cast<s32>(s32v)) * static_cast<s32>(t32v);
            lo = sext32(static_cast<u32>(product));
            hi = sext32(static_cast<u32>(product >> 32));
            set64(rd, lo);
            break;
        }

        case 0x19: {  // MULTU
            // As MULT with unsigned operands.
            u64 product = static_cast<u64>(s32v) * t32v;
            lo = sext32(static_cast<u32>(product));
            hi = sext32(static_cast<u32>(product >> 32));
            set64(rd, lo);
            break;
        }

        case 0x1A: {  // DIV
            s32 n = static_cast<s32>(s32v), d = static_cast<s32>(t32v);

            // A zero divisor does not trap: the quotient is 1 or -1, the remainder the numerator
            // (assumed).
            if (d == 0) {
                lo = sext32(n < 0 ? 1u : 0xFFFFFFFFu);
                hi = sext32(static_cast<u32>(n));
            } else if (n == INT32_MIN && d == -1) {
                // The one quotient that does not fit in 32 bits gives itself and a remainder of 0
                // (assumed).
                lo = sext32(0x80000000u);
                hi = 0;
            } else {
                lo = sext32(static_cast<u32>(n / d));
                hi = sext32(static_cast<u32>(n % d));
            }

            break;
        }

        case 0x1B:  // DIVU
            // A zero divisor does not trap: the quotient is all ones, the remainder the numerator
            // (assumed).
            if (t32v == 0) {
                lo = sext32(0xFFFFFFFFu);
                hi = sext32(s32v);
            } else {
                lo = sext32(s32v / t32v);
                hi = sext32(s32v % t32v);
            }

            break;

        case 0x20:  // ADD
        case 0x21:  // ADDU
            // ADD runs as ADDU: the core has no overflow exception.
            set32(rd, s32v + t32v);
            break;

        case 0x22:  // SUB
        case 0x23:  // SUBU
            // SUB runs as SUBU: the core has no overflow exception.
            set32(rd, s32v - t32v);
            break;

        case 0x24:  // AND
            set64(rd, s & t);
            break;

        case 0x25:  // OR
            set64(rd, s | t);
            break;

        case 0x26:  // XOR
            set64(rd, s ^ t);
            break;

        case 0x27:  // NOR
            set64(rd, ~(s | t));
            break;

        case 0x28:  // MFSA
            set64(rd, sa);
            break;

        case 0x29:  // MTSA
            sa = s32v;
            break;

        case 0x2A:  // SLT
            set64(rd, static_cast<s64>(s) < static_cast<s64>(t));
            break;

        case 0x2B:  // SLTU
            set64(rd, s < t);
            break;

        case 0x2C:  // DADD
        case 0x2D:  // DADDU
            // DADD runs as DADDU: the core has no overflow exception.
            set64(rd, s + t);
            break;

        case 0x2E:  // DSUB
        case 0x2F:  // DSUBU
            // DSUB runs as DSUBU: the core has no overflow exception.
            set64(rd, s - t);
            break;

        case 0x30:  // TGE
        case 0x31:  // TGEU
        case 0x32:  // TLT
        case 0x33:  // TLTU
        case 0x34:  // TEQ
        case 0x36:  // TNE
            // Traps: never taken by working code.
            break;

        case 0x38:  // DSLL
            set64(rd, t << shift);
            break;

        case 0x3A:  // DSRL
            set64(rd, t >> shift);
            break;

        case 0x3B:  // DSRA
            set64(rd, static_cast<u64>(static_cast<s64>(t) >> shift));
            break;

        case 0x3C:  // DSLL32
            // The 32 forms add 32 to the shift amount (documented).
            set64(rd, t << (shift + 32));
            break;

        case 0x3E:  // DSRL32
            // As DSLL32, shifting right.
            set64(rd, t >> (shift + 32));
            break;

        case 0x3F:  // DSRA32
            // As DSLL32, shifting right and keeping the sign.
            set64(rd, static_cast<u64>(static_cast<s64>(t) >> (shift + 32)));
            break;

        default:
            // An instruction the core does not know is counted.
            not_known(op, at);
            break;
    }
}

void Ee::regimm(u32 op, u32 at) {
    unsigned rs = rs_of(op);
    s64 s = static_cast<s64>(gpr[rs].lo);
    s32 imm = imm_of(op);

    // The rt field chooses the instruction in this group (documented).
    switch (rt_of(op)) {
        case 0x00:  // BLTZ
            branch(s < 0, at, imm, false);
            break;

        case 0x01:  // BGEZ
            branch(s >= 0, at, imm, false);
            break;

        case 0x02:  // BLTZL
            branch(s < 0, at, imm, true);
            break;

        case 0x03:  // BGEZL
            branch(s >= 0, at, imm, true);
            break;

        case 0x10:  // BLTZAL
            // The link, 8 bytes on, is written even when the branch is not taken (documented).
            gpr[31].lo = at + 8;
            branch(s < 0, at, imm, false);
            break;

        case 0x11:  // BGEZAL
            // The link is written even when the branch is not taken (documented).
            gpr[31].lo = at + 8;
            branch(s >= 0, at, imm, false);
            break;

        case 0x12:  // BLTZALL
            // The link is written even when the branch is not taken (documented).
            gpr[31].lo = at + 8;
            branch(s < 0, at, imm, true);
            break;

        case 0x13:  // BGEZALL
            // The link is written even when the branch is not taken (documented).
            gpr[31].lo = at + 8;
            branch(s >= 0, at, imm, true);
            break;

        case 0x18:  // MTSAB
            // The shift amount in bytes: the low four bits of rs and of the immediate, xor-ed.
            sa = (static_cast<u32>(gpr[rs].lo) & 0xF) ^ (static_cast<u32>(imm) & 0xF);
            break;

        case 0x19:  // MTSAH
            // In halfwords: the low three bits of rs and of the immediate, xor-ed and doubled.
            sa = ((static_cast<u32>(gpr[rs].lo) & 7) ^ (static_cast<u32>(imm) & 7)) << 1;
            break;

        default:
            // Codes 0x08 to 0x0E are the trap instructions with an immediate (documented).
            if (rt_of(op) >= 0x08 && rt_of(op) <= 0x0E) {
                break;
            }

            not_known(op, at);
            break;
    }
}

// --- COP0 ---

void Ee::cop0_op(u32 op) {
    unsigned rt = rt_of(op), rd = rd_of(op);

    // The rs field chooses the kind of operation (documented).
    switch (rs_of(op)) {
        case 0x00:  // MFC0
            // Register 9 is Count, which reads as the core's cycle counter (documented).
            set32(rt, rd == 9 ? static_cast<u32>(cycles) : cop0[rd]);
            break;

        case 0x04:  // MTC0
            cop0[rd] = static_cast<u32>(gpr[rt].lo);
            break;

        case 0x08:  // BC0
            /*
             * The condition is "no DMA transfer is running" (assumed). Bit 0 of rt selects the
             * true form and bit 1 the likely form (documented).
             */
            branch((rt & 1) != 0, pc - 4, imm_of(op), (rt & 2) != 0);
            break;

        case 0x10:  // CO
            // The function field, bits 0-5, chooses the operation (documented).
            switch (op & 0x3F) {
                case 0x18:  // ERET
                    // Return from the exception: continue at EPC, register 14 (documented).
                    pc = cop0[14];
                    next_pc = pc + 4;
                    break;

                case 0x38:  // EI
                    // Interrupts are enabled by bit 16 of Status, register 12 (documented).
                    cop0[12] |= 0x10000;
                    break;

                case 0x39:  // DI
                    // Interrupts are disabled by clearing bit 16 of Status (documented).
                    cop0[12] &= ~0x10000u;
                    break;

                default:
                    // The TLB instructions: there is no TLB here.
                    break;
            }

            break;

        default:
            // An instruction the core does not know is counted, at the address pc - 4.
            not_known(op, pc - 4);
            break;
    }
}

// --- COP1 ---

void Ee::fpu_flags(u32 problems) {
    // Each operation rewrites the overflow and underflow flags; the sticky copies stay set
    // (documented).
    fcr31 &= ~(kO | kU);

    // Overflow sets its flag and its sticky copy (documented).
    if (problems & fp::kOverflow) {
        fcr31 |= kO | kSO;
    }

    // Underflow sets its flag and its sticky copy (documented).
    if (problems & fp::kUnderflow) {
        fcr31 |= kU | kSU;
    }
}

void Ee::cop1_op(u32 op, u32 at) {
    // The FPU names the register fields ft (bits 16-20), fs (11-15) and fd (6-10) (documented).
    unsigned rt = rt_of(op), fs = rd_of(op), fd = sa_of(op), ft = rt;
    u32 problems = 0;

    // The rs field chooses the move kind or the format (documented).
    switch (rs_of(op)) {
        case 0x00:  // MFC1
            set32(rt, fpr[fs]);
            break;

        case 0x02:  // CFC1
            // Only FCR31 and FCR0 exist: FCR0 reads 0x2E00, the rest read 0 (assumed).
            set32(rt, fs == 31 ? fcr31 : fs == 0 ? 0x2E00u : 0u);
            break;

        case 0x04:  // MTC1
            fpr[fs] = static_cast<u32>(gpr[rt].lo);
            break;

        case 0x06:  // CTC1
            // Only FCR31 can be written; its writable bits take the value and bits 24 and 0 stay
            // set (assumed).
            if (fs == 31) {
                fcr31 = (static_cast<u32>(gpr[rt].lo) & 0x0083C078u) | 0x01000001u;
            }

            break;

        case 0x08:  // BC1F, BC1T and the likely forms
            // Taken when the C bit equals bit 0 of rt; bit 1 of rt selects the likely form
            // (documented).
            branch(((fcr31 & kC) != 0) == ((rt & 1) != 0), at, imm_of(op), (rt & 2) != 0);
            break;

        case 0x10:  // S
            // The single-precision operations, chosen by the function field (documented).
            switch (op & 0x3F) {
                case 0x00:  // ADD.S
                    fpr[fd] = fp::add(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x01:  // SUB.S
                    fpr[fd] = fp::sub(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x02:  // MUL.S
                    fpr[fd] = fp::mul(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x03:  // DIV
                    fpr[fd] = fp::div(fpr[fs], fpr[ft], problems);

                    // The divide flags are rewritten by each divide; the sticky copies stay set
                    // (documented).
                    fcr31 &= ~(kI | kD);

                    // An invalid operation sets its flag and its sticky copy (documented).
                    if (problems & fp::kInvalid) {
                        fcr31 |= kI | kSI;
                    }

                    // A division by zero sets its flag and its sticky copy (documented).
                    if (problems & fp::kDivideByZero) {
                        fcr31 |= kD | kSD;
                    }

                    break;

                case 0x04:  // SQRT of ft
                    fpr[fd] = fp::sqrt(fpr[ft], problems);

                    // The flags are rewritten as for DIV.
                    fcr31 &= ~(kI | kD);

                    // A negative operand is an invalid operation (documented).
                    if (problems & fp::kInvalid) {
                        fcr31 |= kI | kSI;
                    }

                    break;

                case 0x05:  // ABS
                    fpr[fd] = fpr[fs] & ~fp::kSign;

                    // The overflow and underflow flags are cleared, as for any operation (assumed).
                    fcr31 &= ~(kO | kU);
                    break;

                case 0x06:  // MOV
                    fpr[fd] = fpr[fs];
                    break;

                case 0x07:  // NEG
                    fpr[fd] = fpr[fs] ^ fp::kSign;

                    // The overflow and underflow flags are cleared, as for ABS (assumed).
                    fcr31 &= ~(kO | kU);
                    break;

                case 0x16:  // RSQRT: fs / sqrt(ft)
                    fpr[fd] = fp::rsqrt(fpr[fs], fpr[ft], problems);

                    // The flags are rewritten as for DIV.
                    fcr31 &= ~(kI | kD);

                    // An invalid operation sets its flag and its sticky copy (documented).
                    if (problems & fp::kInvalid) {
                        fcr31 |= kI | kSI;
                    }

                    // A division by zero sets its flag and its sticky copy (documented).
                    if (problems & fp::kDivideByZero) {
                        fcr31 |= kD | kSD;
                    }

                    break;

                case 0x18:  // ADDA.S
                    facc = fp::add(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x19:  // SUBA.S
                    facc = fp::sub(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x1A:  // MULA.S
                    facc = fp::mul(fpr[fs], fpr[ft], problems);
                    fpu_flags(problems);
                    break;

                case 0x1C:  // MADD.S
                    fpr[fd] = fp::add(facc, fp::mul(fpr[fs], fpr[ft], problems), problems);
                    fpu_flags(problems);
                    break;

                case 0x1D:  // MSUB.S
                    fpr[fd] = fp::sub(facc, fp::mul(fpr[fs], fpr[ft], problems), problems);
                    fpu_flags(problems);
                    break;

                case 0x1E:  // MADDA.S
                    facc = fp::add(facc, fp::mul(fpr[fs], fpr[ft], problems), problems);
                    fpu_flags(problems);
                    break;

                case 0x1F:  // MSUBA.S
                    facc = fp::sub(facc, fp::mul(fpr[fs], fpr[ft], problems), problems);
                    fpu_flags(problems);
                    break;

                case 0x24:  // CVT.W: towards zero, saturating
                    fpr[fd] = static_cast<u32>(fp::to_int(fpr[fs]));
                    break;

                case 0x28:  // MAX.S
                    fpr[fd] = fp::max(fpr[fs], fpr[ft]);

                    // The overflow and underflow flags are cleared, as for ABS (assumed).
                    fcr31 &= ~(kO | kU);
                    break;

                case 0x29:  // MIN.S
                    fpr[fd] = fp::min(fpr[fs], fpr[ft]);

                    // The overflow and underflow flags are cleared, as for ABS (assumed).
                    fcr31 &= ~(kO | kU);
                    break;

                case 0x30:  // C.F
                    // The condition is never true, so C is cleared.
                    fcr31 &= ~kC;
                    break;

                case 0x32:  // C.EQ
                    // C is set when the operands are equal as sign and magnitude, zeros alike.
                    fcr31 = (fcr31 & ~kC) | (fp::key(fpr[fs]) == fp::key(fpr[ft]) ? kC : 0);
                    break;

                case 0x34:  // C.LT
                    // C is set when fs is less than ft as sign and magnitude.
                    fcr31 = (fcr31 & ~kC) | (fp::key(fpr[fs]) < fp::key(fpr[ft]) ? kC : 0);
                    break;

                case 0x36:  // C.LE
                    // C is set when fs is less than or equal to ft as sign and magnitude.
                    fcr31 = (fcr31 & ~kC) | (fp::key(fpr[fs]) <= fp::key(fpr[ft]) ? kC : 0);
                    break;

                default:
                    // An instruction the core does not know is counted.
                    not_known(op, at);
                    break;
            }

            break;

        case 0x14:  // W: CVT.S
            // The only word-format operation is the conversion to single, function 0x20
            // (documented).
            if ((op & 0x3F) == 0x20) {
                fpr[fd] = fp::from_int(static_cast<s32>(fpr[fs]));
            } else {
                not_known(op, at);
            }

            break;

        default:
            // An instruction the core does not know is counted.
            not_known(op, at);
            break;
    }
}

// --- COP2 ---

void Ee::vu0_sync() {
    // Only a running microprogram needs catching up.
    if (!vu0_.stopped()) {
        vu0_.advance((cycles - vu0_cycles_) / kCyclesPerInstruction);
    }

    vu0_cycles_ = cycles;
}

void Ee::vu0_finish(u32 at) {
    // Nothing is running, so there is nothing to wait for.
    if (vu0_.stopped()) {
        return;
    }

    /*
     * A microprogram is a few thousand instructions at most; one that runs on
     * is waiting for something the EE will never send, or has gone wrong here.
     */
    vu0_.advance(2'000'000);
    vu0_cycles_ = cycles;

    // The microprogram did not stop within the limit.
    if (!vu0_.stopped()) {
        // Only the first runaway is described: where it started and which EE instruction waited.
        if (vu0_runaways++ == 0) {
            vu0_runaway_start = vu0_started_at_;
            vu0_runaway_from = at;
        }
    }
}

void Ee::cop2_op(u32 op, u32 at) {
    unsigned rt = rt_of(op), rd = rd_of(op);

    // Bit 25 is set for an operation on VU0 itself (documented).
    if (op & (1u << 25)) {
        // An operation on VU0 itself: the EE waits for a running microprogram.
        vu0_finish(at);

        // The function field, bits 0-5, chooses the operation (documented).
        switch (op & 0x3F) {
            case 0x38:  // VCALLMS
            case 0x39:  // VCALLMSR
                // VCALLMS starts at the 15-bit address in bits 6-20, VCALLMSR at the one in CMSAR0,
                // register 27 (documented).
                vu0_started_at_ = (op & 0x3F) == 0x38 ? (op >> 6) & 0x7FFF : vu0_.control(27);
                vu0_.start(vu0_started_at_);
                vu0_cycles_ = cycles;
                break;

            default:
                // Every other operation runs on VU0 at once.
                vu0_.macro(op);
                break;
        }

        return;
    }

    /*
     * Moves between the EE and VU0. With the interlock bit, a move from VU0
     * waits for the microprogram to end and a move to VU0 waits for the next
     * point the microprogram marks (its M bit); without it, the move happens
     * while the microprogram runs.
     */
    if (!vu0_.stopped()) {
        unsigned kind = rs_of(op);

        // Bit 0 is the interlock bit; kinds 1 and 2 are the moves from VU0 (documented).
        if ((op & 1) && (kind == 0x01 || kind == 0x02)) {
            vu0_finish(at);
        } else if ((op & 1) && (kind == 0x05 || kind == 0x06)) {
            // Kinds 5 and 6 are the moves to VU0: wait for the next M-bit point (documented).
            vu0_sync();
            vu0_.advance_to_sync(2'000'000);
            vu0_cycles_ = cycles;
        } else {
            // No interlock: the move happens while the microprogram runs.
            vu0_sync();
        }
    }

    // The rs field chooses the move (documented).
    switch (rs_of(op)) {
        case 0x01:  // QMFC2
            // The four words of the float register become the two halves of rt, word 0 lowest.
            if (rt) {
                gpr[rt].lo = vu0_.vf[rd][0] | (static_cast<u64>(vu0_.vf[rd][1]) << 32);
                gpr[rt].hi = vu0_.vf[rd][2] | (static_cast<u64>(vu0_.vf[rd][3]) << 32);
            }

            break;

        case 0x02:  // CFC2
            // Control register 16 is the status flags; reading it tells VU0 that they are looked
            // at.
            if (rd == 16) {
                vu0_.status_was_read();
            }

            set32(rt, vu0_.control(rd));
            break;

        case 0x05:  // QMTC2
            // The halves of rt fill the register's four words; VF0 is constant, so register 0 is
            // skipped (documented).
            if (rd) {
                vu0_.vf[rd] =
                    {static_cast<u32>(gpr[rt].lo),
                     static_cast<u32>(gpr[rt].lo >> 32),
                     static_cast<u32>(gpr[rt].hi),
                     static_cast<u32>(gpr[rt].hi >> 32)};
            }

            break;

        case 0x06:  // CTC2
            vu0_.set_control(rd, static_cast<u32>(gpr[rt].lo));
            break;

        case 0x08:  // BC2
            // The condition is "VU0 is running", and it never is when the EE looks (assumed).
            branch((rt & 1) == 0, at, imm_of(op), (rt & 2) != 0);
            break;

        default:
            // An instruction the core does not know is counted.
            not_known(op, at);
            break;
    }
}

}  // namespace ps2
