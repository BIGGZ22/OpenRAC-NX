// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The source-chain walker of an EE DMA channel.
 *
 * A source chain is a linked list of tags in guest memory. Each tag says how many quadwords to send
 * and where the next tag is. The walker follows the list and hands each block of data to a sink,
 * and it stops where the hardware stops: at the end of the list, or at a tag with its interrupt bit
 * set. It models no other channel mode and no timing; the whole chain is sent at once.
 *
 * Sources: the EE's DMA controller as publicly documented.
 */

#pragma once

#include <functional>

#include "memory.h"
#include "types.h"

namespace ps2 {

/**
 * The registers of one DMA channel, as the EE's DMA controller keeps them.
 *
 * The chain walker reads and updates them the way the hardware does, so a handler that runs
 * between two pieces of a chain sees what a real one would see.
 */
struct DmaChannel {
    /** Channel control (CHCR): bit 6 TTE, bit 7 TIE, bit 8 STR, bits 4-5 ASP, bits 16-31 TAG. */
    u32 chcr = 0;

    /** Memory address (MADR): where the data being sent comes from. */
    u32 madr = 0;

    /** Quadword count (QWC): quadwords still to send of a transfer started with a count. */
    u32 qwc = 0;

    /** Tag address (TADR): where the next tag is. */
    u32 tadr = 0;

    /** Address stack (ASR0 and ASR1): the return addresses of CALL tags. */
    u32 asr[2] = {0, 0};
};

namespace dmatag {

/** The ID field of a DMA tag, bits 28-30, for a source chain (documented). */
enum : u32 {
    REFE,  // data at ADDR, then the chain ends
    CNT,   // data follows the tag, the next tag follows the data
    NEXT,  // data follows the tag, the next tag is at ADDR
    REF,   // data at ADDR, the next tag follows this one
    REFS,  // as REF, with stall control
    CALL,  // data follows the tag, the next tag is at ADDR, the return address is pushed
    RET,   // data follows the tag, the next tag is the address popped
    END    // data follows the tag, then the chain ends
};

}  // namespace dmatag

/**
 * Why a chain stopped.
 *
 * After `Interrupt` the chain can be resumed from the channel's registers.
 */
enum class DmaStop {
    End,        // the chain finished
    Interrupt,  // a tag with IRQ set, with TIE on: the handler runs, then the chain may resume
    Runaway,    // more tags than any real list has: the list is corrupt
};

/**
 * Where a channel's data goes (VIF1 for channel 1).
 *
 * It is called once per block with a pointer into guest memory and the block's size in bytes. The
 * pointer is valid only during the call.
 */
using DmaSink = std::function<void(const u8* data, std::size_t bytes)>;

/**
 * Runs a source chain from the channel's current state until it stops.
 *
 * The channel's registers are left as the hardware leaves them, so a handler can read TADR and the
 * TAG field and the chain can be resumed by calling this again.
 *
 * @param memory The guest memory the tags and the data are read from.
 * @param channel The channel to run. TADR names the first tag; it is updated as the chain goes.
 * @param sink Receives each block of data, and the second half of each tag when TTE is on.
 * @return Why the chain stopped.
 */
DmaStop run_source_chain(GuestMemory& memory, DmaChannel& channel, const DmaSink& sink);

}  // namespace ps2
