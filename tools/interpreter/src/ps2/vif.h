// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * VIF1, the vector interface unit in front of VU1, as a decoder of display list commands.
 *
 * It models the commands the games use on VIF1: the setup commands (STCYCL, OFFSET, BASE, ITOP,
 * STMOD, MSKPATH3, MARK, STMASK, STROW, STCOL), UNPACK in every format with masks, modes and write
 * cycles, MPG, the program starts (MSCAL, MSCALF, MSCNT) with the double buffer, DIRECT and
 * DIRECTHL, and the flushes as no-ops. It leaves out the FIFO, the stalls, interrupts on the IRQ
 * bit and the status registers: each command takes effect as soon as its data has arrived.
 *
 * Sources: the VIF and its command set as publicly documented.
 */

#pragma once

#include <array>
#include <functional>
#include <vector>

#include "gif.h"
#include "types.h"

namespace ps2 {

/**
 * VIF1: the interface between the display list and VU1. It unpacks vertex
 * data into VU1's data memory, loads microprograms, starts them, and passes
 * GIF packets straight through to the GS (DIRECT).
 *
 * All of it runs on the thread that calls `write`, which is the thread that feeds the GIF. The
 * callbacks are called on that thread, from inside `write`.
 */
class Vif1 {
public:
    /** Size of VU1's data memory and of its program memory in bytes: 16 KB each (documented). */
    static constexpr u32 kMemoryBytes = 16 * 1024;

    /**
     * Makes a VIF1 in its reset state.
     *
     * @param gif The GIF that DIRECT packets are written to, on path 2.
     */
    explicit Vif1(Gif& gif) : gif_(gif) { reset(); }

    /** Clears VU1's memories, the registers and any command waiting for more data. */
    void reset();

    /**
     * Takes display list data, in any chunking; a command split across calls is finished when the
     * rest arrives.
     *
     * @param data The next bytes of the list.
     * @param bytes How many there are. A trailing part of a 4-byte word is dropped.
     */
    void write(const u8* data, std::size_t bytes);

    /**
     * Start the microprogram at `address` (in instructions), or continue the one that stopped
     * (MSCNT) when `resume` is true.
     *
     * Called by MSCAL, MSCALF and MSCNT, on the thread that calls `write`, after the double buffer
     * has moved on. `address` is 0 and `resume` true for MSCNT.
     */
    std::function<void(u32 address, bool resume)> on_start;

    /** Called when MPG has written program memory. */
    std::function<void()> on_program;

    /** VU1 data memory, 1,024 quadwords. */
    std::array<u8, kMemoryBytes> data{};

    /** VU1 program memory, 2,048 instructions. */
    std::array<u8, kMemoryBytes> micro{};

    /**
     * Registers, named as on the machine: CL and WL from STCYCL, the mode from STMOD and the mask
     * from STMASK.
     */
    u32 cl = 1, wl = 1, mode = 0, mask = 0;

    /** The ROW and COL registers, set by STROW and STCOL. */
    std::array<u32, 4> row{}, col{};

    /**
     * The double buffer registers BASE, OFFSET, TOPS, TOP, ITOPS and ITOP, which BASE, OFFSET,
     * ITOP and every program start set, and MARK, which MARK sets.
     */
    u32 base = 0, ofst = 0, tops = 0, top = 0, itops = 0, itop = 0, mark = 0;

    /** The double buffer flag, DBF: which half of the buffer the next program start hands over. */
    bool dbf = false;

    /** MSKPATH3: whether path 3 is masked. Only stored; nothing in the model reads it. */
    bool path3_masked = false;

    /** Commands met that are not VIF codes. */
    u64 unknown_codes = 0;

private:
    /**
     * Counts the words of data that follow a command.
     *
     * @param code The VIF code, the first word of the command.
     * @return The number of 32-bit words after it, or 0.
     */
    std::size_t operand_words(u32 code) const;

    /**
     * Runs one complete command.
     *
     * @param code The VIF code.
     * @param operands The words after it.
     * @param words How many there are.
     */
    void execute(u32 code, const u32* operands, std::size_t words);

    /**
     * Runs an UNPACK command: writes its vectors into VU1's data memory.
     *
     * @param code The VIF code.
     * @param operands The packed data after it.
     * @param words How many 32-bit words there are.
     */
    void unpack(u32 code, const u32* operands, std::size_t words);

    /**
     * Starts a microprogram, after handing it the buffer the list has filled.
     *
     * @param address Instruction to start at.
     * @param resume True to continue the program that stopped instead.
     */
    void start(u32 address, bool resume);

    /** The GIF that DIRECT packets go to. */
    Gif& gif_;

    /** Words received that do not yet make a whole command. */
    std::vector<u32> pending_;
};

}  // namespace ps2
