// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Host code in place of guest functions: loading a library built from a game's decompiled C and
 * running its functions where the retail program would run its own.
 *
 * This is how the port grows out of the interpreter (docs/DESIGN.md, route C): every decompiled
 * function that is compiled for the host and proven against the retail one takes that much away
 * from what is interpreted. The library is built on the user's machine by runtime/port; the
 * runtime ships no game code.
 */

#pragma once

#include <array>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ps2/ee.h"
#include "ps2/memory.h"
#include "sys/native_abi.h"

namespace sys {

using ps2::u32;
using ps2::u64;
using ps2::u8;

/**
 * A loaded library of host functions and which guest addresses they stand in for right now.
 *
 * A function is bound to its address only while the guest's memory there holds the retail code
 * the function was written from: the bytes are checked against the library's checksum the first
 * time the address is reached, and again after the program says it changed code (a level
 * loading). So a function of another level, or of another version of the game, is never run.
 *
 * A game with a program for each level has the same function at another address in each, and
 * calls it by that address. So the level in memory is found again whenever code changed, the
 * library is told, and a level's functions are bound in that level only.
 *
 * Only the thread that runs the EE uses it.
 */
class Native {
public:
    /**
     * Where host code keeps its own data and its stack: in the first megabyte of main memory,
     * which is the kernel's on the console and holds nothing here.
     *
     * They are in main memory, not beside it, because a function's local buffer is as good an
     * address as any to the game: it hands such buffers to the DMA controller and masks their
     * addresses like any other (seen in game code: a texture upload from a structure on the
     * stack). Data goes up from `kDataBase`; the stack goes down from `kStackTop` to
     * `kDataEnd`. Interrupt handlers have their own stack above (`Machine::kHandlerStack`).
     */
    static constexpr u32 kDataBase = 0x00080000;
    static constexpr u32 kDataEnd = 0x00090000;
    static constexpr u32 kStackTop = 0x000F0000;

    /**
     * Makes an empty set over a core and its memory.
     *
     * @param ee The core whose registers the functions use; it must outlive this object.
     * @param memory The guest's memory; it must outlive this object.
     */
    Native(ps2::Ee& ee, ps2::GuestMemory& memory);

    /**
     * Loads a library and makes its functions candidates for their addresses.
     *
     * @param path The library file.
     * @param[out] error Why it failed, when it did.
     * @return True if it is loaded.
     */
    bool load(const std::string& path, std::string* error);

    /** Forgets which functions are bound: each is checked again when its address is reached. */
    void code_changed();

    /** How many functions the library has, and how many are bound to their addresses now. */
    std::size_t functions() const { return functions_.size(); }

    std::size_t bound() const { return bound_; }

    /** The level whose program is in memory, or `OPENRAC_NATIVE_BOOT`; as last looked at. */
    int level() const { return level_; }

    /** How many times host code ran in place of a guest function. */
    u64 calls = 0;

    /**
     * Which of the library's functions are used, set before `load`: those whose place in the
     * library's table is from `first` up to but not including `last`, less those named in `skip`.
     * For finding the function that misbehaves by halving.
     */
    std::size_t first = 0;
    std::size_t last = ~std::size_t{0};
    std::set<std::string> skip;

    /** For that search: when not negative, `load` prints the name at this place of the table. */
    long name_at = -1;

    /**
     * How many of each function's first calls are checked against the retail code; 0 for none.
     *
     * A checked call runs the retail function in the interpreter, puts the machine back as it
     * was, runs the host function and compares what the two left in memory and in the result
     * register. A function that differs is handed back to the interpreter for good and listed
     * by `report`. The retail result is the one that stays.
     */
    unsigned check = 0;

    /**
     * Counted up by the owner whenever a guest function reaches outside the guest's memory: a
     * replaced library function, a kernel call, a device register. A call that did cannot be
     * run twice, so it is not checked.
     */
    u64 outside = 0;

    /**
     * Whether calls are counted by level and address, set before `load`: every call instruction
     * the interpreter runs and every call host code makes. `write_calls` gives the counts. It
     * shows which guest functions a run uses that no host function stands in for yet.
     */
    bool count_calls = false;

    /**
     * Writes the counted calls, one line for each level and address: the level (-1 for the boot
     * program), the address in hexadecimal, the number of calls, and 1 if host code ran there.
     *
     * @param out Where to write.
     */
    void write_calls(std::FILE* out) const;

    /**
     * Writes one line for each function that ran: how often, its address and its name.
     *
     * @param out Where to write.
     * @param most How many lines at most, the busiest first.
     */
    void report(std::FILE* out, std::size_t most) const;

private:
    /** What is known about one function's binding. */
    enum class State : u8 {
        Unchecked,  // Not compared with the guest's memory since code last changed.
        Bound,      // The guest's memory holds the retail function: host code runs.
        Other,      // Something else is there: the interpreter runs it.
    };

    /** One function of the library. */
    struct Function {
        OpenracNativeFunction entry;  // The library's record.
        State state = State::Unchecked;
        u64 calls = 0;           // Times it ran.
        unsigned checked = 0;    // Calls compared with the retail code so far.
        unsigned unchecked = 0;  // Calls that could not be compared: they reached outside.
        std::string differs;     // How it differed from the retail code; empty if it never did.
        bool unset = false;      // Its result changed with what its stack held before the call.
    };

    /** Everything a function may change that is compared or put back: registers and memory. */
    struct Snapshot {
        std::array<ps2::Ee::Reg, 32> gpr;  // The general registers.
        std::array<u32, 32> fpr;           // The FPU registers.
        u64 hi, lo, hi1, lo1;              // The multiply results.
        u32 sa, facc, fcr31;               // SA and the FPU's accumulator and flags.
        std::vector<u8> ram;               // Main memory.
        std::vector<u8> scratchpad;        // The scratchpad.
    };

    /**
     * Runs the host function for an address, if it is bound.
     *
     * @param address The address the program counter reached.
     * @return True if host code ran.
     */
    bool enter(u32 address);

    /**
     * Runs one call both ways and compares (see `check`).
     *
     * @param function The function; it is bound and its mark is set.
     * @return True if the call was made; false if nothing was done and the interpreter is to
     * make it (the retail function did not come back without a timed event).
     */
    bool check_call(Function& function);

    /**
     * Says how what one run of a call left differs from what another left.
     *
     * @param ours One run's registers and memories.
     * @param theirs The other run's.
     * @param sp The caller's stack pointer: the megabyte below it is dead stack and not compared.
     * @param result What the function returns (`OpenracNativeFunction::result`).
     * @param who What to call the other run in the text ("the retail code").
     * @return The first difference in words, or an empty string when there is none.
     */
    std::string difference(
        const Snapshot& ours, const Snapshot& theirs, u32 sp, u32 result, const char* who
    ) const;

    /**
     * Copies the registers and memories.
     *
     * @param[out] to Where to.
     */
    void take(Snapshot& to);

    /**
     * Puts registers and memories back from a copy.
     *
     * @param from The copy.
     */
    void put_back(const Snapshot& from);

    /** Finds which level's program is in memory and tells the library if it is another. */
    void find_level();

    /**
     * Counts one call to an address in the level that is in memory.
     *
     * @param address The address called.
     */
    void note_call(u32 address);

    /**
     * Compares the guest's memory at a function's address with the code it stands in for.
     *
     * @param function The function.
     * @return True if the checksum of the bytes there is the library's.
     */
    bool matches(const Function& function);

    /** The callbacks the library is given; `context` is this object. */
    static uint64_t get_r(void* context, int n);
    static void set_r(void* context, int n, uint64_t value);
    static uint32_t get_f(void* context, int n);
    static void set_f(void* context, int n, uint32_t bits);
    static void call(void* context, uint32_t address);
    static uint64_t hardware_read(void* context, uint32_t address, uint32_t bytes);
    static void hardware_write(void* context, uint32_t address, uint32_t bytes, uint64_t value);
    static uint32_t allocate(void* context, uint32_t bytes, uint32_t alignment);
    static uint32_t float_op(uint32_t op, uint32_t a, uint32_t b);
    static int32_t float_compare(uint32_t a, uint32_t b);

    ps2::Ee& ee_;                                    // The core.
    ps2::GuestMemory& memory_;                       // The guest's memory.
    OpenracHost host_{};                             // What the library was given.
    std::vector<Function> functions_;                // The library's functions.
    const OpenracNativeLibrary* library_ = nullptr;  // The loaded library.

    /** Indexes into `functions_` by guest address: one for each level that has code there. */
    std::unordered_map<u32, std::vector<std::size_t>> by_address_;

    std::vector<u8> marks_;  // One byte a word of main memory, for the core.

    /** Calls counted, by level plus one in the upper half of the key and address in the lower. */
    std::unordered_map<u64, u64> call_counts_;

    /** The keys of `call_counts_` at which host code ran. */
    std::set<u64> host_ran_;

    std::size_t bound_ = 0;            // Functions in state `Bound`.
    int level_ = OPENRAC_NATIVE_BOOT;  // The level whose program is in memory.
    bool level_known_ = false;         // `level_` was found since code last changed.
    u32 stack_pointer_ = 0;            // The host code's shared stack pointer.
    u32 next_data_ = kDataBase;        // Where the next module's data goes.
    bool checking_ = false;            // A call is being compared; calls inside it are not.
    Snapshot before_;                  // The machine before a checked call.
    Snapshot host_result_;             // What the host function left.
    Snapshot host_again_;              // What it left when run again over another stack.
    Snapshot retail_result_;           // What the retail function left.
};

}  // namespace sys
