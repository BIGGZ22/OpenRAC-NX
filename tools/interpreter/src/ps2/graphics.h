// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * VIF1, VU1, the GIF and the GS connected as they are on the board.
 *
 * It holds the units and the callbacks between them, and nothing else. Path 3 of the GIF is not
 * wired here: whoever drives the DMA writes to `gif` on path 3.
 *
 * Sources: how the units are wired, as publicly documented.
 */

#pragma once

#include <algorithm>

#include "fp_quad.h"
#include "gif.h"
#include "gs.h"
#include "vif.h"
#include "vu.h"

namespace ps2 {

/**
 * The drawing side of the machine, connected as it is on the board: VIF1
 * feeds VU1's memories and starts its programs, VU1 kicks GIF packets out of
 * its data memory on path 1, VIF1 passes packets on path 2, and both end at
 * the GS.
 *
 * VU1 runs inside `Vif1::write`, so the callbacks set up here run on the thread that calls it.
 */
struct Graphics {
    /** The Graphics Synthesizer. */
    Gs gs;

    /** The GIF in front of it. */
    Gif gif{gs};

    /** VIF1, which passes DIRECT packets to the GIF. */
    Vif1 vif{gif};

    /** VU1, running on the memories that VIF1 fills. */
    Vu vu1{Vu::Memory{vif.micro.data(), Vif1::kMemoryBytes, vif.data.data(), Vif1::kMemoryBytes}};

    /** Statistics: instructions VU1 ran, programs started, and programs that did not stop. */
    u64 vu1_instructions = 0, vu1_starts = 0, vu1_runaways = 0;

    /** Where the last program that did not stop was started, and where it was when cut off. */
    u32 vu1_runaway_start = 0, vu1_runaway_pc = 0;

    /** Connects the units: the callbacks of VIF1 and VU1 are set to call each other. */
    Graphics() {
        // Called by VIF1 when MSCAL, MSCALF or MSCNT starts a program, on the thread that feeds it.
        vif.on_start = [this](u32 address, bool resume) {
            // No real program runs this long between two stops; one that does has
            // gone wrong here.
            const u64 limit = 4'000'000;
            vu1_instructions += resume ? vu1.resume(limit) : vu1.run(address, limit);
            fp::want_nearest();  // the program left the host rounding towards zero
            vu1_starts++;

            // The program was cut off at the limit instead of ending: count it, keep where.
            if (!vu1.stopped()) {
                vu1_runaways++;
                vu1_runaway_start = address;
                vu1_runaway_pc = vu1.pc;
            }
        };

        // Called by VIF1 after MPG has written program memory.
        vif.on_program = [this] {
            vu1.program_changed();
        };

        // Nothing outside VU1 reads its flags, so it need not work out the ones no program reads.
        vu1.skip_unread_flags = true;

        // Called by VU1 for XTOP and XITOP, which read the VIF's registers.
        vu1.on_top = [this] {
            return vif.top;
        };
        vu1.on_itop = [this] {
            return vif.itop;
        };

        // Called by VU1 for XGKICK, with the quadword of data memory a GIF packet starts at.
        vu1.on_kick = [this](u32 quadword) {
            /*
             * A packet runs to the end of the data its last tag (the one with EOP)
             * announces, and wraps at the end of data memory.
             */
            const u32 mask = Vif1::kMemoryBytes / 16 - 1;
            u32 at = quadword & mask;
            std::size_t count = Gif::packet_quadwords(vif.data.data(), at, mask);

            // The part of the packet before the end of data memory.
            std::size_t first = std::min<std::size_t>(count, mask + 1 - at);
            gif.write(1, &vif.data[at * 16], first);

            // The rest wraps round to the start of data memory.
            if (count > first) {
                gif.write(1, vif.data.data(), count - first);
            }
        };
    }

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
};

}  // namespace ps2
