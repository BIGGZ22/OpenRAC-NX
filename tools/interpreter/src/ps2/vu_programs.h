// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Which of a game's vector unit programs is in a unit's program memory, and how much each ran.
 *
 * A game's executable stores its vector unit programs in chunks of up to 0x800 bytes (one MPG
 * command's worth) and names each chunk in a table: the assembler the games were built with
 * (the DVP assembler of the console's toolchain) writes a section `.DVP.ovlytab` with one record
 * a chunk, and a section named `.DVP.overlay..<address>.<program>.<line>.<chunk>` whose size is
 * the chunk's. All chunks of one program share the program number. With the table, a unit's
 * program memory can be told apart chunk by chunk, which is what a renderer needs before host
 * code can stand in for a program (docs/VU_PROGRAMS.md).
 *
 * Sources: the ELF32 format as publicly documented; the table's layout as described in ReRAC's
 * `docs/formats/vu_microprograms.md` (ISC) and read again from our own disc images.
 */

#pragma once

#include <vector>

#include "types.h"
#include "vu.h"

namespace ps2 {

/** One chunk of a vector unit program, as a game's executable stores it. */
struct VuChunk {
    /** The program's number, the same for all its chunks. */
    u32 program = 0;

    /** Which chunk of the program this is, counted from 0. */
    u32 chunk = 0;

    /** Where the chunk goes in a unit's program memory, in bytes. */
    u32 address = 0;

    /** The chunk's instructions. */
    std::vector<u8> code;
};

/** How much one program ran. */
struct VuProgramUse {
    /** The program's number, or 0 for program memory that matched no chunk. */
    u32 program = 0;

    /** How many times a run started inside the program. */
    u64 starts = 0;

    /** How many instruction pairs of the program ran. */
    u64 pairs = 0;
};

/**
 * Reads the chunks of the vector unit programs from an executable.
 *
 * @param elf The bytes of an ELF32 program.
 * @return The chunks in the table's order; empty if the program has no table or is not an ELF.
 */
std::vector<VuChunk> vu_program_chunks(const std::vector<u8>& elf);

/**
 * Sums what a unit ran by program.
 *
 * Each 0x800 bytes of a content of program memory count for the program whose chunk is there,
 * byte for byte, at the chunk's address.
 *
 * @param chunks The chunks of the game's programs.
 * @param uses What the unit ran, from `Vu::uses`.
 * @return One entry for each program that ran, the one with the most pairs first.
 */
std::vector<VuProgramUse> vu_program_use(
    const std::vector<VuChunk>& chunks, const std::vector<Vu::Use>& uses
);

}  // namespace ps2
