// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The source-chain walker declared in dma.h.
 *
 * Sources: the EE's DMA controller as publicly documented.
 */

#include "dma.h"

namespace ps2 {

DmaStop run_source_chain(GuestMemory& memory, DmaChannel& channel, const DmaSink& sink) {
    // CHCR: TTE is bit 6, TIE bit 7, ASP bits 4-5, the number of CALL levels in use (documented).
    const bool transfer_tag = (channel.chcr & 0x40) != 0;
    const bool tag_interrupt = (channel.chcr & 0x80) != 0;
    u32 depth = (channel.chcr >> 4) & 3;

    // Called on every way out of the chain, on the caller's thread.
    auto stop = [&](DmaStop why) {
        // STR off, ASP updated: 0x130 is STR (bit 8) with ASP (bits 4-5).
        channel.chcr = (channel.chcr & ~0x130u) | (depth << 4);
        return why;
    };

    // Data left over from a transfer that was started with a count.
    if (channel.qwc) {
        // QWC counts quadwords of 16 bytes.
        sink(memory.dma(channel.madr), std::size_t{channel.qwc} * 16);
        channel.madr += channel.qwc * 16;
        channel.qwc = 0;
    }

    // The chain ends at an END-type tag or an interrupt; the count stops a list with neither.
    for (u32 tags = 0; tags < (1u << 22); tags++) {
        const u8* tag = memory.dma(channel.tadr);

        /*
         * DMA tag, low 64 bits: QWC bits 0-15, ID bits 28-30, IRQ bit 31, ADDR bits 32-62, SPR bit
         * 63 (documented). SPR stays in the address, where `dma` reads it as bit 31.
         */
        u64 lo = load<u64>(tag);
        u32 qwc = static_cast<u32>(lo & 0xFFFF);
        u32 id = static_cast<u32>(bits(lo, 28, 3));
        bool irq = bits(lo, 31, 1) != 0;
        u32 address = static_cast<u32>(lo >> 32);

        // The TAG field of CHCR (bits 16-31) takes bits 16-31 of the tag just read.
        channel.chcr = (channel.chcr & 0xFFFFu) | (static_cast<u32>(lo) & 0xFFFF0000u);

        // With TTE on, the tag's second doubleword (bytes 8-15) goes to the sink before the data.
        if (transfer_tag) {
            sink(tag + 8, 8);
        }

        // A tag is one quadword (16 bytes); the data after it starts at the next quadword.
        u32 after_tag = channel.tadr + 16;
        u32 from = after_tag;
        bool end = false;

        switch (id) {
            // The data is at ADDR, and the chain ends after it.
            case dmatag::REFE:
                from = address;
                end = true;
                break;

            // The data follows the tag, and the next tag follows the data.
            case dmatag::CNT:
                channel.tadr = after_tag + qwc * 16;
                break;

            // The data follows the tag, and the next tag is at ADDR.
            case dmatag::NEXT:
                channel.tadr = address;
                break;

            /*
             * The data is at ADDR, and the next tag follows this one. REFS adds stall control, which
             * this model does not apply (documented).
             */
            case dmatag::REF:
            case dmatag::REFS:
                from = address;
                channel.tadr = after_tag;
                break;

            /*
             * The data follows the tag, the next tag is at ADDR, and the address after the data goes
             * on the address stack, which has two levels. With both in use nothing is pushed.
             */
            case dmatag::CALL:
                if (depth < 2) {
                    channel.asr[depth++] = after_tag + qwc * 16;
                }
                channel.tadr = address;
                break;

            // The data follows the tag, and the next tag is the one popped off the address stack.
            case dmatag::RET:
                // With nothing on the stack there is nowhere to return to: the chain ends.
                if (depth > 0) {
                    channel.tadr = channel.asr[--depth];
                } else {
                    end = true;
                }
                break;

            // END (ID 7): the data follows the tag, and the chain ends after it.
            default:
                end = true;
                break;
        }

        // MADR names the block being sent and ends up just past it.
        channel.madr = from;

        // A tag with QWC 0 carries no data.
        if (qwc) {
            sink(memory.dma(from), std::size_t{qwc} * 16);
            channel.madr += qwc * 16;
        }

        // The last tag has been sent.
        if (end) {
            return stop(DmaStop::End);
        }

        // A tag with IRQ set stops the chain for the game's handler, if TIE is on.
        if (irq && tag_interrupt) {
            return stop(DmaStop::Interrupt);
        }
    }

    // More tags than any real list has: the walker gave up at 2^22.
    return stop(DmaStop::Runaway);
}

}  // namespace ps2
