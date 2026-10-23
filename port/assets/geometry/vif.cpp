// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/vif.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// VIF1 command-list framing (ReRAC docs/formats/tfrag_rac1.md 2.1).

#include "assets/geometry/vif.h"

namespace openrac::assets::vif {

std::vector<Code> parse(ByteView list) {
    std::vector<Code> out;
    std::size_t pos = 0;

    while (pos + 4 <= list.size()) {
        const u32 word = list.u32_at(pos);
        Code code;
        code.cmd = static_cast<u8>((word >> 24) & 0x7f);
        code.num = static_cast<u8>((word >> 16) & 0xff);
        code.imm = static_cast<u16>(word & 0xffff);
        code.offset = pos;
        std::size_t payload = 0;

        if (code.is_unpack()) {
            payload = (std::size_t{code.count()} * code.element_size() + 3) & ~std::size_t{3};
        } else {
            switch (code.cmd) {
                case kStmask:
                    payload = 4;
                    break;
                case kStrow:
                case kStcol:
                    payload = 16;
                    break;
                case kMpg:
                    payload = std::size_t{code.count()} * 8;
                    break;
                case kDirect:
                case kDirecthl:
                    payload = (code.imm == 0 ? std::size_t{65536} : std::size_t{code.imm}) * 16;
                    break;
                default:
                    break;
            }
        }

        if (pos + 4 + payload > list.size()) {
            fail("VIF code {:#x} at {:#x} runs past the end of its list", code.cmd, pos);
        }

        code.data = list.sub(pos + 4, payload, "VIF payload");
        out.push_back(code);
        pos += 4 + payload;
    }

    return out;
}

}  // namespace openrac::assets::vif
