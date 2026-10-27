// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The machine's state at the start of a guest function, written to a file and read back.
 *
 * For checking a function written in C against the retail one by what they do (`openrac-fcheck`):
 * `openrac-boot --capture` writes the state at real calls, the checker runs both functions from it.
 * It leaves out the devices: a function that waits for one cannot be checked this way.
 */

#pragma once

#include <array>
#include <string>
#include <vector>

#include "ps2/ee.h"
#include "ps2/vu.h"

namespace sys {

using ps2::u16;
using ps2::u32;
using ps2::u64;
using ps2::u8;

/** Everything a call can read or change apart from the devices. */
struct CallState {
    /** The function's first instruction, where the program counter was. */
    u32 address = 0;

    /** The EE's registers. */
    std::array<ps2::Ee::Reg, 32> gpr{};
    std::array<u32, 32> fpr{};
    u64 hi = 0, lo = 0, hi1 = 0, lo1 = 0;
    u32 sa = 0, facc = 0, fcr31 = 0;

    /** VU0's registers, which hand-written routines use from the EE (macro mode). */
    std::array<std::array<u32, 4>, 32> vf{};
    std::array<u16, 16> vi{};
    std::array<u32, 4> acc{};
    u32 q = 0, p = 0, i = 0, r = 0, mac = 0, status = 0, clip = 0;

    /** Main memory and the scratchpad. */
    std::vector<u8> ram, scratchpad;

    /** VU0's program and data memories. */
    std::vector<u8> vu0_micro, vu0_data;

    /** Takes the state from a running machine. */
    void take(const ps2::Ee& ee, const ps2::Vu& vu0, ps2::GuestMemory& memory,
              const u8* micro, const u8* data);

    /** Puts it into a machine. */
    void put(ps2::Ee& ee, ps2::Vu& vu0, ps2::GuestMemory& memory, u8* micro, u8* data) const;

    /**
     * Writes the state to a file, the memories compressed.
     *
     * @return False when the file cannot be written.
     */
    bool save(const std::string& path) const;

    /**
     * Reads a state that `save` wrote.
     *
     * @return False when the file cannot be read or is not such a file; `error` says why.
     */
    bool load(const std::string& path, std::string* error);
};

}  // namespace sys
