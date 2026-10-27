// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The vector unit's interpreter: running pairs, the timing rules, the registers and every
 * instruction.
 *
 * `step` runs one pair. Before it runs, `work_out` decides once per pair what the pair reads that
 * can make it wait and which function runs each half; the answer is kept in an `Image` of the
 * program memory. The instructions are written as four switches (`upper_body`,
 * `upper_special_body`, `lower_body`, `lower_special_body`), and each pair gets a function that is
 * one of those with its selector fixed, so nothing is decoded while a program runs.
 *
 * The four-field arithmetic needs the host to round towards zero while a
 * unit computes. Every way in asks for it (fp::want_toward_zero) and none
 * switches back: whoever computes as the host does asks for its own mode.
 *
 * Sources: the vector unit manual as publicly documented, and what the games' own microprograms
 * rely on. Facts tagged (documented) are in the manual; (measured) ones were found by tests of
 * this model.
 */

#include "vu.h"

#include <cfenv>
#include <cmath>
#include <cstring>

#include "fp.h"
#include "fp_quad.h"

namespace ps2 {
namespace {

/** The dest mask of x, y and z together: bits 3, 2 and 1. */
constexpr u32 kXyz = 14;

/**
 * Maps the problems a division raised to the divider's two status bits.
 *
 * The divider's two status bits are invalid (bit 4) and divide by zero (bit 5) (documented).
 *
 * @param problems The `fp` flags the division raised.
 * @return The status bits to set when the result arrives.
 */
inline u32 divide_flags(u32 problems) {
    return ((problems & fp::kInvalid) ? 0x10u : 0u) | ((problems & fp::kDivideByZero) ? 0x20u : 0u);
}

/**
 * Sign-extends a field.
 *
 * @param v The value; only its low `width` bits count.
 * @param width Number of bits in the field.
 * @return The field as a signed number.
 */
inline s32 sign_extend(u32 v, unsigned width) {
    // Flip the field's sign bit and subtract it: that carries the sign into the upper bits.
    u32 m = 1u << (width - 1);
    return static_cast<s32>((v ^ m) - m);
}

/**
 * Tells whether a dest mask selects a field.
 *
 * @param dest The mask: x is bit 3 down to w in bit 0 (documented).
 * @param field The field, 0 for x to 3 for w.
 * @return True if the field is selected.
 */
inline bool has(u32 dest, unsigned field) {
    return (dest & (8u >> field)) != 0;
}

/**
 * How long each function unit takes: the instruction this many later sees
 * the result (documented).
 */
constexpr unsigned kDiv = 7, kSqrt = 7, kRsqrt = 13;

/** Use is counted for each 256 pairs of program memory: a pair's address shifted by this. */
constexpr unsigned kUseShift = 8;

}  // namespace

Vu::Vu(Memory memory) : memory_(memory), pc_mask_(memory.micro_bytes / 8 - 1) {
    reset();
}

void Vu::reset() {
    // Power-on state: everything zero except the registers below that are constant or start set.
    for (auto& reg : vf) {
        reg.fill(0);
    }

    // VF0 reads as (0, 0, 0, 1) whatever a program does to it (documented).
    vf[0][3] = as_u32(1.0f);
    vi.fill(0);
    acc.fill(0);
    q = p = i = 0;

    // R holds 1.0 with a random mantissa: the exponent bits are fixed at 0x3F800000 (documented).
    r = 0x3F800000;
    mac = status = clip = 0;
    mac_latest_ = status_latest_ = clip_latest_ = 0;
    flag_first_ = flag_count_ = 0;
    q_at_ = p_at_ = 0;
    cycle_ = 0;
    readable_ = {};
    register_ready_ = {};
    branch_in_ = stop_in_ = kick_in_ = 0;
    backup_ttl_ = 0;
    pc = 0;
    running_ = false;
}

// --- running ---

u64 Vu::run(u32 address, u64 limit) {
    // The address wraps at the size of program memory.
    pc = address & pc_mask_;
    straight_ = 0;
    branch_in_ = stop_in_ = 0;
    count_start();
    return resume(limit);
}

void Vu::start(u32 address) {
    pc = address & pc_mask_;
    straight_ = 0;
    branch_in_ = stop_in_ = 0;
    running_ = true;
    count_start();
}

void Vu::count_start() {
    // Program memory was written since the last pair: the start counts under its new content.
    if (program_dirty_) {
        choose_image();
    }

    image_->starts[pc >> kUseShift]++;
}

std::vector<Vu::Use> Vu::uses() const {
    std::vector<Use> out;

    for (const auto& [hash, image] : images_) {
        out.push_back(Use{image->micro, image->pairs, image->starts});
    }

    return out;
}

u64 Vu::advance(u64 instructions) {
    // The unit computes with the console's rounding while it runs (documented).
    fp::want_toward_zero();
    u64 count = 0;

    // Stop early if the program ends before the budget is spent.
    while (running_ && count < instructions) {
        step();
        count++;
    }
    return count;
}

u64 Vu::advance_to_sync(u64 limit) {
    fp::want_toward_zero();
    u64 count = 0;
    sync_point_ = false;

    // Stop at the end of the program, after a pair with the M bit, or when the limit is reached.
    while (running_ && !sync_point_ && count < limit) {
        step();
        count++;
    }
    return count;
}

u64 Vu::resume(u64 limit) {
    fp::want_toward_zero();
    running_ = true;
    u64 count = 0;

    // Stop at the end of the program or when the limit is reached.
    while (running_ && count < limit) {
        step();
        count++;
    }
    return count;
}

void Vu::finish_q() {
    q = q_next_;
    q_at_ = 0;

    // The divider's two flags (invalid, divide by zero) arrive with its result.
    // The lambda below is called in this function only, on each place a status value is kept.
    auto apply = [this](u32& s) {
        // Status bits 4-5 are the current flags and bits 10-11 the remembered ones (documented).
        s = (s & ~0x30u) | q_flags_ | (q_flags_ << 6);
    };
    apply(status);
    apply(status_latest_);

    // Flags still on their way get the divider's bits too.
    for (Flags& f : flag_pipe_) {
        apply(f.status);
    }
}

void Vu::fire_kick() {
    kick_in_ = 0;

    // Nobody listening: the packet is dropped.
    if (on_kick) {
        // What takes the packet (the GS) computes with the usual rounding.
        on_kick(kick_address_);
        fp::want_toward_zero();
    }
}

void Vu::work_out(Needs& needs, u32 up, u32 low) const {
    needs = Needs{};
    needs.up = up;
    needs.low = low;
    needs.known = true;

    // Called below for each float register the pair reads: record it with the fields it needs.
    // At most four are kept, since a pair reads that many at most; register 0 never waits.
    auto read = [&](unsigned reg, u32 mask) {
        if (reg && mask && needs.count < 4) {
            needs.reg[needs.count] = static_cast<u8>(reg);
            needs.mask[needs.count] = static_cast<u8>(mask);
            needs.count++;
        }
    };

    // Upper word: dest bits 21-24, ft 16-20, fs 11-15, function 0-5 (documented).
    u32 dest = (up >> 21) & 0xF;
    unsigned ft = (up >> 16) & 31, fs = (up >> 11) & 31;
    u32 fn = up & 0x3F;

    // The upper instruction reads fs and ft in the fields its dest names, or one broadcast field.
    if (fn < 0x1C) {
        // Functions 0x00-0x1B take one broadcast field of ft, named by the low two bits.
        read(fs, dest);
        read(ft, 8u >> (fn & 3));
    } else if (fn < 0x28) {
        // Functions 0x1C-0x27 take Q or I as the second operand: only fs is read.
        read(fs, dest);
    } else if (fn < 0x30) {
        // Functions 0x28-0x2F take ft in the same fields as fs.
        read(fs, dest);
        read(ft, dest);
    } else if (fn >= 0x3C) {
        // The special group: the index is bits 6-10 above bits 0-1.
        u32 index = (((up >> 6) & 0x1F) << 2) | (up & 3);

        if (index < 0x10 || (index >= 0x18 && index < 0x1C)) {
            // The accumulator forms with a broadcast field of ft.
            read(fs, dest);
            read(ft, 8u >> (index & 3));
        } else if (index == 0x1F) {  // CLIP
            // CLIP reads x, y and z of fs (mask 14) and the w of ft (mask 1).
            read(fs, 14);
            read(ft, 1);
        } else if (index >= 0x28 && index < 0x2F) {
            // The accumulator forms with a full ft.
            read(fs, dest);
            read(ft, dest);
        } else if (index != 0x2F) {
            // The rest read fs only; 0x2F is the NOP and reads nothing.
            read(fs, dest);
        }
    }

    // With the I bit the lower word is a number, so it reads nothing.
    if (!(up & 0x80000000u)) {
        // Lower word: opcode bits 25-31, dest 21-24, it 16-20, is 11-15 (documented).
        u32 op = low >> 25;
        u32 ldest = (low >> 21) & 0xF;
        unsigned it = (low >> 16) & 31, is = (low >> 11) & 31;

        // Field selectors of the divider and function unit, as masks: fsf bits 21-22, ftf 23-24.
        u32 fsf = 8u >> ((low >> 21) & 3), ftf = 8u >> ((low >> 23) & 3);

        if (op == 0x01) {  // SQ
            // SQ stores fs, which sits in the `is` field.
            read(is, ldest);
        } else if (op == 0x40 && (low & 0x3F) >= 0x3C) {
            // The special group with a 7-bit index: the instructions that read float registers.
            switch ((((low >> 6) & 0x1F) << 2) | (low & 3)) {
                case 0x30:  // MOVE
                    read(is, ldest);
                    break;

                case 0x31:  // MR32
                    read(is, 15);
                    break;

                case 0x35:  // SQI
                case 0x37:  // SQD
                    read(is, ldest);
                    break;

                case 0x38:  // DIV
                case 0x3A:  // RSQRT
                    // Two fields in, and the result needs the divider to be free.
                    read(is, fsf);
                    read(it, ftf);
                    needs.wait = 1;
                    break;

                case 0x39:  // SQRT
                    read(it, ftf);
                    needs.wait = 1;
                    break;

                case 0x3B:  // WAITQ
                    needs.wait = 1;
                    break;

                case 0x3C:  // MTIR
                case 0x42:  // RINIT
                case 0x43:  // RXOR
                    read(is, fsf);
                    break;

                case 0x70:  // ESADD
                case 0x71:  // ERSADD
                case 0x72:  // ELENG
                case 0x73:  // ERLENG
                case 0x74:  // EATANxy
                case 0x75:  // EATANxz
                    // These use x, y and z of fs.
                    read(is, 14);
                    break;

                case 0x76:  // ESUM
                    // ESUM adds all four fields.
                    read(is, 15);
                    break;

                case 0x78:  // ESQRT
                case 0x79:  // ERSQRT
                case 0x7A:  // ERCPR
                case 0x7C:  // ESIN
                case 0x7D:  // EATAN
                case 0x7E:  // EEXP
                    read(is, fsf);
                    break;

                case 0x7B:  // WAITP
                    needs.wait = 2;
                    break;

                default:
                    // The rest read no float register and wait for nothing.
                    break;
            }
        }
    }

    // How the two halves are run: the slot in the tables of `upper_runs` and `lower_runs`.
    needs.upper_run = kUpperRuns[fn < 0x3C ? fn : 64 + ((((up >> 6) & 0x1F) << 2) | (up & 3))];
    {
        u32 op = low >> 25, lfn = low & 0x3F;

        // Lower slots: the operation field; for the special group 128 plus the function field
        // below 0x3C, else 192 plus the index.
        needs.lower_run = kLowerRuns
            [op != 0x40   ? op
             : lfn < 0x3C ? 128 + lfn
                          : 192 + ((((low >> 6) & 0x1F) << 2) | (low & 3))];
    }

    // An upper NOP is the special index 0x2F (low 11 bits 0x2FF).
    needs.upper_nop = (up & 0x7FF) == 0x2FF;

    // The lower NOP is MOVE of nothing, the usual filler; with the I bit the lower word is a number.
    needs.lower_nop = !(up & 0x80000000u) && low == 0x8000033Cu;

    // The float register the upper instruction writes: fd for the arithmetic
    // that has one, ft for the conversions and ABS, none for ACC and CLIP.
    unsigned written = 0;
    if (fn < 0x30) {
        written = (up >> 6) & 31;
    } else if (fn >= 0x3C) {
        u32 index = (((up >> 6) & 0x1F) << 2) | (up & 3);

        // Indexes 0x10-0x17 are the conversions and 0x1D is ABS: they write ft.
        if ((index >= 0x10 && index < 0x18) || index == 0x1D) {
            written = ft;
        }
    } else {
        // Not an instruction: take the careful way, as if any register might be written.
        written = 32;
    }

    // Any lower instruction names its float registers in these two places.
    needs.together =
        written != 0
        && (written == 32 || ((low >> 16) & 31) == written || ((low >> 11) & 31) == written);
}

bool Vu::never_waits(const Needs& needs, u32 at) const {
    // Three pairs back is as far as a register can still be in flight: a write is readable
    // four cycles later (documented).
    for (u32 back = 1; back <= 3; back++) {
        u32 where = (at - back) & pc_mask_;
        u32 low = load<u32>(memory_.micro + where * 8),
            up = load<u32>(memory_.micro + where * 8 + 4);
        unsigned written[2] = {0, 0};
        u32 fn = up & 0x3F;

        // Which float register the upper instruction might write: fd, or ft for the conversions.
        if (fn < 0x30) {
            written[0] = (up >> 6) & 31;
        } else if (fn >= 0x3C) {
            u32 index = (((up >> 6) & 0x1F) << 2) | (up & 3);

            // Indexes 0x10-0x17 are the conversions and 0x1D is ABS: they write ft.
            if ((index >= 0x10 && index < 0x18) || index == 0x1D) {
                written[0] = (up >> 16) & 31;
            }
        } else {
            // Not an instruction: take the careful way and say the pair may wait.
            return false;
        }

        // The lower instruction may write ft: the loads (opcode 0) and the special group.
        if (!(up & 0x80000000u)) {
            u32 op = low >> 25;
            if (op == 0x00 || (op == 0x40 && (low & 0x3F) >= 0x3C)) {
                written[1] = (low >> 16) & 31;
            }
        }

        // A register this pair reads was possibly written there: it may have to wait.
        for (unsigned n = 0; n < needs.count; n++) {
            if (needs.reg[n] == written[0] || needs.reg[n] == written[1]) {
                return false;
            }
        }
    }

    return true;
}

void Vu::program_changed() {
    program_dirty_ = true;
}

void Vu::status_was_read() {
    // Only needed while flags that nobody reads are being skipped.
    if (skip_unread_flags) {
        skip_unread_flags = false;

        // What was worked out assumed nobody would.
        images_.clear();
        image_ = nullptr;
        needs_ = nullptr;
        program_dirty_ = true;
    }
}

void Vu::choose_image() {
    program_dirty_ = false;

    // A hash of program memory: FNV-1a over 64-bit words, from its offset basis and prime.
    u64 sum = 0xCBF29CE484222325ull;
    for (u32 n = 0; n < memory_.micro_bytes; n += 8) {
        sum = (sum ^ load<u64>(memory_.micro + n)) * 0x100000001B3ull;
    }

    std::unique_ptr<Image>& slot = images_[sum];

    // The hash matches but the content differs: forget the old image and start a new one.
    if (slot && std::memcmp(slot->micro.data(), memory_.micro, memory_.micro_bytes) != 0) {
        slot.reset();
    }

    // No image for this content yet.
    if (!slot) {
        // Too many images are kept (256 is more than any game swaps): start again from none.
        if (images_.size() > 256) {
            // (`slot` is gone with it)
            images_.clear();
            choose_image();
            return;
        }

        slot = std::make_unique<Image>();
        slot->micro.assign(memory_.micro, memory_.micro + memory_.micro_bytes);
        slot->needs.resize(memory_.micro_bytes / 8);
        slot->pairs.assign((memory_.micro_bytes / 8) >> kUseShift, 0);
        slot->starts.assign((memory_.micro_bytes / 8) >> kUseShift, 0);
    }
    image_ = slot.get();
    needs_ = image_->needs.data();
    pairs_run_ = image_->pairs.data();
    sticky_readers_ = image_->sticky_readers;
}

namespace {

// Does this upper instruction set MAC and status flags?
inline bool sets_flags(u32 up) {
    u32 fn = up & 0x3F;

    // The arithmetic sets flags; the maximum and minimum (0x10-0x17, 0x1D, 0x1F, 0x2B, 0x2F) do not.
    if (fn < 0x10 || (fn >= 0x18 && fn <= 0x1C) || fn == 0x1E || (fn >= 0x20 && fn <= 0x2A)
        || (fn >= 0x2C && fn <= 0x2E)) {
        return true;
    }

    // The other functions below the special group do not.
    if (fn < 0x3C) {
        return false;
    }

    // The special group: the same set of arithmetic, by its 7-bit index.
    u32 index = (((up >> 6) & 0x1F) << 2) | (up & 3);
    return index < 0x10 || (index >= 0x18 && index <= 0x1C) || index == 0x1E
           || (index >= 0x20 && index <= 0x2A) || (index >= 0x2C && index <= 0x2E);
}

// Lower instructions that read the MAC or status flags.
inline bool reads_flags(u32 low) {
    // FSEQ, FSAND, FSOR (status), FMEQ, FMAND, FMOR (MAC): the opcodes 0x14, 0x16-0x18, 0x1A-0x1B.
    u32 op = low >> 25;
    return op == 0x14 || op == 0x16 || op == 0x17 || op == 0x18 || op == 0x1A || op == 0x1B;
}

}  // namespace

// What is in program memory as a whole: does anything read flags in a way
// that depends on every instruction (the bits that remember, the MAC flags)?
void Vu::look_at_programs() {
    image_->looked_at = true;
    image_->sticky_readers = sticky_readers_ = false;

    // Every pair of program memory; the loop ends at the last one or at the first reader found.
    for (u32 n = 0; n <= pc_mask_; n++) {
        u32 low = load<u32>(memory_.micro + n * 8), up = load<u32>(memory_.micro + n * 8 + 4);

        // The lower word is a number for I, not an instruction.
        if (up & 0x80000000u) {
            continue;
        }

        u32 op = low >> 25;
        u32 imm12 = ((low >> 10) & 0x800) | (low & 0x7FF);

        // FSSET, FMEQ, FMAND and FMOR read the MAC flags or the remembered bits; FSEQ, FSAND and
        // FSOR do when their mask has any of the remembered status bits 6-11 (0xFC0).
        if (op == 0x15 || op == 0x18 || op == 0x1A || op == 0x1B
            || ((op == 0x14 || op == 0x16 || op == 0x17) && (imm12 & 0xFC0))) {
            image_->sticky_readers = sticky_readers_ = true;
            return;
        }
    }
}

// Can any instruction read the flags the upper instruction at `at` sets?
// They are seen from four cycles on and until the next instruction that
// sets flags has had its four cycles. A reader in that stretch, or anything
// that leaves the straight line (a branch, the end of the program), counts.
bool Vu::flags_can_be_read(u32 at) const {
    // Skipping is off, or something reads the remembered bits: every flag may be read.
    if (!skip_unread_flags || sticky_readers_) {
        return true;
    }

    int next_setter = -1;

    // Look at the pairs after this one; the loop ends with an answer, or after 40 pairs.
    // (From the pair before: if that one branches, this is its delay slot and
    // what follows is somewhere else. The pair itself is looked at only for a
    // branch.)
    for (int n = -1; n <= 40; n++) {
        u32 where = (at + static_cast<u32>(n)) & pc_mask_;
        u32 low = load<u32>(memory_.micro + where * 8),
            up = load<u32>(memory_.micro + where * 8 + 4);

        // The E bit (bit 30): the program ends, and what it leaves behind is always worked out.
        if (up & 0x40000000u) {
            return true;
        }

        if (!(up & 0x80000000u)) {
            u32 op = low >> 25;

            // A flag reader after this pair, or a branch or jump (opcodes 0x20-0x2F), which
            // leaves the straight line.
            if ((n > 0 && reads_flags(low)) || (op >= 0x20 && op <= 0x2F)) {
                return true;
            }
        }

        // The pair before and this one itself only count for what they branch to.
        if (n <= 0) {
            continue;
        }

        if (next_setter < 0 && sets_flags(up)) {
            next_setter = n;
        }

        // Five more pairs after the next setter: its four cycles, and one over.
        if (next_setter >= 0 && n >= next_setter + 5) {
            return false;
        }
    }

    // Nothing decided within 40 pairs: assume it may be read.
    return true;
}

void Vu::step() {
    // Program memory was written since the last pair: find the image for its new content.
    if (program_dirty_) [[unlikely]] {
        choose_image();
    }

    u32 at = pc;
    pc = (pc + 1) & pc_mask_;
    Needs& needs = needs_[at];

    pairs_run_[at >> kUseShift]++;

    // The first time this pair runs with this content, work out what it needs.
    if (!needs.known) [[unlikely]] {
        // The image as a whole has not been looked at yet.
        if (!image_->looked_at) {
            look_at_programs();
        }

        u32 low_word = load<u32>(memory_.micro + at * 8),
            up_word = load<u32>(memory_.micro + at * 8 + 4);
        work_out(needs, up_word, low_word);
        needs.flags_wanted = !sets_flags(up_word) || flags_can_be_read(at);
        needs.no_wait = never_waits(needs, at);
    }
    const u32 low = needs.low, up = needs.up;

    // A tool is watching each pair.
    if (on_step) [[unlikely]] {
        on_step(at, up, low);
    }

    // When this pair runs: the next cycle, or later if it reads a float
    // register not yet readable, or needs a unit that is busy.
    flags_wanted_ = needs.flags_wanted;
    u64 ready = cycle_ + 1;

    // (A register can only be in flight from one of the last three pairs run.)
    // A pair none of whose registers was written in those three, reached in a straight line,
    // cannot wait for one.
    if (!needs.no_wait || straight_ < 3) {
        // Each float register the pair reads.
        for (unsigned n = 0; n < needs.count; n++) {
            unsigned reg = needs.reg[n];

            // The register's latest write is not readable yet.
            if (register_ready_[reg] > ready) [[unlikely]] {
                // Written in the last three cycles: which fields?
                const std::array<u64, 4>& fields = readable_[reg];
                u32 mask = needs.mask[n];

                // Wait for each field the pair reads, x in bit 3 down to w in bit 0.
                if ((mask & 8) && fields[0] > ready) {
                    ready = fields[0];
                }
                if ((mask & 4) && fields[1] > ready) {
                    ready = fields[1];
                }
                if ((mask & 2) && fields[2] > ready) {
                    ready = fields[2];
                }
                if ((mask & 1) && fields[3] > ready) {
                    ready = fields[3];
                }
            }
        }
    }

    straight_++;

    // The pair needs the divider (1) or the function unit (2) to have finished.
    if (needs.wait) [[unlikely]] {
        if (needs.wait == 1 && q_at_ > ready) {
            ready = q_at_;
        }
        if (needs.wait == 2 && p_at_ > ready) {
            ready = p_at_;
        }
    }

    // A branch tests the value an integer register had before the instruction
    // just ahead of it, unless it had to wait: then the write got through.
    if (backup_ttl_) {
        u64 passed = ready - cycle_;
        backup_ttl_ = passed >= backup_ttl_ ? 0 : backup_ttl_ - static_cast<unsigned>(passed);
    }

    cycle_ = ready;

    // What has arrived by then.
    while (flag_count_ && flag_pipe_[flag_first_].at <= cycle_) {
        const Flags& f = flag_pipe_[flag_first_];
        mac = f.mac;
        status = f.status;
        clip = f.clip;

        // The ring has eight slots.
        flag_first_ = (flag_first_ + 1) & 7;
        flag_count_--;
    }

    // The divider's and the function unit's results arrive when their time is up.
    if (q_at_ && q_at_ <= cycle_) {
        finish_q();
    }
    if (p_at_ && p_at_ <= cycle_) {
        p = p_next_;
        p_at_ = 0;
    }

    timed_ = true;

    // Upper bit 29 is the M bit: a point the program marks for the EE to wait for (documented).
    if (up & 0x20000000u) [[unlikely]] {
        sync_point_ = true;
    }

    if (up & 0x80000000u) {
        // The I bit: the lower word is a number for the I register, not an
        // instruction. The upper instruction of the same pair still reads the
        // old I (the games' own sine routine depends on it).
        if (!needs.upper_nop) {
            needs.upper_run(*this, up);
        }
        i = low;
    } else if (!needs.together) {
        // Neither half touches what the other writes (but for the integer
        // registers and memory, which only the lower half has).
        if (!needs.upper_nop) {
            needs.upper_run(*this, up);
        }
        if (!needs.lower_nop) {
            needs.lower_run(*this, low, at);
        }
    } else {
        // Both halves read the registers as they were. So the lower instruction
        // runs with the upper one's register put back, and the upper one's
        // result goes in afterwards, over anything the lower one wrote there.
        in_upper_ = true;
        upper_reg_ = 0;
        needs.upper_run(*this, up);
        in_upper_ = false;
        std::array<u32, 4> result{};

        // The upper instruction wrote a float register: keep its result, put the old value back.
        if (upper_reg_) {
            result = vf[upper_reg_];
            vf[upper_reg_] = upper_old_;
        }

        needs.lower_run(*this, low, at);

        // And now the upper result goes in over anything the lower one wrote there.
        if (upper_reg_) {
            vf[upper_reg_] = result;
        }
    }

    // Something is due after this pair: a kick, a branch or the stop, or this pair has the E bit.
    if (kick_in_ | branch_in_ | stop_in_ | (up & 0x40000000u)) [[unlikely]] {
        // The E bit (bit 30): this instruction and the next, then stop (documented).
        if ((up & 0x40000000u) && !stop_in_) {
            stop_in_ = 2;
        }

        // A kick counts down and is sent after the next pair has run (documented).
        if (kick_in_ && --kick_in_ == 0) {
            fire_kick();
        }

        // After the delay slot the branch is taken and the straight-line count starts over.
        if (branch_in_ && --branch_in_ == 0) {
            pc = branch_target_;
            straight_ = 0;
        }

        if (stop_in_ && --stop_in_ == 0) {
            running_ = false;

            // Nothing is left waiting when a program has stopped.
            settle();
        }
    }
}

void Vu::settle() {
    // Every flag still on its way becomes visible.
    while (flag_count_) {
        const Flags& f = flag_pipe_[flag_first_];
        mac = f.mac;
        status = f.status;
        clip = f.clip;
        flag_first_ = (flag_first_ + 1) & 7;
        flag_count_--;
    }

    // The divider and the function unit deliver at once.
    if (q_at_) {
        finish_q();
    }
    if (p_at_) {
        p = p_next_;
        p_at_ = 0;
    }

    // A pending kick is sent.
    if (kick_in_) {
        fire_kick();
    }

    backup_ttl_ = 0;
}

void Vu::macro(u32 code) {
    fp::want_toward_zero();
    in_upper_ = false;

    // The EE's own instructions are not timed.
    timed_ = false;
    flags_wanted_ = true;
    u32 fn = code & 0x3F;

    // Function field below 0x30: an upper instruction.
    if (fn < 0x30) {
        upper(code);
    } else if (fn < 0x38) {
        // Functions 0x30-0x37 are the lower integer operations.
        lower_special(code);
    } else if (fn >= 0x3C) {
        // The special group: indexes below 0x30 are upper instructions, the rest lower ones.
        if (((((code >> 6) & 0x1F) << 2) | (code & 3)) < 0x30) {
            upper_special(code);
        } else {
            lower_special(code);
        }
    } else {
        // Functions 0x38-0x3B are not instructions: counted.
        unknown_ops++;
    }

    settle();
}

u32 Vu::control(unsigned reg) const {
    // Control registers 0-15 are the integer registers.
    if (reg < 16) {
        return vi[reg];
    }

    switch (reg) {
        case 16:  // status
            return status;

        case 17:  // MAC
            return mac;

        case 18:  // clip
            return clip;

        case 20:  // R
            // R's exponent is fixed; the EE sees the 23-bit mantissa (documented).
            return r & 0x7FFFFF;

        case 21:  // I
            return i;

        case 22:  // Q
            return q;

        case 26:  // TPC
            // The program counter is read in bytes: 8 to an instruction.
            return pc * 8;

        case 27:  // CMSAR0
            return cmsar0_;

        case 28:  // FBRST
            return fbrst_;

        case 29:  // VPU-STAT
            // Bit 0: VU0 is running.
            return running_ ? 1 : 0;

        default:
            // A number not listed reads as zero.
            return 0;
    }
}

void Vu::set_control(unsigned reg, u32 value) {
    // Control registers 0-15 are the integer registers; VI0 cannot be written.
    if (reg < 16) {
        if (reg) {
            vi[reg] = static_cast<u16>(value);
        }
        return;
    }

    switch (reg) {
        case 16:  // status
            // Only the remembered bits (6-11) can be written; bits 0-5 keep their value.
            status = (status & 0x3F) | (value & 0xFC0);
            status_latest_ = status;
            break;

        case 18:  // clip
            // The clipping flags are 24 bits wide (documented).
            clip = clip_latest_ = value & 0xFFFFFF;
            break;

        case 20:  // R
            // Only the mantissa is written; the exponent stays that of 1.0 (documented).
            r = (value & 0x7FFFFF) | 0x3F800000;
            break;

        case 21:  // I
            i = value;
            break;

        case 22:  // Q
            q = value;
            break;

        case 27:  // CMSAR0
            // A 16-bit program address.
            cmsar0_ = value & 0xFFFF;
            break;

        case 28:  // FBRST
            // Only bits 2, 3, 10 and 11 are kept (assumed).
            fbrst_ = value & 0x0C0C;
            break;

        default:
            // The others cannot be written.
            break;
    }
}

// --- helpers ---

inline void Vu::about_to_write_vf(unsigned reg, u32 mask) {
    // The lower instruction must still see the old value: keep it while the upper one runs.
    if (in_upper_) {
        upper_reg_ = reg;
        upper_old_ = vf[reg];
    }

    // Only a running program is timed.
    if (timed_) {
        // A write is readable four cycles later (documented).
        u64 from = cycle_ + 4;
        register_ready_[reg] = from;
        std::array<u64, 4>& fields = readable_[reg];

        // Each field written, x in bit 3 down to w in bit 0, is readable from then.
        if (mask & 8) {
            fields[0] = from;
        }
        if (mask & 4) {
            fields[1] = from;
        }
        if (mask & 2) {
            fields[2] = from;
        }
        if (mask & 1) {
            fields[3] = from;
        }
    }
}

void Vu::write_vf(unsigned reg, u32 mask, const std::array<u32, 4>& value) {
    // VF0 cannot be written (documented).
    if (reg == 0) {
        return;
    }

    about_to_write_vf(reg, mask);

    // Store only the fields the mask names.
    for (unsigned field = 0; field < 4; field++) {
        if (has(mask, field)) {
            vf[reg][field] = value[field];
        }
    }
}

void Vu::write_vi(unsigned reg, u16 value) {
    reg &= 15;

    // VI0 reads as 0 whatever is written to it (documented).
    if (reg == 0) {
        return;
    }

    // A branch right after this instruction still tests the old value. When
    // two writes to one register follow each other, the value kept is the one
    // from before the first.
    if (!(backup_ttl_ && backup_reg_ == reg)) {
        backup_reg_ = reg;
        backup_value_ = vi[reg];
    }

    // The old value stays visible to a branch for the next two pairs (measured).
    backup_ttl_ = 2;
    vi[reg] = value;
}

void Vu::write_vi_from_flags(unsigned reg, u16 value) {
    reg &= 15;

    // VI0 reads as 0 whatever is written to it (documented).
    if (reg == 0) {
        return;
    }

    // These instructions finish early: a branch right after one tests the new value (seen in
    // game code).
    if (backup_reg_ == reg) {
        backup_ttl_ = 0;
    }

    vi[reg] = value;
}

u16 Vu::branch_vi(unsigned reg) const {
    reg &= 15;

    // While the old value is held, the branch sees it instead of the register.
    return (backup_ttl_ && backup_reg_ == reg) ? backup_value_ : vi[reg];
}

void Vu::branch(u32 target) {
    branch_target_ = target & pc_mask_;

    // After the instruction in the delay slot: this pair and the next (documented).
    branch_in_ = 2;
}

void Vu::start_q(u32 value, unsigned latency, u32 divide_flags) {
    // A result still on its way is delivered first; the instruction has already waited for the
    // divider, so this only happens when it did not need to.
    if (q_at_) {
        finish_q();
    }

    q_next_ = value;
    q_flags_ = divide_flags;
    q_at_ = cycle_ + latency;
}

void Vu::start_p(double value, unsigned latency) {
    u32 unused = 0;

    // A result still on its way is replaced: the earlier one arrives at once.
    if (p_at_) {
        p = p_next_;
    }

    // P takes the result as a console number; an overflow flag here is dropped.
    p_next_ = fp::from_double(value, unused);
    p_at_ = cycle_ + latency;
}

u32 Vu::operand(u32 code, From from, unsigned field) const {
    // ft is bits 16-20 of the instruction.
    unsigned ft = (code >> 16) & 31;

    switch (from) {
        case From::Ft:
            return vf[ft][field];

        case From::Bc:
            // The broadcast field is the low two bits of the instruction.
            return vf[ft][code & 3];

        case From::Q:
            return q;

        default:
            // From::I.
            return i;
    }
}

u32 Vu::result(u32 value, u32 problems, unsigned field, u32& flags) const {
    // MAC flags per field, x highest: zero bits 0-3, sign 4-7, underflow 8-11, overflow 12-15
    // (documented).
    unsigned shift = 3 - field;

    if (value & fp::kSign) {
        flags |= 0x0010u << shift;
    }

    if (fp::is_zero(value)) {
        flags |= 0x0001u << shift;
    }

    if (problems & fp::kUnderflow) {
        flags |= 0x0100u << shift;
    }

    if (problems & fp::kOverflow) {
        flags |= 0x1000u << shift;
    }

    return value;
}

void Vu::post() {
    // Cannot happen: at most two a cycle, four cycles deep. If it did, the oldest is dropped.
    if (flag_count_ == 8) {
        flag_first_ = (flag_first_ + 1) & 7;
        flag_count_--;
    }

    // The newest values show four cycles from now (documented).
    flag_pipe_[(flag_first_ + flag_count_) & 7] =
        {mac_latest_, status_latest_, clip_latest_, cycle_ + 4};
    flag_count_++;
}

void Vu::post_flags(u32 mac_bits) {
    u32 now = 0;

    // Status bits 0-3 say whether any field set the zero, sign, underflow or overflow flag
    // (MAC bits 0-3, 4-7, 8-11 and 12-15) (documented).
    if (mac_bits & 0x000F) {
        now |= 1;
    }
    if (mac_bits & 0x00F0) {
        now |= 2;
    }
    if (mac_bits & 0x0F00) {
        now |= 4;
    }
    if (mac_bits & 0xF000) {
        now |= 8;
    }

    mac_latest_ = mac_bits;

    // Bits 0-3 are this result's; bits 6-9 remember every one seen.
    status_latest_ = (status_latest_ & 0xFF0u) | now | (now << 6);
    post();
}

template <Vu::Op op, Vu::From from, bool to_acc>
void Vu::arith(u32 code) {
    // Upper word: dest bits 21-24, ft 16-20, fs 11-15, fd 6-10 (documented).
    u32 dest = (code >> 21) & 0xF;
    unsigned ft = (code >> 16) & 31, fs = (code >> 11) & 31, fd = (code >> 6) & 31;

    // The second operand, a value a field.
    std::array<u32, 4> b;
    switch (from) {
        case From::Ft:
            b = vf[ft];
            break;

        case From::Bc:
            // One field of ft, named by the low two bits, in every field.
            b.fill(vf[ft][code & 3]);
            break;

        case From::Q:
            b.fill(q);
            break;

        default:
            // From::I.
            b.fill(i);
            break;
    }

    const std::array<u32, 4>& a = vf[fs];

    // All four fields at once where every field wanted is an ordinary number.
    std::array<u32, 4> quick, product;
    bool fast = false;

    switch (op) {
        case Op::Add:
            fast = fp::quad_add(a.data(), b.data(), dest, quick.data());
            break;

        case Op::Sub:
            fast = fp::quad_add(a.data(), b.data(), dest, quick.data(), true);
            break;

        case Op::Mul:
            fast = fp::quad_mul(a.data(), b.data(), dest, quick.data());
            break;

        case Op::Madd:
            // The product is made first, then added to the accumulator.
            fast = fp::quad_mul(a.data(), b.data(), dest, product.data())
                   && fp::quad_add(acc.data(), product.data(), dest, quick.data());
            break;

        default:
            // Op::Msub: the product is subtracted from the accumulator.
            fast = fp::quad_mul(a.data(), b.data(), dest, product.data())
                   && fp::quad_add(acc.data(), product.data(), dest, quick.data(), true);
            break;
    }

    // Nobody reads the flags this instruction sets, so the fast result can be stored at once.
    if (fast && !flags_wanted_) [[likely]] {
        if (to_acc) {
            fp::quad_merge(acc.data(), quick.data(), dest);
        } else if (fd) {
            // VF0 is never written; any other target gets the usual bookkeeping.
            about_to_write_vf(fd, dest);
            fp::quad_merge(vf[fd].data(), quick.data(), dest);
        }
        return;
    }

    std::array<u32, 4> out = to_acc ? acc : vf[fd];
    u32 flags = 0;

    if (fast) {
        // The fast path gave the values; the flags still have to be worked out from them.
        for (unsigned field = 0; field < 4; field++) {
            if (has(dest, field)) {
                out[field] = result(quick[field], 0, field, flags);
            }
        }
    } else {
        // At least one field was not ordinary: compute each wanted field one at a time.
        for (unsigned field = 0; field < 4; field++) {
            // Fields outside the mask keep their value.
            if (!has(dest, field)) {
                continue;
            }

            u32 problems = 0, value;

            switch (op) {
                case Op::Add:
                    value = fp::add(a[field], b[field], problems);
                    break;

                case Op::Sub:
                    value = fp::sub(a[field], b[field], problems);
                    break;

                case Op::Mul:
                    value = fp::mul(a[field], b[field], problems);
                    break;

                case Op::Madd:
                    value = fp::add(acc[field], fp::mul(a[field], b[field], problems), problems);
                    break;

                default:
                    // Op::Msub.
                    value = fp::sub(acc[field], fp::mul(a[field], b[field], problems), problems);
                    break;
            }

            out[field] = result(value, problems, field, flags);
        }
    }

    // Into the accumulator, or into fd (write_vf ignores VF0).
    if (to_acc) {
        acc = out;
    } else {
        write_vf(fd, dest, out);
    }

    // The flags go on their way only if something can read them.
    if (flags_wanted_) {
        post_flags(flags);
    }
}

void Vu::min_max(u32 code, From from, bool max) {
    // Upper word: dest bits 21-24, fs 11-15, fd 6-10 (documented).
    u32 dest = (code >> 21) & 0xF;
    unsigned fs = (code >> 11) & 31, fd = (code >> 6) & 31;
    std::array<u32, 4> out = vf[fd];

    // Fields outside the mask keep their value. MAX and MINI set no flags (documented).
    for (unsigned field = 0; field < 4; field++) {
        if (has(dest, field)) {
            u32 a = vf[fs][field], b = operand(code, from, field);
            out[field] = max ? fp::max(a, b) : fp::min(a, b);
        }
    }
    write_vf(fd, dest, out);
}

// --- upper instructions ---

[[gnu::always_inline]] inline void Vu::upper_body(u32 code, u32 fn) {
    switch (fn) {
        // ADDbc
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            arith<Op::Add, From::Bc, false>(code);
            break;

        // SUBbc
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            arith<Op::Sub, From::Bc, false>(code);
            break;

        // MADDbc
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            arith<Op::Madd, From::Bc, false>(code);
            break;

        // MSUBbc
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            arith<Op::Msub, From::Bc, false>(code);
            break;

        // MAXbc
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            min_max(code, From::Bc, true);
            break;

        // MINIbc
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            min_max(code, From::Bc, false);
            break;

        // MULbc
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            arith<Op::Mul, From::Bc, false>(code);
            break;

        case 0x1C:  // MULq
            arith<Op::Mul, From::Q, false>(code);
            break;

        case 0x1D:  // MAXi
            min_max(code, From::I, true);
            break;

        case 0x1E:  // MULi
            arith<Op::Mul, From::I, false>(code);
            break;

        case 0x1F:  // MINIi
            min_max(code, From::I, false);
            break;

        case 0x20:  // ADDq
            arith<Op::Add, From::Q, false>(code);
            break;

        case 0x21:  // MADDq
            arith<Op::Madd, From::Q, false>(code);
            break;

        case 0x22:  // ADDi
            arith<Op::Add, From::I, false>(code);
            break;

        case 0x23:  // MADDi
            arith<Op::Madd, From::I, false>(code);
            break;

        case 0x24:  // SUBq
            arith<Op::Sub, From::Q, false>(code);
            break;

        case 0x25:  // MSUBq
            arith<Op::Msub, From::Q, false>(code);
            break;

        case 0x26:  // SUBi
            arith<Op::Sub, From::I, false>(code);
            break;

        case 0x27:  // MSUBi
            arith<Op::Msub, From::I, false>(code);
            break;

        case 0x28:  // ADD
            arith<Op::Add, From::Ft, false>(code);
            break;

        case 0x29:  // MADD
            arith<Op::Madd, From::Ft, false>(code);
            break;

        case 0x2A:  // MUL
            arith<Op::Mul, From::Ft, false>(code);
            break;

        case 0x2B:  // MAX
            min_max(code, From::Ft, true);
            break;

        case 0x2C:  // SUB
            arith<Op::Sub, From::Ft, false>(code);
            break;

        case 0x2D:  // MSUB
            arith<Op::Msub, From::Ft, false>(code);
            break;

        case 0x2E: {  // OPMSUB: the second half of a cross product
            unsigned ft = (code >> 16) & 31, fs = (code >> 11) & 31, fd = (code >> 6) & 31;
            std::array<u32, 4> out = vf[fd];
            u32 flags = 0;

            // x, y and z, each with the next two components: fs.y * ft.z, fs.z * ft.x, fs.x * ft.y.
            for (unsigned field = 0; field < 3; field++) {
                unsigned a = (field + 1) % 3, b = (field + 2) % 3;
                u32 problems = 0;
                u32 value = fp::sub(acc[field], fp::mul(vf[fs][a], vf[ft][b], problems), problems);
                out[field] = result(value, problems, field, flags);
            }
            write_vf(fd, kXyz, out);
            post_flags(flags);
            break;
        }

        case 0x2F:  // MINI
            min_max(code, From::Ft, false);
            break;

        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            // The special group is decoded by its index.
            upper_special(code);
            break;

        default:
            // Function numbers not listed are counted, and the pair goes on.
            unknown_ops++;
            break;
    }
}

void Vu::upper(u32 code) {
    upper_body(code, code & 0x3F);
}

void Vu::upper_special(u32 code) {
    upper_special_body(code, (((code >> 6) & 0x1F) << 2) | (code & 3));
}

[[gnu::always_inline]] inline void Vu::upper_special_body(u32 code, u32 fn) {
    // Upper word: dest bits 21-24, ft 16-20, fs 11-15 (documented). `fn` is the 7-bit index.
    u32 dest = (code >> 21) & 0xF;
    unsigned ft = (code >> 16) & 31, fs = (code >> 11) & 31;

    switch (fn) {
        // ADDAbc
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            arith<Op::Add, From::Bc, true>(code);
            break;

        // SUBAbc
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            arith<Op::Sub, From::Bc, true>(code);
            break;

        // MADDAbc
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
            arith<Op::Madd, From::Bc, true>(code);
            break;

        // MSUBAbc
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            arith<Op::Msub, From::Bc, true>(code);
            break;

        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13: {  // ITOF0, 4, 12, 15
            // The integer in each field is divided by 2^shift: 0, 4, 12 or 15 bits (documented).
            static constexpr s32 shift[4] = {0, 4, 12, 15};
            std::array<u32, 4> out{};
            for (unsigned field = 0; field < 4; field++) {
                out[field] = fp::from_int(static_cast<s32>(vf[fs][field]), shift[fn & 3]);
            }
            write_vf(ft, dest, out);
            break;
        }
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17: {  // FTOI0, 4, 12, 15
            // Each field is multiplied by 2^shift and cut to an integer.
            static constexpr s32 shift[4] = {0, 4, 12, 15};
            std::array<u32, 4> out{};
            for (unsigned field = 0; field < 4; field++) {
                out[field] = static_cast<u32>(fp::to_int(vf[fs][field], shift[fn & 3]));
            }
            write_vf(ft, dest, out);
            break;
        }
        // MULAbc
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            arith<Op::Mul, From::Bc, true>(code);
            break;

        case 0x1C:  // MULAq
            arith<Op::Mul, From::Q, true>(code);
            break;

        case 0x1D: {  // ABS
            // Clearing the sign bit is the absolute value (documented).
            std::array<u32, 4> out{};
            for (unsigned field = 0; field < 4; field++) {
                out[field] = vf[fs][field] & ~fp::kSign;
            }
            write_vf(ft, dest, out);
            break;
        }
        case 0x1E:  // MULAi
            arith<Op::Mul, From::I, true>(code);
            break;

        case 0x1F: {  // CLIP: x, y and z of fs against plus and minus |w| of ft
            // The bound is the magnitude of ft's w, as an ordered key (zeros equal).
            s64 w = fp::key(vf[ft][3] & ~fp::kSign);
            u32 now = 0;

            // Two bits per field: above +|w| and below -|w| (documented).
            for (unsigned field = 0; field < 3; field++) {
                s64 v = fp::key(vf[fs][field]);

                if (v > w) {
                    now |= 1u << (field * 2);
                }

                if (v < -w) {
                    now |= 2u << (field * 2);
                }
            }

            // The six new bits shift in under the older ones, which move up; 24 bits are kept.
            clip_latest_ = ((clip_latest_ << 6) | now) & 0xFFFFFFu;
            post();
            break;
        }
        case 0x20:  // ADDAq
            arith<Op::Add, From::Q, true>(code);
            break;

        case 0x21:  // MADDAq
            arith<Op::Madd, From::Q, true>(code);
            break;

        case 0x22:  // ADDAi
            arith<Op::Add, From::I, true>(code);
            break;

        case 0x23:  // MADDAi
            arith<Op::Madd, From::I, true>(code);
            break;

        case 0x24:  // SUBAq
            arith<Op::Sub, From::Q, true>(code);
            break;

        case 0x25:  // MSUBAq
            arith<Op::Msub, From::Q, true>(code);
            break;

        case 0x26:  // SUBAi
            arith<Op::Sub, From::I, true>(code);
            break;

        case 0x27:  // MSUBAi
            arith<Op::Msub, From::I, true>(code);
            break;

        case 0x28:  // ADDA
            arith<Op::Add, From::Ft, true>(code);
            break;

        case 0x29:  // MADDA
            arith<Op::Madd, From::Ft, true>(code);
            break;

        case 0x2A:  // MULA
            arith<Op::Mul, From::Ft, true>(code);
            break;

        case 0x2C:  // SUBA
            arith<Op::Sub, From::Ft, true>(code);
            break;

        case 0x2D:  // MSUBA
            arith<Op::Msub, From::Ft, true>(code);
            break;

        case 0x2E: {  // OPMULA: the first half of a cross product
            u32 flags = 0;
            std::array<u32, 4> out = acc;

            // x, y and z, each with the next two components: fs.y * ft.z, fs.z * ft.x, fs.x * ft.y.
            for (unsigned field = 0; field < 3; field++) {
                unsigned a = (field + 1) % 3, b = (field + 2) % 3;
                u32 problems = 0;
                u32 value = fp::mul(vf[fs][a], vf[ft][b], problems);
                out[field] = result(value, problems, field, flags);
            }
            acc = out;
            post_flags(flags);
            break;
        }
        case 0x2F:  // NOP
            break;

        default:
            // Indexes not listed are counted, and the pair goes on.
            unknown_ops++;
            break;
    }
}

// --- lower instructions ---

void Vu::lower(u32 code, u32 at) {
    lower_body(code, at, code >> 25);
}

void Vu::lower_special(u32 code) {
    lower_special_body(code, code & 0x3F, (((code >> 6) & 0x1F) << 2) | (code & 3));
}

[[gnu::always_inline]] inline void Vu::lower_body(u32 code, u32 at, u32 op) {
    // Lower word: dest bits 21-24, it 16-20, is 11-15 (documented).
    u32 dest = (code >> 21) & 0xF;
    unsigned it = (code >> 16) & 31, is = (code >> 11) & 31;

    // Immediates: 11 bits signed in bits 0-10; 12 and 15 bits with the top bits in bits 21-24;
    // 24 bits in bits 0-23 (documented).
    s32 imm11 = sign_extend(code & 0x7FF, 11);
    u32 imm12 = ((code >> 10) & 0x800) | (code & 0x7FF);
    u32 imm15 = ((code >> 10) & 0x7800) | (code & 0x7FF);
    u32 imm24 = code & 0xFFFFFF;

    // A branch counts instructions from the pair after the branch.
    u32 next = at + 1 + static_cast<u32>(imm11);

    // Called by ILW in this function only: the first field the mask selects, x before y.
    auto first_field = [dest]() -> unsigned {
        for (unsigned field = 0; field < 4; field++) {
            if (has(dest, field)) {
                return field;
            }
        }
        return 0;
    };

    switch (op) {
        case 0x00: {  // LQ
            const u8* m = quad(static_cast<u32>(vi[is & 15] + imm11));
            write_vf(
                it, dest, {load<u32>(m), load<u32>(m + 4), load<u32>(m + 8), load<u32>(m + 12)}
            );
            break;
        }
        case 0x01: {  // SQ
            u8* m = quad(static_cast<u32>(vi[it & 15] + imm11));
            for (unsigned field = 0; field < 4; field++) {
                if (has(dest, field)) {
                    store<u32>(m + field * 4, vf[is][field]);
                }
            }
            break;
        }
        case 0x04:  // ILW
            write_vi(
                it, load<u16>(quad(static_cast<u32>(vi[is & 15] + imm11)) + first_field() * 4)
            );
            break;

        case 0x05: {  // ISW
            u8* m = quad(static_cast<u32>(vi[is & 15] + imm11));
            for (unsigned field = 0; field < 4; field++) {
                if (has(dest, field)) {
                    store<u32>(m + field * 4, vi[it & 15]);
                }
            }
            break;
        }
        case 0x08:  // IADDIU
            write_vi(it, static_cast<u16>(vi[is & 15] + imm15));
            break;

        case 0x09:  // ISUBIU
            write_vi(it, static_cast<u16>(vi[is & 15] - imm15));
            break;

        case 0x10:  // FCEQ
            write_vi_from_flags(1, (clip & 0xFFFFFF) == imm24);
            break;

        case 0x11:  // FCSET
            clip_latest_ = imm24;
            post();
            break;

        case 0x12:  // FCAND
            write_vi_from_flags(1, (clip & imm24) != 0);
            break;

        case 0x13:  // FCOR
            write_vi_from_flags(1, ((clip | imm24) & 0xFFFFFF) == 0xFFFFFF);
            break;

        case 0x14:  // FSEQ
            write_vi_from_flags(it, (status & 0xFFF) == imm12);
            break;

        case 0x15:  // FSSET: the remembered bits only
            status_latest_ = (status_latest_ & 0x3F) | (imm12 & 0xFC0);
            post();
            break;

        case 0x16:  // FSAND
            write_vi_from_flags(it, static_cast<u16>(status & imm12));
            break;

        case 0x17:  // FSOR
            write_vi_from_flags(it, static_cast<u16>((status | imm12) & 0xFFF));
            break;

        case 0x18:  // FMEQ
            write_vi_from_flags(it, (mac & 0xFFFF) == vi[is & 15]);
            break;

        case 0x1A:  // FMAND
            write_vi_from_flags(it, static_cast<u16>(mac & vi[is & 15]));
            break;

        case 0x1B:  // FMOR
            write_vi_from_flags(it, static_cast<u16>(mac | vi[is & 15]));
            break;

        case 0x1C:  // FCGET
            write_vi_from_flags(it, static_cast<u16>(clip & 0xFFF));
            break;

        case 0x20:  // B
            branch(next);
            break;

        case 0x21:  // BAL
            // The link is the pair after the delay slot; VI0 is not written.
            if (it & 15) {
                vi[it & 15] = static_cast<u16>(at + 2);
            }

            branch(next);
            break;

        case 0x24:  // JR
            branch(vi[is & 15]);
            break;

        case 0x25: {  // JALR
            u32 target = vi[is & 15];

            // The link is the pair after the delay slot; VI0 is not written.
            if (it & 15) {
                vi[it & 15] = static_cast<u16>(at + 2);
            }
            branch(target);
            break;
        }
        case 0x28:  // IBEQ
            // Taken when the two registers are equal; both are read as a branch sees them.
            if (branch_vi(it) == branch_vi(is)) {
                branch(next);
            }
            break;

        case 0x29:  // IBNE
            // Taken when the two registers differ.
            if (branch_vi(it) != branch_vi(is)) {
                branch(next);
            }
            break;

        case 0x2C:  // IBLTZ
            // Taken when the register, as a signed 16-bit number, is below zero.
            if (static_cast<s16>(branch_vi(is)) < 0) {
                branch(next);
            }
            break;

        case 0x2D:  // IBGTZ
            // Taken when the register is above zero.
            if (static_cast<s16>(branch_vi(is)) > 0) {
                branch(next);
            }
            break;

        case 0x2E:  // IBLEZ
            // Taken when the register is zero or below.
            if (static_cast<s16>(branch_vi(is)) <= 0) {
                branch(next);
            }
            break;

        case 0x2F:  // IBGEZ
            // Taken when the register is zero or above.
            if (static_cast<s16>(branch_vi(is)) >= 0) {
                branch(next);
            }
            break;

        case 0x40:  // special group
            lower_special(code);
            break;

        default:
            // Opcodes not listed are counted, and the pair goes on.
            unknown_ops++;
            break;
    }
}

[[gnu::always_inline]] inline void Vu::lower_special_body(u32 code, u32 fn, u32 index) {
    // Lower word: dest 21-24, it 16-20, is 11-15, id 6-10; the field selectors fsf (21-22) and
    // ftf (23-24) sit where dest is (documented).
    u32 dest = (code >> 21) & 0xF;
    unsigned it = (code >> 16) & 31, is = (code >> 11) & 31, id = (code >> 6) & 31;
    unsigned fsf = (code >> 21) & 3, ftf = (code >> 23) & 3;

    // Functions below 0x3C are the integer arithmetic.
    if (fn < 0x3C) {
        switch (fn) {
            case 0x30:  // IADD
                write_vi(id, static_cast<u16>(vi[is & 15] + vi[it & 15]));
                break;

            case 0x31:  // ISUB
                write_vi(id, static_cast<u16>(vi[is & 15] - vi[it & 15]));
                break;

            case 0x32:  // IADDI
                // The immediate is the 5-bit field in the id place, signed.
                write_vi(it, static_cast<u16>(vi[is & 15] + sign_extend(id, 5)));
                break;

            case 0x34:  // IAND
                write_vi(id, vi[is & 15] & vi[it & 15]);
                break;

            case 0x35:  // IOR
                write_vi(id, vi[is & 15] | vi[it & 15]);
                break;

            default:
                // Functions not listed are counted, and the pair goes on.
                unknown_ops++;
                break;
        }

        return;
    }

    // The lambdas below are called in this function only, to move quadwords of data memory.
    auto load_quad = [this](u32 address) -> std::array<u32, 4> {
        const u8* m = quad(address);
        return {load<u32>(m), load<u32>(m + 4), load<u32>(m + 8), load<u32>(m + 12)};
    };
    auto store_quad = [this, dest](u32 address, const std::array<u32, 4>& v) {
        u8* m = quad(address);
        for (unsigned field = 0; field < 4; field++) {
            if (has(dest, field)) {
                store<u32>(m + field * 4, v[field]);
            }
        }
    };
    auto all = [](u32 v) -> std::array<u32, 4> {
        return {v, v, v, v};
    };
    // Steps the random number in R: a 23-bit shift register fed back from bits 4 and 22 (assumed).
    auto advance_random = [this] {
        u32 x = (r >> 4) & 1, y = (r >> 22) & 1;
        r = (((r << 1) ^ x ^ y) & 0x7FFFFF) | 0x3F800000;
    };

    // The function unit's operands, for the instructions that use it.
    double x = 0, y = 0, z = 0, one = 0;

    // Indexes from 0x70 are the function unit's E instructions: x, y and z of fs, and field fsf.
    if (index >= 0x70) {
        x = fp::to_double(vf[is][0]);
        y = fp::to_double(vf[is][1]);
        z = fp::to_double(vf[is][2]);
        one = fp::to_double(vf[is][fsf]);
    }

    // The special group is decoded by its 7-bit index (documented).
    switch (index) {
        case 0x30:  // MOVE
            write_vf(it, dest, vf[is]);
            break;

        case 0x31:  // MR32: rotate the fields one place
            write_vf(it, dest, {vf[is][1], vf[is][2], vf[is][3], vf[is][0]});
            break;

        case 0x34: {  // LQI
            // Load from the address in is, then add one to it.
            u16 address = vi[is & 15];
            write_vf(it, dest, load_quad(address));
            write_vi(is, static_cast<u16>(address + 1));
            break;
        }

        case 0x35: {  // SQI
            // Store at the address in it, then add one to it.
            u16 address = vi[it & 15];
            store_quad(address, vf[is]);
            write_vi(it, static_cast<u16>(address + 1));
            break;
        }

        case 0x36: {  // LQD
            // Take one off the address in is, then load from it.
            u16 address = static_cast<u16>(vi[is & 15] - 1);
            write_vi(is, address);
            write_vf(it, dest, load_quad(address));
            break;
        }

        case 0x37: {  // SQD
            // Take one off the address in it, then store at it.
            u16 address = static_cast<u16>(vi[it & 15] - 1);
            write_vi(it, address);
            store_quad(address, vf[is]);
            break;
        }

        case 0x38: {  // DIV
            u32 problems = 0;
            u32 value = fp::div(vf[is][fsf], vf[it][ftf], problems);
            start_q(value, kDiv, divide_flags(problems));
            break;
        }

        case 0x39: {  // SQRT
            u32 problems = 0;
            u32 value = fp::sqrt(vf[it][ftf], problems);
            start_q(value, kSqrt, divide_flags(problems));
            break;
        }

        case 0x3A: {  // RSQRT
            u32 problems = 0;
            u32 value = fp::rsqrt(vf[is][fsf], vf[it][ftf], problems);
            start_q(value, kRsqrt, divide_flags(problems));
            break;
        }

        case 0x3B:  // WAITQ: the pair has waited; the result is in
            break;

        case 0x3C:  // MTIR
            // The low 16 bits of the field go to the integer register.
            write_vi(it, static_cast<u16>(vf[is][fsf]));
            break;

        case 0x3D:  // MFIR
            // The 16-bit integer is sign extended to 32 bits and put in every selected field.
            write_vf(
                it, dest, all(static_cast<u32>(static_cast<s32>(static_cast<s16>(vi[is & 15]))))
            );
            break;

        case 0x3E:  // ILWR
            // Only the first selected field is read, as for ILW; the loop ends at that field.
            for (unsigned field = 0; field < 4; field++) {
                if (has(dest, field)) {
                    write_vi(it, load<u16>(quad(vi[is & 15]) + field * 4));
                    break;
                }
            }

            break;

        case 0x3F:  // ISWR
            store_quad(vi[is & 15], all(vi[it & 15]));
            break;

        case 0x40:  // RNEXT
            advance_random();
            write_vf(it, dest, all(r));
            break;

        case 0x41:  // RGET
            write_vf(it, dest, all(r));
            break;

        case 0x42:  // RINIT
            // R keeps the exponent of 1.0 and takes 23 bits of the field (documented).
            r = (vf[is][fsf] & 0x7FFFFF) | 0x3F800000;
            break;

        case 0x43:  // RXOR
            r = ((r ^ vf[is][fsf]) & 0x7FFFFF) | 0x3F800000;
            break;

        case 0x64:  // MFP
            write_vf(it, dest, all(p));
            break;

        case 0x68:  // XTOP
            // Without a callback there is no VIF, and the register reads 0.
            write_vi(it, static_cast<u16>(on_top ? on_top() : 0));
            break;

        case 0x69:  // XITOP
            write_vi(it, static_cast<u16>(on_itop ? on_itop() : 0));
            break;

        case 0x6C:  // XGKICK
            // A kick still waiting goes first, so two kicks in a row keep their order.
            if (kick_in_) {
                fire_kick();
            }

            kick_address_ = vi[is & 15];

            // The packet goes after the next instruction has run (documented).
            kick_in_ = 2;
            break;

        /*
         * The function unit's E instructions start a result that arrives after a fixed number of
         * cycles (documented): ESADD 11, ERSADD 18, ELENG 18, ERLENG 24, EATAN forms 54, ESUM 12,
         * ESQRT 12, ERSQRT 18, ERCPR 12, ESIN 29, EEXP 44.
         */
        case 0x70:  // ESADD
            start_p(x * x + y * y + z * z, 11);
            break;

        case 0x71:  // ERSADD
            start_p(1.0f / (x * x + y * y + z * z), 18);
            break;

        case 0x72:  // ELENG
            start_p(std::sqrt(x * x + y * y + z * z), 18);
            break;

        case 0x73:  // ERLENG
            start_p(1.0f / std::sqrt(x * x + y * y + z * z), 24);
            break;

        case 0x74:  // EATANxy
            start_p(std::atan2(y, x), 54);
            break;

        case 0x75:  // EATANxz
            start_p(std::atan2(z, x), 54);
            break;

        case 0x76:  // ESUM
            start_p(x + y + z + fp::to_double(vf[is][3]), 12);
            break;

        case 0x78:  // ESQRT
            start_p(std::sqrt(std::fabs(one)), 12);
            break;

        case 0x79:  // ERSQRT
            start_p(1.0f / std::sqrt(std::fabs(one)), 18);
            break;

        case 0x7A:  // ERCPR
            start_p(1.0f / one, 12);
            break;

        case 0x7B:  // WAITP: likewise
            break;

        case 0x7C:  // ESIN
            start_p(std::sin(one), 29);
            break;

        case 0x7D:  // EATAN
            start_p(std::atan(one), 54);
            break;

        case 0x7E:  // EEXP
            start_p(std::exp(-one), 44);
            break;

        default:
            // Indexes not listed are counted, and the pair goes on.
            unknown_ops++;
            break;
    }
}

// --- one function per slot ---

/*
 * Upper slots: 0-63 by the function field, then 64-191 the special ones by
 * their index. Lower slots: 0-127 by the operation field, 128-191 the
 * integer operations of the special group by function field, 192-319 the
 * rest of that group by index.
 */
template <unsigned Slot>
void Vu::upper_as(Vu& vu, u32 code) {
    // Slots below 64 are the ordinary functions; the rest are the special group by index.
    if (Slot < 64) {
        vu.upper_body(code, Slot);
    } else {
        vu.upper_special_body(code, Slot - 64);
    }
}

template <unsigned Slot>
void Vu::lower_as(Vu& vu, u32 code, u32 at) {
    if (Slot < 128) {
        // The operation field.
        vu.lower_body(code, at, Slot);
    } else if (Slot < 192) {
        // The integer operations of the special group, by function field.
        vu.lower_special_body(code, Slot - 128, 0);
    } else {
        // The rest of the special group (function 0x3C-0x3F), by index.
        vu.lower_special_body(code, 0x3C, Slot - 192);
    }
}

template <std::size_t... N>
constexpr std::array<Vu::UpperRun, sizeof...(N)> Vu::upper_runs(std::index_sequence<N...>) {
    return {&Vu::upper_as<N>...};
}

template <std::size_t... N>
constexpr std::array<Vu::LowerRun, sizeof...(N)> Vu::lower_runs(std::index_sequence<N...>) {
    return {&Vu::lower_as<N>...};
}

// 192 upper slots and 320 lower slots, as the comment above counts them.
const std::array<Vu::UpperRun, 192> Vu::kUpperRuns =
    Vu::upper_runs(std::make_index_sequence<192>{});
const std::array<Vu::LowerRun, 320> Vu::kLowerRuns =
    Vu::lower_runs(std::make_index_sequence<320>{});

}  // namespace ps2
