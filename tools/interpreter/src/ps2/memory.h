// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The memory a game program sees: main memory and the scratchpad, laid out as the console's
 * address space.
 *
 * The interpreter turns every address a program uses into a pointer to host memory by masking, so
 * an address the program got wrong lands somewhere inside the two memories and never outside
 * them. Host code compiled from a game's decompiled C uses guest addresses as they are, as
 * offsets from one base: for that, the memories are mapped inside one 4 GB reservation, main
 * memory at each of its mirrors. Memory-mapped registers are not modelled here.
 *
 * Sources: the EE's memory sizes and address map as publicly documented.
 */

#pragma once

#include <cstddef>

#include "types.h"

namespace ps2 {

/**
 * The memory a game program sees: 32 MB of main memory and the 16 KB scratchpad.
 *
 * It owns a 4 GB reservation of address space, most of it unmapped. Not copyable.
 */
class GuestMemory {
public:
    /** Size of main memory in bytes: 32 MB (documented). */
    static constexpr u32 kRamBytes = 32 * 1024 * 1024;

    /** Size of the scratchpad in bytes: 16 KB (documented). */
    static constexpr u32 kScratchpadBytes = 16 * 1024;

    /** Where the scratchpad is in the address space (documented). */
    static constexpr u32 kScratchpadBase = 0x70000000;

    /** Reserves the address space and maps the memories into it, zero-filled. */
    GuestMemory();

    /** Gives the address space back. */
    ~GuestMemory();

    GuestMemory(const GuestMemory&) = delete;
    GuestMemory& operator=(const GuestMemory&) = delete;

    /**
     * The base of the address space: a guest address is an offset from it.
     *
     * Main memory answers at 0, 0x20000000, 0x80000000 and 0xA0000000 (the cached and uncached
     * segments, documented), the scratchpad at `kScratchpadBase`. Anything else is unmapped:
     * touching it stops the process.
     *
     * @return The base, or null on a host where the mirrors could not be mapped.
     */
    u8* space() { return mirrored_ ? space_ : nullptr; }

    /**
     * Turns a DMA address into a host pointer.
     *
     * Bit 31 selects the scratchpad (the SPR flag of a tag or of MADR), otherwise the address is a
     * main memory address in any segment. A scratchpad address is rounded down to a quadword.
     *
     * @param address The address as it sits in a tag or in MADR, SPR flag included.
     * @return A pointer into the scratchpad or into main memory.
     */
    u8* dma(u32 address) {
        // SPR flag: the address is an offset into the scratchpad.
        if (address & 0x80000000u) {
            return scratchpad_ + (address & (kScratchpadBytes - 1) & ~0xFu);
        }
        return ram_ + (address & (kRamBytes - 1));
    }

    /**
     * Turns a main memory address into a host pointer.
     *
     * @param address An address in any segment; the bits above the memory size are dropped.
     * @return A pointer into main memory.
     */
    u8* ram(u32 address) { return ram_ + (address & (kRamBytes - 1)); }

    /**
     * Turns a scratchpad offset into a host pointer.
     *
     * @param offset Byte offset in the scratchpad; the bits above its size are dropped.
     * @return A pointer into the scratchpad.
     */
    u8* scratchpad(u32 offset) { return scratchpad_ + (offset & (kScratchpadBytes - 1)); }

    /**
     * Reads a value from main memory.
     *
     * @tparam T The type to read, usually an integer.
     * @param address A main memory address; it need not be aligned.
     * @return The value at that address.
     */
    template <typename T>
    T read(u32 address) {
        return load<T>(ram(address));
    }

    /**
     * Writes a value to main memory.
     *
     * @tparam T The type to write, usually an integer.
     * @param address A main memory address; it need not be aligned.
     * @param value The value to store.
     */
    template <typename T>
    void write(u32 address, T value) {
        store<T>(ram(address), value);
    }

private:
    /** The 4 GB reservation; a guest address is an offset into it. */
    u8* space_ = nullptr;

    /** Main memory: the reservation's first 32 MB. */
    u8* ram_ = nullptr;

    /** The scratchpad, at `kScratchpadBase` in the reservation. */
    u8* scratchpad_ = nullptr;

    /** True when main memory also answers at its mirrors, so that `space()` can be used. */
    bool mirrored_ = false;
};

}  // namespace ps2
