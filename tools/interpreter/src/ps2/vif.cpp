// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The VIF1 declared in vif.h: command decoding, UNPACK, MPG and microprogram starts.
 *
 * Sources: the VIF code formats, the UNPACK formats, write cycles, masks and modes, and the double
 * buffer as publicly documented.
 */

#include "vif.h"

#include <algorithm>

namespace ps2 {
namespace {

/** The CMD field of a VIF code, bits 24-30, for the commands this model knows (documented). */
enum : u32 {
    kNop = 0x00,
    kStcycl = 0x01,
    kOffset = 0x02,
    kBase = 0x03,
    kItop = 0x04,
    kStmod = 0x05,
    kMskpath3 = 0x06,
    kMark = 0x07,
    kFlushe = 0x10,
    kFlush = 0x11,
    kFlusha = 0x13,
    kMscal = 0x14,
    kMscalf = 0x15,
    kMscnt = 0x17,
    kStmask = 0x20,
    kStrow = 0x30,
    kStcol = 0x31,
    kMpg = 0x4A,
    kDirect = 0x50,
    kDirecthl = 0x51,
};

/**
 * Returns the CMD field of a VIF code.
 *
 * @param code The VIF code, the first word of a command.
 * @return Bits 24-30 of the code; the IRQ bit 31 is dropped.
 */
inline u32 command(u32 code) {
    return (code >> 24) & 0x7F;
}

/**
 * Returns the NUM field of a VIF code, where 0 stands for 256.
 *
 * @param code The VIF code.
 * @return Bits 16-23 of the code, or 256 when they are 0 (documented).
 */
inline u32 count(u32 code) {
    u32 n = (code >> 16) & 0xFF;
    return n ? n : 256;
}

/**
 * Tells whether a VIF code is an UNPACK command.
 *
 * @param code The VIF code.
 * @return True for CMD 0x60-0x7F, where bits 5 and 6 are both set (documented).
 */
inline bool is_unpack(u32 code) {
    return (command(code) & 0x60) == 0x60;
}

/**
 * Returns the bytes one vector takes in the list.
 *
 * It is `vn + 1` elements of 32, 16 or 8 bits, or 16 bits in all for the 5:5:5:1 colour format.
 *
 * @param code The VIF code of an UNPACK command.
 * @return The size of one vector in bytes.
 */
inline u32 vector_bytes(u32 code) {
    // UNPACK CMD bits 2-3 are VN (elements minus one) and bits 0-1 are VL (element size).
    u32 vn = (command(code) >> 2) & 3, vl = command(code) & 3;
    return (vn == 3 && vl == 3) ? 2 : (vn + 1) * (4u >> vl);
}

}  // namespace

void Vif1::reset() {
    // Power-on state: memories and registers zero, except the cycle lengths, which are 1.
    data.fill(0);
    micro.fill(0);
    cl = wl = 1;
    mode = mask = 0;
    row.fill(0);
    col.fill(0);
    base = ofst = tops = top = itops = itop = mark = 0;
    dbf = false;
    path3_masked = false;
    pending_.clear();
}

std::size_t Vif1::operand_words(u32 code) const {
    u32 cmd = command(code);

    // UNPACK has as much data as its vectors need.
    if (is_unpack(code)) {
        /*
         * With WL <= CL every vector written comes from the list. With WL > CL
         * only the first CL of every WL do; the rest are filled in.
         */
        u32 n = count(code);
        u32 from_list = (wl <= cl || wl == 0) ? n : cl * (n / wl) + std::min(n % wl, cl);

        // Rounded up to whole words.
        return (from_list * vector_bytes(code) + 3) / 4;
    }

    switch (cmd) {
        // One word: the mask.
        case kStmask:
            return 1;

        // Four words: one for each field of the register.
        case kStrow:
        case kStcol:
            return 4;

        // NUM instructions of 8 bytes, two words each.
        case kMpg:
            return count(code) * 2;

        // IMMEDIATE quadwords of GIF data, 0 meaning 65536 of them, four words each.
        case kDirect:
        case kDirecthl: {
            u32 quadwords = code & 0xFFFF;
            return (quadwords ? quadwords : 65536) * 4;
        }

        // No data follows the other commands.
        default:
            return 0;
    }
}

void Vif1::write(const u8* bytes, std::size_t size) {
    // Add the new words to those left over from the last call; a partial word is dropped.
    std::size_t words = size / 4;
    std::size_t old = pending_.size();
    pending_.resize(old + words);
    std::memcpy(pending_.data() + old, bytes, words * 4);

    std::size_t at = 0;

    // Run every command that is complete; the loop ends at the first one still waiting for data.
    while (at < pending_.size()) {
        u32 code = pending_[at];
        std::size_t operands = operand_words(code);

        // The rest of the command has not arrived: keep it for the next call.
        if (at + 1 + operands > pending_.size()) {
            break;
        }

        execute(code, pending_.data() + at + 1, operands);
        at += 1 + operands;
    }

    pending_.erase(pending_.begin(), pending_.begin() + static_cast<std::ptrdiff_t>(at));
}

void Vif1::start(u32 address, bool resume) {
    /*
     * Starting a program hands it the buffer just filled and moves the VIF on
     * to the other one of the pair.
     */
    itop = itops;
    top = tops;

    // The double buffer: the next buffer is at BASE, or at BASE + OFFSET, alternately.
    if (dbf) {
        tops = base;
        dbf = false;
    } else {
        // The sum wraps at 10 bits, the 1,024 quadwords of data memory.
        tops = (base + ofst) & 0x3FF;
        dbf = true;
    }

    // Nothing is listening: the program is not run.
    if (on_start) {
        on_start(address, resume);
    }
}

void Vif1::execute(u32 code, const u32* operands, std::size_t words) {
    // IMMEDIATE, bits 0-15 of the code, carries the command's argument (documented).
    u32 imm = code & 0xFFFF;

    // UNPACK commands are decoded on their own.
    if (is_unpack(code)) {
        unpack(code, operands, words);
        return;
    }

    switch (command(code)) {
        case kNop:
        case kFlushe:
        case kFlush:
        case kFlusha:
            // Everything here completes before the next command is read, so there
            // is nothing to wait for.
            break;

        // CL is bits 0-7 of the immediate, WL bits 8-15 (documented).
        case kStcycl:
            cl = imm & 0xFF;
            wl = imm >> 8;
            break;

        // OFFSET is 10 bits; it also restarts the double buffer at BASE (documented).
        case kOffset:
            ofst = imm & 0x3FF;
            dbf = false;
            tops = base;
            break;

        // BASE is 10 bits.
        case kBase:
            base = imm & 0x3FF;
            break;

        // ITOP is 10 bits.
        case kItop:
            itops = imm & 0x3FF;
            break;

        // The mode is bits 0-1: 0 normal, 1 offset, 2 difference.
        case kStmod:
            mode = imm & 3;
            break;

        // Bit 15 set masks path 3.
        case kMskpath3:
            path3_masked = (imm & 0x8000) != 0;
            break;

        // MARK keeps the whole 16-bit immediate.
        case kMark:
            mark = imm;
            break;

        // The immediate is the start address in instructions; MSCALF is the same here.
        case kMscal:
        case kMscalf:
            start(imm, false);
            break;

        // Continues from where the last program stopped.
        case kMscnt:
            start(0, true);
            break;

        // The operand word is the MASK register (documented).
        case kStmask:
            mask = operands[0];
            break;

        // The four operand words are the fields x, y, z, w of ROW.
        case kStrow:
            std::copy(operands, operands + 4, row.begin());
            break;

        // The four operand words are the fields x, y, z, w of COL.
        case kStcol:
            std::copy(operands, operands + 4, col.begin());
            break;

        case kMpg: {
            // `imm` is the load address in instructions of 8 bytes; the program
            // memory wraps.
            std::size_t at = (static_cast<std::size_t>(imm) * 8) & (kMemoryBytes - 1);

            // The words of the command arrive as host words; their bytes are the program's.
            const u8* src = reinterpret_cast<const u8*>(operands);
            for (std::size_t i = 0; i < words * 4; i++) {
                micro[(at + i) & (kMemoryBytes - 1)] = src[i];
            }

            // The microprogram changed: whoever runs it has to forget what it decoded.
            if (on_program) {
                on_program();
            }
            break;
        }

        // The data is GIF packets, quadwords: they go to the GIF on path 2.
        case kDirect:
        case kDirecthl:
            gif_.write(2, reinterpret_cast<const u8*>(operands), words / 4);
            break;

        // A code the model does not know: count it and carry on.
        default:
            unknown_codes++;
            break;
    }
}

void Vif1::unpack(u32 code, const u32* operands, std::size_t words) {
    /*
     * UNPACK: CMD bits 0-1 are VL (element size), bits 2-3 VN (elements minus one), bit 4 M (the
     * mask applies). IMMEDIATE bits 0-9 are ADDR, bit 14 USN (unsigned), bit 15 FLG (ADDR is
     * relative to TOPS) (documented).
     */
    u32 cmd = command(code), imm = code & 0xFFFF;
    u32 vn = (cmd >> 2) & 3, vl = cmd & 3;
    bool masked = (cmd & 0x10) != 0;
    bool is_unsigned = (imm & 0x4000) != 0;
    u32 address = imm & 0x3FF;
    if (imm & 0x8000) {
        address += tops;
    }

    const u8* src = reinterpret_cast<const u8*>(operands);
    const u8* end = src + words * 4;
    u32 bytes = vector_bytes(code);

    // Widens one element to 32 bits, sign-extended unless USN; past the end it reads as 0.
    auto element = [&](const u8* p) -> u32 {
        // The data of the command ran out before the vectors did.
        if (p >= end) {
            return 0;
        }

        switch (vl) {
            case 0:
                return load<u32>(p);

            case 1:
                return is_unsigned ? u32{load<u16>(p)}
                                   : static_cast<u32>(static_cast<s32>(load<s16>(p)));

            // 8-bit elements; VL 3 is the 5:5:5:1 format, which does not come here.
            default:
                return is_unsigned ? u32{*p}
                                   : static_cast<u32>(static_cast<s32>(static_cast<s8>(*p)));
        }
    };

    // Bytes from one element to the next.
    u32 step = 4u >> vl;

    u32 total = count(code);
    u32 cycle = 0;

    // One pass writes one vector; `cycle` counts the writes of the current write cycle.
    for (u32 n = 0; n < total; n++) {
        // With WL > CL, the writes after the first CL of each cycle take nothing from the list.
        bool from_list = wl <= cl || cycle < cl;
        std::array<u32, 4> in{};

        if (from_list) {
            // V4-5: R, G, B take 5 bits each, shifted up by 3; A is 1 bit, put at bit 7.
            if (vn == 3 && vl == 3) {
                u32 c = load<u16>(src);
                in =
                    {(c & 0x1F) << 3,
                     ((c >> 5) & 0x1F) << 3,
                     ((c >> 10) & 0x1F) << 3,
                     ((c >> 15) & 1) << 7};
            } else if (vn == 0) {
                // One element: it goes to all four fields.
                in.fill(element(src));
            } else if (vn == 1) {
                // The two fields that are not in the list repeat the two that are.
                u32 x = element(src), y = element(src + step);
                in = {x, y, x, y};
            } else {
                // A three-element vector's W is whatever follows it in the list.
                for (u32 f = 0; f < 4; f++) {
                    in[f] = element(src + f * step);
                }
            }
            src += bytes;
        }

        // The address wraps at VU1's 1,024 quadwords.
        u8* dst = &data[(address & 0x3FF) * 16];

        // The first three writes of a cycle have a COL entry and mask bits of their own.
        u32 line = std::min<u32>(cycle, 3);

        for (u32 f = 0; f < 4; f++) {
            // MASK: two bits for each of the 16 fields (4 lines of x, y, z, w), low field first.
            u32 m = masked ? (mask >> ((line * 4 + f) * 2)) & 3 : 0;
            u32 value;

            switch (m) {
                // Not masked: the data, changed by the mode (1 adds ROW; 2 adds it and keeps the
                // sum in ROW).
                case 0:
                    value = in[f];
                    if (mode == 1) {
                        value += row[f];
                    } else if (mode == 2) {
                        value += row[f];
                        row[f] = value;
                    }
                    break;

                // Masked to ROW.
                case 1:
                    value = row[f];
                    break;

                // Masked to COL.
                case 2:
                    value = col[line];
                    break;

                // Mask 3 protects the field from writing.
                default:
                    continue;  // write protected
            }
            store<u32>(dst + f * 4, value);
        }

        address++;
        cycle++;

        // CL >= WL: after WL writes the address skips ahead to the next cycle's start.
        if (wl <= cl) {
            if (cycle == wl) {
                address += cl - wl;
                cycle = 0;
            }
        } else if (cycle == wl) {
            // CL < WL: the write cycle is complete, the filling writes included.
            cycle = 0;
        }
    }
}

}  // namespace ps2
