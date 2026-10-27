// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * A vector unit as an interpreter: the microprogram memory, the registers and the timing rules a
 * program depends on.
 *
 * It decodes instructions as they run and models no pipeline beyond the rules in the class
 * comment. The fast four-field path of the arithmetic is in fp_quad.h, and the scalar arithmetic
 * is in fp.h. The implementation is in vu.cpp.
 *
 * Sources: the vector unit manual as publicly documented, and what the games' own microprograms
 * rely on.
 */

#pragma once

#include <array>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "types.h"

namespace ps2 {

/**
 * A vector unit running microprograms: VU1 behind VIF1, or VU0 when the EE starts a microprogram on
 * it.
 *
 * It is an interpreter that keeps the timing microprograms depend on (documented):
 * - the upper and lower instruction of a pair see the same state;
 * - a float register written by one instruction can be read four cycles later, and an instruction
 *   that needs it sooner waits;
 * - flags appear four cycles after the instruction that set them;
 * - Q and P arrive when their dividers and function units finish;
 * - XGKICK sends its packet one instruction late.
 *
 * A branch tests the value an integer register had before the instruction right ahead of it,
 * unless that instruction read flags into it or the branch had to wait (measured). Arithmetic is
 * the console's own (fp.h), on raw bit patterns.
 */
class Vu {
public:
    /**
     * The two memories a unit works on.
     *
     * The caller owns both and keeps them alive for as long as the unit exists.
     */
    struct Memory {
        /** Program memory, 8 bytes an instruction. */
        u8* micro = nullptr;

        /** Size of `micro` in bytes, a power of two: 16 KB for VU1, 4 KB for VU0 (documented). */
        u32 micro_bytes = 0;

        /** Data memory, 16 bytes a quadword. */
        u8* data = nullptr;

        /** Size of `data` in bytes, a power of two. */
        u32 data_bytes = 0;
    };

    /**
     * Makes a unit that works on the given memories, in its power-on state.
     *
     * @param memory The program and data memory, owned by the caller.
     */
    explicit Vu(Memory memory);

    /** Puts the registers, the timing state and the program counter back to power-on values. */
    void reset();

    /**
     * Runs from instruction `address` until an instruction with the E bit has finished (with the
     * one after it), or `limit` instructions have run.
     *
     * @param address First instruction to run, counted in instructions from the start of program
     *        memory.
     * @param limit Most instructions to run. The unit stops there if the program has not ended.
     * @return How many ran.
     */
    u64 run(u32 address, u64 limit = u64{1} << 26);

    /**
     * Continues after the last stop.
     *
     * @param limit Most instructions to run.
     * @return How many ran.
     */
    u64 resume(u64 limit = u64{1} << 26);

    /** True once the program has stopped. */
    bool stopped() const { return !running_; }

    /**
     * Skip working out MAC and status flags that no instruction of the loaded programs can read.
     *
     * What a program leaves behind when it ends is always worked out. Safe for VU1, whose flags
     * nothing outside it reads, and for VU0 as long as the EE reads no status flags
     * (`status_was_read`).
     */
    bool skip_unread_flags = false;

    /**
     * Tells the unit that something outside read the status flags.
     *
     * The status flags' remembering bits depend on every instruction: nothing is skipped from now
     * on.
     */
    void status_was_read();

    /** Forgets what was worked out about program memory, after a write to it. */
    void program_changed();

    /**
     * Starts a program running beside the EE, which is how VU0 is used.
     *
     * Start a program, then let it run a number of instructions at a time with `advance`.
     *
     * @param address First instruction to run, counted in instructions from the start of program
     *        memory.
     */
    void start(u32 address);

    /**
     * Runs a started program for a number of instructions.
     *
     * @param instructions Most instructions to run. It stops sooner if the program ends.
     * @return How many ran.
     */
    u64 advance(u64 instructions);

    /**
     * Runs a started program until an instruction with the M bit (a point the program marks for the
     * EE to wait for) or the end.
     *
     * @param limit Most instructions to run.
     * @return How many ran.
     */
    u64 advance_to_sync(u64 limit);

    /**
     * Runs one instruction given by the EE (a COP2 operation).
     *
     * It runs at once and the EE waits for its result, so nothing is left in flight afterwards.
     *
     * @param code The instruction word.
     */
    void macro(u32 code);

    /**
     * Reads a control register, as the EE does with CFC2.
     *
     * The numbers are 0-15 the integer registers, 16 status, 17 MAC, 18 clip, 20 R, 21 I, 22 Q,
     * 26 TPC, 27 CMSAR0, 28 FBRST, 29 VPU-STAT, 31 CMSAR1 (documented).
     *
     * @param reg The control register number.
     * @return Its value; 0 for a number that is not listed.
     */
    u32 control(unsigned reg) const;

    /**
     * Writes a control register, as the EE does with CTC2.
     *
     * @param reg The control register number, as in `control`.
     * @param value The value to write. Fields the hardware does not keep are cut, and a number that
     *        is not writable is ignored.
     */
    void set_control(unsigned reg, u32 value);

    /**
     * Called when an XGKICK packet is due: a GIF packet starts at this quadword of data memory.
     *
     * It runs on the thread that runs the unit.
     */
    std::function<void(u32 quadword)> on_kick;

    /**
     * For looking into a program: called before each pair runs, with the pair's place and its two
     * words.
     *
     * It runs on the thread that runs the unit.
     */
    std::function<void(u32 at, u32 upper, u32 lower)> on_step;

    /**
     * Called by XTOP and XITOP, which read VIF registers: they return the value for the register.
     *
     * They run on the thread that runs the unit. A missing callback reads as 0.
     */
    std::function<u32()> on_top, on_itop;

    /**
     * How much one content of program memory ran: for telling which of a game's programs a unit
     * runs the most (vu_programs.h).
     */
    struct Use {
        /** The program memory as it was. */
        std::vector<u8> micro;

        /** Instruction pairs run, for each 256 pairs (0x800 bytes) of program memory. */
        std::vector<u64> pairs;

        /** Runs started, by the 256 pairs the first instruction is in. */
        std::vector<u64> starts;
    };

    /**
     * Says what ran, one entry for each content program memory has had.
     *
     * The counts are kept with what the unit has worked out about a content, and go with it if
     * that is thrown away (more than 256 contents).
     *
     * @return The entries, in no order.
     */
    std::vector<Use> uses() const;

    /**
     * The float registers. A float register is four raw 32-bit values, x first, so that integers
     * moved through them survive. VF0 is (0, 0, 0, 1).
     */
    std::array<std::array<u32, 4>, 32> vf{};

    /** The integer registers. VI0 is 0. */
    std::array<u16, 16> vi{};

    /** The accumulator, four raw 32-bit values, x first. */
    std::array<u32, 4> acc{};

    /** The Q, P, I and R registers, as raw 32-bit patterns. */
    u32 q = 0, p = 0, i = 0, r = 0;

    /** The MAC, status and clip flags, as instructions read them now. */
    u32 mac = 0, status = 0, clip = 0;

    /** The program counter, in instructions. */
    u32 pc = 0;

    /**
     * Reads one field of a float register.
     *
     * @param reg The float register number, 0 to 31.
     * @param field The field, 0 for x to 3 for w.
     * @return The field as a float.
     */
    float f(unsigned reg, unsigned field) const { return as_float(vf[reg][field]); }

    /**
     * Writes one field of a float register.
     *
     * @param reg The float register number, 0 to 31.
     * @param field The field, 0 for x to 3 for w.
     * @param value The value to store.
     */
    void set_f(unsigned reg, unsigned field, float value) { vf[reg][field] = as_u32(value); }

    /** Counts the instruction words that decode to nothing. */
    u64 unknown_ops = 0;

private:
    /**
     * The three flag registers as one instruction left them, with the time they become visible.
     */
    struct Flags {
        /** The MAC, status and clip flags. */
        u32 mac = 0, status = 0, clip = 0;

        /** The cycle from which instructions see them. */
        u64 at = 0;
    };

    /** The function for one upper instruction (see vu.cpp, at the end). */
    using UpperRun = void (*)(Vu&, u32 code);

    /** The function for one lower instruction, which also gets the pair's place (see vu.cpp). */
    using LowerRun = void (*)(Vu&, u32 code, u32 at);

    /**
     * What a pair reads that can make it wait, and how it is run, worked out once per pair.
     */
    struct Needs {
        /** The function that runs the upper instruction. */
        UpperRun upper_run = nullptr;

        /** The function that runs the lower instruction. */
        LowerRun lower_run = nullptr;

        /** The pair's upper and lower instruction words. */
        u32 up = 0, low = 0;

        /** True once the rest of this entry has been worked out. */
        bool known = false;

        /** How many entries of `reg` and `mask` are in use. */
        u8 count = 0;

        /** The float registers the pair reads that can make it wait, and the fields of each. */
        u8 reg[4] = {0, 0, 0, 0}, mask[4] = {0, 0, 0, 0};

        /**
         * What the pair waits for, or 0 for nothing.
         *
         * 1: the divider must be free, 2: the function unit must be done.
         */
        u8 wait = 0;

        /** Something can read the flags its upper instruction sets. */
        bool flags_wanted = true;

        /** The upper or lower instruction does nothing, so it is not run. */
        bool upper_nop = false, lower_nop = false;

        /**
         * The lower instruction may read or write the float register the upper one writes, so the
         * two have to be kept apart with care.
         */
        bool together = true;

        /**
         * None of the three pairs before it in memory writes a float register it reads: coming to
         * it in a straight line, it never waits for one.
         */
        bool no_wait = false;
    };

    /** The arithmetic of an instruction, as `arith` takes it. */
    enum class Op {
        Add,
        Sub,
        Mul,
        Madd,  // acc + a * b
        Msub   // acc - a * b
    };

    /** Where the second operand of an instruction comes from. */
    enum class From {
        Ft,  // the same field of ft
        Bc,  // one field of ft, chosen by the low two bits of the instruction
        Q,   // the Q register in every field
        I    // the I register in every field
    };

    /**
     * Runs one pair of program memory, and everything that comes due after it.
     *
     * It finds when the pair can start, lets the flags, Q and P that have arrived take effect,
     * runs the two halves, then carries out a branch, a stop or a kick that is due.
     *
     * Hot path: it runs once per pair and is longer than the 60 lines of 8.2; no measurement of
     * splitting it is recorded.
     */
    void step();

    /**
     * Runs an upper instruction given as a word, by its function field.
     *
     * @param code The instruction word.
     */
    void upper(u32 code);

    /**
     * Runs a lower instruction given as a word, by its operation field.
     *
     * @param code The instruction word.
     * @param at The place of its pair in program memory, which a branch counts from.
     */
    void lower(u32 code, u32 at);

    /**
     * Runs an upper instruction of the special group, found by its index.
     *
     * @param code The instruction word.
     */
    void upper_special(u32 code);

    /**
     * Runs a lower instruction of the special group, found by its function field and index.
     *
     * @param code The instruction word.
     */
    void lower_special(u32 code);

    /**
     * The four families of instructions are each written as one switch; the function for an
     * instruction at a known place in program memory is the switch with its selector fixed (see
     * `upper_as` and `lower_as` below), so nothing is decoded while a program runs.
     *
     * This is the switch for the upper instructions outside the special group.
     *
     * @param code The instruction word.
     * @param fn Its function field, bits 0-5.
     */
    void upper_body(u32 code, u32 fn);

    /**
     * The switch for the upper instructions of the special group.
     *
     * @param code The instruction word.
     * @param fn Its index in the group: bits 6-10 followed by bits 0-1.
     */
    void upper_special_body(u32 code, u32 fn);

    /**
     * The switch for the lower instructions outside the special group.
     *
     * @param code The instruction word.
     * @param at The place of its pair in program memory, which a branch counts from.
     * @param op Its operation field, bits 25-31.
     */
    void lower_body(u32 code, u32 at, u32 op);

    /**
     * The switch for the lower instructions of the special group.
     *
     * @param code The instruction word.
     * @param fn Its function field, bits 0-5; below 0x3C it names an integer operation.
     * @param index Its index in the group, bits 6-10 followed by bits 0-1, when `fn` is 0x3C or
     *        more.
     */
    void lower_special_body(u32 code, u32 fn, u32 index);

    /**
     * Runs an upper instruction through the switch with its slot fixed at compile time.
     *
     * @tparam Slot The slot in `kUpperRuns`.
     * @param vu The unit to run it on.
     * @param code The instruction word.
     */
    template <unsigned Slot>
    static void upper_as(Vu& vu, u32 code);

    /**
     * Runs a lower instruction through the switch with its slot fixed at compile time.
     *
     * @tparam Slot The slot in `kLowerRuns`.
     * @param vu The unit to run it on.
     * @param code The instruction word.
     * @param at The place of its pair in program memory.
     */
    template <unsigned Slot>
    static void lower_as(Vu& vu, u32 code, u32 at);

    /**
     * Builds the table of upper instruction functions.
     *
     * @tparam N The slot numbers, in order.
     * @return One `upper_as<N>` for each slot.
     */
    template <std::size_t... N>
    static constexpr std::array<UpperRun, sizeof...(N)> upper_runs(std::index_sequence<N...>);

    /**
     * Builds the table of lower instruction functions.
     *
     * @tparam N The slot numbers, in order.
     * @return One `lower_as<N>` for each slot.
     */
    template <std::size_t... N>
    static constexpr std::array<LowerRun, sizeof...(N)> lower_runs(std::index_sequence<N...>);

    /**
     * The upper instruction functions. Upper slots: 0-63 by the function field, then 64-191 the
     * special ones by their index.
     */
    static const std::array<UpperRun, 192> kUpperRuns;

    /**
     * The lower instruction functions. Lower slots: 0-127 by the operation field, 128-191 the
     * integer operations of the special group by function field, 192-319 the rest of that group by
     * index.
     */
    static const std::array<LowerRun, 320> kLowerRuns;

    /**
     * The arithmetic instructions, one function for each operation, source of the second operand
     * and target, so that nothing about those is decided while a program runs.
     *
     * Hot path: it runs for most instructions and is longer than the 60 lines of 8.2; no
     * measurement of splitting it is recorded.
     *
     * @tparam op The operation.
     * @tparam from Where the second operand comes from.
     * @tparam to_acc True to write the accumulator, false to write the float register fd.
     * @param code The instruction word.
     */
    template <Op op, From from, bool to_acc>
    void arith(u32 code);

    /**
     * Runs a maximum or minimum instruction.
     *
     * @param code The instruction word.
     * @param from Where the second operand comes from.
     * @param max True for the maximum, false for the minimum.
     */
    void min_max(u32 code, From from, bool max);

    /**
     * Reads one field of the second operand of an instruction.
     *
     * @param code The instruction word.
     * @param from Where the operand comes from.
     * @param field The field, 0 for x to 3 for w.
     * @return The field as a raw pattern.
     */
    u32 operand(u32 code, From from, unsigned field) const;

    /**
     * Notes a computed field's four MAC flags: zero, sign, underflow, overflow.
     *
     * @param value The computed pattern.
     * @param problems The flags `fp::pack` raised for it.
     * @param field The field, 0 for x to 3 for w.
     * @param[out] flags The MAC flags of the instruction; this field's bits are ORed in.
     * @return `value`, unchanged.
     */
    u32 result(u32 value, u32 problems, unsigned field, u32& flags) const;

    /**
     * Records the flags of an arithmetic instruction as the newest and posts them to the pipe.
     *
     * @param mac_bits The instruction's MAC flags.
     */
    void post_flags(u32 mac_bits);

    /** Posts the newest MAC, status and clip values to the pipe, to show four cycles later. */
    void post();

    /**
     * Brings everything in flight to its end: flags, the divider, the function unit, a kick.
     */
    void settle();

    /** Makes the divider's result and its two flags (invalid, divide by zero) visible. */
    void finish_q();

    /** Sends the pending XGKICK packet to `on_kick`. */
    void fire_kick();

    /**
     * Writes fields of a float register; a write to VF0 is ignored.
     *
     * @param reg The float register number.
     * @param mask The dest bits, x in bit 3 down to w in bit 0.
     * @param value The four fields; only those named by `mask` are stored.
     */
    void write_vf(unsigned reg, u32 mask, const std::array<u32, 4>& value);

    /**
     * Does what goes with a write to fields of a float register (not VF0): the upper
     * instruction's old value is kept for the lower one, and the fields are readable four cycles
     * on.
     *
     * @param reg The float register number.
     * @param mask The dest bits, x in bit 3 down to w in bit 0.
     */
    void about_to_write_vf(unsigned reg, u32 mask);

    /**
     * Writes an integer register, keeping the old value for a branch right after it; VI0 is
     * ignored.
     *
     * @param reg The integer register number; only bits 0-3 count.
     * @param value The value to store.
     */
    void write_vi(unsigned reg, u16 value);

    /**
     * Writes an integer register from an instruction that reads flags; VI0 is ignored.
     *
     * The instructions that read flags into an integer register finish early: a branch right after
     * one tests the new value.
     *
     * @param reg The integer register number; only bits 0-3 count.
     * @param value The value to store.
     */
    void write_vi_from_flags(unsigned reg, u16 value);

    /**
     * Reads an integer register as a branch sees it.
     *
     * @param reg The integer register number; only bits 0-3 count.
     * @return The value from before the instruction right ahead, while that write is held back.
     */
    u16 branch_vi(unsigned reg) const;

    /**
     * Arranges a branch: it takes effect after the instruction in the delay slot.
     *
     * @param target The instruction to continue at, counted from the start of program memory.
     */
    void branch(u32 target);

    /**
     * Starts a divider result on its way to Q.
     *
     * @param value The result, a raw pattern.
     * @param latency Cycles until it arrives.
     * @param divide_flags The status bits the divider sets with the result.
     */
    void start_q(u32 value, unsigned latency, u32 divide_flags);

    /**
     * Works out what a pair reads that can make it wait: float registers (a register written less
     * than four cycles ago holds the pair up), the divider, the function unit.
     *
     * @param[out] needs The entry to fill in.
     * @param up The upper instruction word.
     * @param low The lower instruction word.
     */
    void work_out(Needs& needs, u32 up, u32 low) const;

    /**
     * Tells whether no float register this pair reads is written by one of the three pairs before
     * it in memory. (Generously: any register a pair might write counts.)
     *
     * @param needs The pair's entry.
     * @param at The pair's place in program memory.
     * @return True when the pair never waits for a float register if it is reached in a straight
     *         line.
     */
    bool never_waits(const Needs& needs, u32 at) const;

    /**
     * Tells whether any instruction can read the flags the upper instruction at `at` sets.
     *
     * They are seen from four cycles on and until the next instruction that sets flags has had its
     * four cycles. A reader in that stretch, or anything that leaves the straight line (a branch,
     * the end of the program), counts.
     *
     * @param at The pair's place in program memory.
     * @return True when the flags may be read; always true when `skip_unread_flags` is off or the
     *         image has sticky readers.
     */
    bool flags_can_be_read(u32 at) const;

    /**
     * Looks at program memory as a whole: does anything read flags in a way that depends on every
     * instruction (the bits that remember, the MAC flags)?
     */
    void look_at_programs();

    /**
     * Starts a function unit result on its way to P.
     *
     * @param value The result, as a host double.
     * @param latency Cycles until it arrives.
     */
    void start_p(double value, unsigned latency);

    /**
     * Returns a pointer to a quadword of data memory, 16 bytes each, wrapping at the memory's size.
     *
     * @param address The quadword number.
     * @return The address of its first byte.
     */
    u8* quad(u32 address) { return memory_.data + ((address * 16) & (memory_.data_bytes - 1)); }

    /** The program and data memory, owned by the caller. */
    Memory memory_;

    /** Program memory size in instructions, minus one. */
    u32 pc_mask_ = 0;

    /**
     * `running_` is true while a microprogram runs. `sync_point_` is set by a pair with the M bit
     * and cleared when `advance_to_sync` starts.
     */
    bool running_ = false, sync_point_ = false;

    /** True while the upper instruction runs: its register write is held back for the lower one. */
    bool in_upper_ = false;

    /** The float register the upper instruction wrote, or 0 when it wrote none. */
    unsigned upper_reg_ = 0;

    /** The value of `upper_reg_` from before the upper instruction wrote it. */
    std::array<u32, 4> upper_old_{};

    /** Time, in cycles: one an instruction, more when an instruction waits. */
    u64 cycle_ = 0;

    /** A microprogram is running (the EE's own instructions are not timed). */
    bool timed_ = false;

    /** The cycle from which each field of each float register can be read. */
    std::array<std::array<u64, 4>, 32> readable_{};

    /** The latest of a register's four fields' `readable_` cycles. */
    std::array<u64, 32> register_ready_{};

    /**
     * What has been worked out about the pairs of program memory, kept for each content the memory
     * has had: the games swap a few programs in and out all the time, and each comes back as it
     * was.
     */
    struct Image {
        /** The program memory as it was. */
        std::vector<u8> micro;

        /** The entry for each pair. */
        std::vector<Needs> needs;

        /** `looked_at` is true once `look_at_programs` has run; `sticky_readers` is its answer. */
        bool looked_at = false, sticky_readers = true;

        /** Pairs run and runs started, for each 256 pairs of the memory (see `Use`). */
        std::vector<u64> pairs, starts;
    };

    /** The images, by a 64-bit hash of their program memory. */
    std::unordered_map<u64, std::unique_ptr<Image>> images_;

    /** The image in use, or null when none has been chosen yet. */
    Image* image_ = nullptr;

    /** The entries of the image in use. */
    Needs* needs_ = nullptr;

    /** The pair counts of the image in use (`Image::pairs`). */
    u64* pairs_run_ = nullptr;

    /** Counts a run started at `pc` under the content program memory has now. */
    void count_start();

    /** Program memory may differ from the image in use. */
    bool program_dirty_ = true;

    /** Something in the image in use reads flags in a way that depends on every instruction. */
    bool sticky_readers_ = true;

    /** Finds what is known about program memory as it is now, or starts afresh. */
    void choose_image();

    /** True when something can read the flags of the instruction being run. */
    bool flags_wanted_ = true;

    /** The flags on their way: a ring of eight, each shown at its cycle. */
    std::array<Flags, 8> flag_pipe_{};

    /** The ring's oldest entry, and how many are in it. */
    unsigned flag_first_ = 0, flag_count_ = 0;

    /** The newest values, visible or not. */
    u32 mac_latest_ = 0, status_latest_ = 0, clip_latest_ = 0;

    /** The divider's result, the status bits that go with it, and the function unit's result. */
    u32 q_next_ = 0, q_flags_ = 0, p_next_ = 0;

    /** The cycle the Q and P results arrive, 0 when none is on its way. */
    u64 q_at_ = 0, p_at_ = 0;

    /** Instructions left until a branch, a stop and a kick take effect; 0 for none. */
    unsigned branch_in_ = 0, stop_in_ = 0, kick_in_ = 0;

    /** Pairs run since the program was started or a branch was taken. */
    u64 straight_ = 0;

    /** Where a branch goes, and where the pending XGKICK packet starts. */
    u32 branch_target_ = 0, kick_address_ = 0;

    /** The integer register last written, and how many more cycles its old value stays visible. */
    unsigned backup_reg_ = 0, backup_ttl_ = 0;

    /** The value of `backup_reg_` from before it was written. */
    u16 backup_value_ = 0;

    /** The control registers CMSAR0 and FBRST, as the EE wrote them. */
    u32 cmsar0_ = 0, fbrst_ = 0;
};

}  // namespace ps2
