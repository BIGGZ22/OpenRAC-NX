// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Game memory: the 4 GB reservation whose offsets are the console's
// addresses (guest.h has the map). One per process.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "openrac/guest.h"

namespace openrac::runtime {

class Memory {
public:
    // Reserves the 4 GB, maps main RAM (with its uncached aliases where the
    // platform can alias pages), the scratchpad, the register pages and the
    // port's region, and sets openrac_guest_base. Ends the program if the
    // reservation fails.
    static Memory& create();
    static Memory& get();

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    std::uint8_t* base() const { return base_; }

    // True when main RAM is visible at its uncached addresses as well
    // (Linux and macOS; Windows 10 1803 and later).
    bool has_aliases() const { return aliases_; }

    // The host bytes at [address, address + size). Ends the program if the
    // range is not mapped.
    std::span<std::uint8_t> bytes(gaddr address, std::size_t size);

    // True if the whole range is mapped.
    bool mapped(gaddr address, std::size_t size) const;

    // Zeroes main RAM, the scratchpad, the register pages and the port's
    // region, and resets the game stack and the string region.
    void clear();

    // The game address that a host address inside the reservation stands
    // for, or false. Used by the crash handler.
    bool contains(const void* host, gaddr* address) const;

private:
    Memory() = default;
    void map_all();

    std::uint8_t* base_ = nullptr;
    bool aliases_ = false;
};

// Installs a handler that reports an access to an unmapped game address
// (with the address and the function being run, when known) before the
// program ends.
void install_crash_handler();

// A debugging aid (OPENRAC_GUARD): the write after the first `skip` into [address, address + bytes)
// of game memory is reported with the host's stack and ends the program. Other writes to the same pages pass and
// disarm it until it is called again (once a frame). POSIX only.
void guard_writes(gaddr address, std::uint32_t bytes, std::uint32_t skip = 0);

}  // namespace openrac::runtime
