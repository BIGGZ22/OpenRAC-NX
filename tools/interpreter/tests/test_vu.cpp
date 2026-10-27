// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Tests of the vector unit interpreter, on programs written here with the
 * encoders in vu_asm.h. What they expect is the documented behaviour of the
 * instruction set and its timing.
 *
 * Every expected value is worked out from the instruction's definition or from arithmetic, as
 * the comment beside it shows. Programs are numbered by instruction (pair) address in the
 * comments of their rows. The last test runs a program through VIF1 and the GIF to the GS.
 *
 * Sources: the vector unit's instruction set, flags and timing as publicly documented.
 */

#include <array>
#include <cfenv>
#include <cstring>
#include <string>
#include <vector>

#include "check.h"
#include "fp_quad.h"
#include "graphics.h"
#include "vu.h"
#include "vu_asm.h"
#include "vu_programs.h"

using namespace ps2;
using namespace ps2::vuasm;

namespace {

/** A vector unit with VU1's memory sizes (16 KB each, documented) and nothing attached. */
struct Unit {
    /** The unit's program memory. */
    std::array<u8, 16384> micro{};

    /** The unit's data memory. */
    std::array<u8, 16384> data{};

    /** The unit under test, running on the two memories above. */
    Vu vu{Vu::Memory{micro.data(), 16384, data.data(), 16384}};

    /**
     * Loads a program and runs it to its end.
     *
     * @param program The program to load at address 0.
     * @param at The pair to start at.
     * @return How many instructions ran.
     */
    u64 run(const Program& program, u32 at = 0) {
        // Four bytes a word: copy the whole program into program memory.
        std::memcpy(micro.data(), program.words().data(), program.words().size() * 4);
        vu.program_changed();
        u64 ran = vu.run(at);

        // The unit leaves the host rounding towards zero; the checks compute as the host does.
        fp::want_nearest();
        return ran;
    }

    /**
     * Sets the four fields of a float register.
     *
     * @param reg Register number.
     * @param x The x field.
     * @param y The y field.
     * @param z The z field.
     * @param w The w field.
     */
    void set(unsigned reg, float x, float y, float z, float w) {
        vu.vf[reg] = {as_u32(x), as_u32(y), as_u32(z), as_u32(w)};
    }

    /**
     * Reads a word of data memory.
     *
     * @param quad Quadword number.
     * @param field Word within the quadword, 0-3.
     * @return The word.
     */
    u32 word(u32 quad, u32 field) const { return load<u32>(&data[quad * 16 + field * 4]); }

    /**
     * Writes a word of data memory.
     *
     * @param quad Quadword number.
     * @param field Word within the quadword, 0-3.
     * @param v The value.
     */
    void set_word(u32 quad, u32 field, u32 v) { store<u32>(&data[quad * 16 + field * 4], v); }
};

/**
 * The usual ending: the E bit on a pair, then one more.
 *
 * The pair after the E bit still runs (documented), so the program is two pairs longer.
 *
 * @param p The program to end.
 */
void finish(Program& p) {
    p.add(nop() | E, lnop());
    p.hi(nop());
}

/**
 * An instruction writes only the fields its mask names, VF0 cannot be written, and the
 * multiply-add forms read the accumulator (documented).
 */
void test_arithmetic_and_masks() {
    Unit u;

    // vf1 and vf2 are the inputs; vf3 starts at 9 so that fields left alone show.
    u.set(1, 1.5f, 2.0f, -3.0f, 4.0f);
    u.set(2, 0.5f, 10.0f, 1.0f, 2.0f);
    u.set(3, 9.0f, 9.0f, 9.0f, 9.0f);
    Program p;
    p.hi(add(XY, 3, 1, 2));         // only x and y are written
    p.hi(mulbc(XYZW, 4, 1, 2, 3));  // every field times vf2.w
    p.hi(sub(XYZW, 0, 1, 2));       // VF0 cannot be written
    p.hi(mula(XYZW, 1, 2));         // ACC = vf1 * vf2
    p.hi(madd(XYZW, 5, 1, 1));      // vf5 = ACC + vf1 * vf1
    p.hi(msubbc(X, 6, 1, 2, 1));    // vf6.x = ACC.x - vf1.x * vf2.y
    p.hi(max(XYZW, 7, 1, 2));
    p.hi(mini(XYZW, 8, 1, 2));
    p.hi(abs(XYZW, 9, 1));
    finish(p);
    u64 ran = u.run(p);

    // Nine instructions and the two of the ending ran, and the program counter is past them.
    CHECK_EQ(ran, u64{11});
    CHECK(u.vu.stopped());
    CHECK_EQ(u.vu.pc, 11u);

    // ADD on x and y: 1.5 + 0.5 and 2 + 10; z and w keep the 9s.
    CHECK(
        u.vu.f(3, 0) == 2.0f && u.vu.f(3, 1) == 12.0f && u.vu.f(3, 2) == 9.0f
        && u.vu.f(3, 3) == 9.0f
    );

    // MULbc by vf2.w = 2 doubles vf1, and VF0 is still (0, 0, 0, 1).
    CHECK(u.vu.f(4, 0) == 3.0f && u.vu.f(4, 2) == -6.0f && u.vu.f(4, 3) == 8.0f);
    CHECK(u.vu.f(0, 0) == 0.0f && u.vu.f(0, 3) == 1.0f);

    // ACC = vf1 * vf2 = (0.75, 20, -3, 8); MADD adds vf1 * vf1 = (2.25, 4, 9, 16).
    CHECK(u.vu.f(5, 0) == 0.75f + 2.25f && u.vu.f(5, 2) == -3.0f + 9.0f);

    // MSUBbc: ACC.x less vf1.x times vf2.y (1.5 * 10).
    CHECK(u.vu.f(6, 0) == 0.75f - 15.0f);

    // The larger of each pair, the smaller of each pair, and the absolute value of -3.
    CHECK(u.vu.f(7, 0) == 1.5f && u.vu.f(7, 1) == 10.0f && u.vu.f(7, 2) == 1.0f);
    CHECK(u.vu.f(8, 0) == 0.5f && u.vu.f(8, 2) == -3.0f);
    CHECK(u.vu.f(9, 2) == 3.0f);
    CHECK_EQ(u.vu.unknown_ops, u64{0});
}

/** OPMULA and OPMSUB together give the cross product: x cross y is z (documented). */
void test_cross_product() {
    Unit u;
    u.set(1, 1.0f, 0.0f, 0.0f, 0.0f);
    u.set(2, 0.0f, 1.0f, 0.0f, 0.0f);
    Program p;
    p.hi(opmula(1, 2));
    p.hi(opmsub(3, 2, 1));
    finish(p);
    u.run(p);

    // (1, 0, 0) cross (0, 1, 0) = (0, 0, 1).
    CHECK(u.vu.f(3, 0) == 0.0f && u.vu.f(3, 1) == 0.0f && u.vu.f(3, 2) == 1.0f);
}

/** The console's numbers have no infinities and no denormals, and the adder cuts (documented). */
void test_numbers() {
    Unit u;

    /*
     * There are no infinities: the largest number plus itself is the largest
     * number, with the overflow flag; a product too small to hold is zero,
     * with underflow; the largest exponent is an ordinary one.
     */
    u.vu.vf[1] = {0x7FFFFFFFu, as_u32(1e-30f), as_u32(1.0f), 0x7F800000u};
    u.vu.vf[2] = {0x7FFFFFFFu, as_u32(1e-30f), 0x33C00000u /* 3 * 2^-25 */, as_u32(1.0f)};
    u.vu.vi[5] = 0xFFFF;
    Program p;
    p.hi(add(X, 3, 1, 2));
    p.hi(mul(Y, 3, 1, 2));
    p.hi(add(Z, 3, 1, 2));  // cut, not rounded: 1 + 0.75 of a last place is 1
    p.hi(mul(W, 3, 1, 2));  // 2^128 times one is 2^128
    finish(p);
    u.run(p);

    // The sum saturates at the largest number 0x7FFFFFFF; 1e-30 squared underflows to zero.
    CHECK_EQ(u.vu.vf[3][0], 0x7FFFFFFFu);
    CHECK_EQ(u.vu.vf[3][1], 0u);

    // 1.0 is 0x3F800000; the sum stays it, and 2^128 (0x7F800000) times 1 is unchanged.
    CHECK_EQ(u.vu.vf[3][2], 0x3F800000u);
    CHECK_EQ(u.vu.vf[3][3], 0x7F800000u);

    // The status register remembers overflow (bit 9) and underflow (bit 8).
    CHECK_EQ(u.vu.status & 0x300, 0x300u);
}

/** Conversions scale by 2^-n and 2^n, cut toward zero, and saturate (documented). */
void test_conversions() {
    Unit u;

    // vf1 holds integers (40, -24, the largest, 0); vf2 holds floats, two of them beyond 32 bits.
    u.vu.vf[1] = {static_cast<u32>(40), static_cast<u32>(-24), 0x7FFFFFFFu, 0};
    u.set(2, 2.75f, -2.75f, 3e9f, -3e9f);
    Program p;
    p.hi(itof4(XY, 3, 1));  // 40 / 16, -24 / 16
    p.hi(itof0(X, 4, 1));
    p.hi(ftoi0(XYZW, 5, 2));  // towards zero, saturating
    p.hi(ftoi4(XY, 6, 2));
    finish(p);
    u.run(p);

    // 40 / 16 = 2.5 and -24 / 16 = -1.5; with no scale 40 stays 40.
    CHECK(u.vu.f(3, 0) == 2.5f && u.vu.f(3, 1) == -1.5f);
    CHECK(u.vu.f(4, 0) == 40.0f);

    // Toward zero: 2.75 gives 2, -2.75 gives -2. 3e9 and -3e9 saturate at the 32-bit limits.
    CHECK_EQ(u.vu.vf[5][0], 2u);
    CHECK_EQ(u.vu.vf[5][1], static_cast<u32>(-2));
    CHECK_EQ(u.vu.vf[5][2], 0x7FFFFFFFu);
    CHECK_EQ(u.vu.vf[5][3], 0x80000000u);

    // With a scale of 4 bits the result is 16 times larger: 2.75 * 16 = 44.
    CHECK_EQ(u.vu.vf[6][0], 44u);
    CHECK_EQ(u.vu.vf[6][1], static_cast<u32>(-44));
}

/** Both halves of a pair read the registers as they were before either wrote (documented). */
void test_pair_reads_old_values() {
    Unit u;
    u.set(1, 1.0f, 1.0f, 1.0f, 1.0f);
    u.set(2, 2.0f, 2.0f, 2.0f, 2.0f);
    u.set_word(9, 0, as_u32(50.0f));
    Program p;

    // The lower instruction stores vf1 while the upper one changes it: memory
    // gets the old value.
    p.add(add(XYZW, 1, 1, 2), sq(XYZW, 1, 8, 0));
    // Both write vf3: the upper instruction's result stays.
    p.add(add(X, 3, 2, 2), lq(X, 3, 9, 0));
    // The upper instruction reads vf4 while the lower one loads it: it adds
    // the old value.
    p.add(add(X, 5, 4, 2), lq(X, 4, 9, 0));
    finish(p);
    u.run(p);

    // The register got the sum (1 + 2) and memory the old 1.
    CHECK(u.vu.f(1, 0) == 3.0f);
    CHECK_EQ(u.word(8, 0), as_u32(1.0f));

    // Both wrote vf3.x: the upper result (2 + 2) stays, and the load of 50 into vf4 happened.
    CHECK(u.vu.f(3, 0) == 4.0f);
    CHECK(u.vu.f(4, 0) == 50.0f);

    // The add read vf4 as zero, not the 50 loaded beside it: 0 + 2.
    CHECK(u.vu.f(5, 0) == 2.0f);
}

/** A pair with the I bit supplies a number for I, which MULi, ADDi and SUBi read (documented). */
void test_immediate() {
    Unit u;
    u.set(1, 4.0f, 0, 0, 0);
    Program p;
    p.loi(nop(), 2.5f);
    p.hi(muli(X, 2, 1));
    p.hi(addi(X, 3, 1));
    p.hi(subi(X, 4, 1));
    finish(p);
    u.run(p);

    // With I = 2.5 and x = 4: 4 * 2.5, 4 + 2.5 and 4 - 2.5.
    CHECK(u.vu.f(2, 0) == 10.0f && u.vu.f(3, 0) == 6.5f && u.vu.f(4, 0) == 1.5f);
}

/** Flags set by an instruction reach the next one four cycles later (documented). */
void test_flags_arrive_four_later() {
    Unit u;

    // vf1 = (5, -1): subtracting it from itself gives zeros, adding gives -2 in y.
    u.set(1, 5.0f, -1.0f, 0, 0);

    // vi5 is the mask FMAND reads the MAC flags through: all bits.
    u.vu.vi[5] = 0xFFFF;
    Program p;
    p.hi(sub(XY, 3, 1, 1));  // 0: x and y zero: MAC zero flags for x (bit 3) and y (bit 2)
    p.lo(fmand(1, 5));       // 1
    p.lo(fmand(2, 5));       // 2
    p.lo(fmand(3, 5));       // 3
    p.lo(fmand(4, 5));       // 4: the first instruction that sees them
    p.hi(add(Y, 3, 1, 1));   // 5: -2: sign flag for y (bit 6), no zero flags
    p.hi(nop());
    p.hi(nop());
    p.hi(nop());
    p.lo(fmand(6, 5));      // 9
    p.lo(fsand(7, 0xFFF));  // 10: status: sign now, and "zero" and "sign" remembered
    finish(p);
    u.run(p);

    // The three instructions before the fourth after the SUB see no flags yet.
    CHECK_EQ(u.vu.vi[1], 0u);
    CHECK_EQ(u.vu.vi[2], 0u);
    CHECK_EQ(u.vu.vi[3], 0u);

    // The fourth sees the zero flags of x and y: MAC bits 3 and 2.
    CHECK_EQ(u.vu.vi[4], 0x000Cu);

    // After the ADD the MAC holds the sign flag of y (bit 6) and no zero flags.
    CHECK_EQ(u.vu.vi[6], 0x0040u);

    // Status: sign (bit 1), plus the remembered zero (bit 6) and sign (bit 7) flags.
    CHECK_EQ(u.vu.vi[7], 0x0002u | 0x0040u | 0x0080u);
}

/** CLIP compares x, y and z with |w| and shifts six bits into the clip flags (documented). */
void test_clip() {
    Unit u;
    u.set(1, 2.0f, -3.0f, 0.5f, 0.0f);
    u.set(2, 0, 0, 0, -1.0f);  // the bound is |w|
    Program p;
    p.hi(clip(1, 2));  // 0: +x and -y are outside: bits 0 and 3
    p.hi(nop());
    p.hi(nop());
    p.lo(fcand(0x000009));  // 3: not visible yet
    p.lo(iadd(3, 1, 0));    // 4: keep that answer in vi3
    p.lo(fcand(0x000009));  // 5: visible since instruction 4
    p.lo(iadd(4, 1, 0));
    p.hi(clip(2, 2));  // 7: inside on every side: shifts the old result up six bits
    p.hi(nop());
    p.hi(nop());
    p.hi(nop());
    p.lo(fcget(5));  // 11
    finish(p);
    u.run(p);

    // The first FCAND is too early to see the result; the second sees it (0x09 AND 0x09 is set).
    CHECK_EQ(u.vu.vi[3], 0u);
    CHECK_EQ(u.vu.vi[4], 1u);

    // The second CLIP moved the old six bits up by six places.
    CHECK_EQ(u.vu.vi[5], 0x09u << 6);
    CHECK_EQ(u.vu.clip, 0x09u << 6);
}

/** Q arrives when the divider finishes; dividing by zero gives the largest number (documented). */
void test_divider() {
    Unit u;
    u.set(1, 4.0f, 9.0f, 0.0f, 0.0f);
    Program p;
    p.lo(div(0, 3, 1, 0));  // 0: Q = vf0.w / vf1.x = 0.25, ready for instruction 7
    for (int n = 1; n <= 5; n++) {
        p.hi(nop());
    }
    p.hi(addq(X, 2, 0));      // 6: still the old Q (0)
    p.hi(addq(X, 3, 0));      // 7: the new one
    p.lo(sqrt(1, 1));         // 8: Q = 3
    p.add(nop(), waitq());    // 9: wait for it
    p.hi(addq(X, 4, 0));      // 10
    p.lo(rsqrt(0, 3, 1, 0));  // 11: Q = 1 / sqrt(4)
    p.lo(waitq());
    p.hi(addq(X, 5, 0));
    p.lo(div(0, 3, 1, 2));  // 14: 1 / 0: the largest number, and the divide flag
    p.lo(waitq());
    p.hi(addq(X, 6, 0));
    p.lo(fsand(7, 0x030));
    finish(p);
    u.run(p);

    // Read before the divider is done, Q is still the old value, 0.
    CHECK(u.vu.f(2, 0) == 0.0f);

    // 1 / 4, then sqrt(9), then 1 / sqrt(4).
    CHECK(u.vu.f(3, 0) == 0.25f);
    CHECK(u.vu.f(4, 0) == 3.0f);
    CHECK(u.vu.f(5, 0) == 0.5f);

    // 1 / 0 is the largest number, with the divide-by-zero flag (status bit 5).
    CHECK_EQ(u.vu.vf[6][0], 0x7FFFFFFFu);
    CHECK_EQ(u.vu.vi[7], 0x020u);
    CHECK_EQ(u.vu.status & 0x800, 0x800u);  // and it is remembered
}

/** P arrives when the function unit finishes; a read before WAITP sees the old P (documented). */
void test_function_unit() {
    Unit u;

    // (3, 4, 12) has length 13, since 9 + 16 + 144 = 169.
    u.set(1, 3.0f, 4.0f, 12.0f, 0.0f);
    Program p;
    p.lo(eleng(1));   // P = sqrt(9 + 16 + 144) = 13
    p.lo(mfp(X, 2));  // too early: the old P
    p.lo(waitp());
    p.lo(mfp(X, 3));
    p.lo(esadd(1));
    p.lo(waitp());
    p.lo(mfp(X, 4));
    finish(p);
    u.run(p);

    // The early read gets the old P; after WAITP the length, then the sum of squares.
    CHECK(u.vu.f(2, 0) == 0.0f);
    CHECK(u.vu.f(3, 0) == 13.0f);
    CHECK(u.vu.f(4, 0) == 169.0f);
}

/** Branches, the branch delay slot, BAL and JR behave as documented. */
void test_branches() {
    Unit u;
    Program p;
    p.lo(iaddiu(1, 0, 3));    // 0: three times round
    p.hi(nop());              // 1
    p.lo(iaddi(2, 2, 1));     // 2: loop
    p.lo(iaddi(1, 1, -1));    // 3
    p.hi(nop());              // 4
    p.lo(ibne(1, 0, 2 - 6));  // 5
    p.lo(iaddi(3, 3, 1));     // 6: the delay slot runs every time
    p.lo(bal(15, 2));         // 7: call 10
    p.hi(nop());              // 8: delay slot
    p.lo(b(3));               // 9: (returned here) skip to 13
    p.lo(iaddiu(4, 0, 77));   // 10: delay slot of the b, and the subroutine's first instruction
    p.lo(jr(15));             // 11
    p.lo(iaddi(5, 5, 1));     // 12: delay slot of the jr
    finish(p);                // 13, 14
    u64 ran = u.run(p);

    // The loop ran three times, and its delay slot (the counter in vi3) ran each time.
    CHECK_EQ(u.vu.vi[2], 3u);
    CHECK_EQ(u.vu.vi[3], 3u);

    // BAL saved the address after its delay slot, 9; the subroutine set vi4 and its delay slot vi5.
    CHECK_EQ(u.vu.vi[15], 9u);
    CHECK_EQ(u.vu.vi[4], 77u);
    CHECK_EQ(u.vu.vi[5], 1u);

    // The program ended at its last pair, and the run was short (it did not run away).
    CHECK_EQ(u.vu.pc, 15u);
    CHECK(ran < 40);
}

/** A branch tests the integer as it was before the instruction ahead of it (measured). */
void test_branch_sees_the_older_integer() {
    // A conditional branch tests a register as it was before the instruction
    // right ahead of it.
    Unit u;
    Program p;
    p.lo(iaddiu(1, 0, 1));  // 0
    p.hi(nop());            // 1
    p.lo(iaddi(1, 1, -1));  // 2: vi1 becomes 0
    p.lo(ibne(1, 0, 2));    // 3: still tests 1: taken, to 6
    p.hi(nop());            // 4
    p.lo(iaddiu(2, 0, 7));  // 5: skipped
    p.lo(iaddi(3, 3, -1));  // 6
    p.hi(nop());            // 7: with an instruction between, the branch sees the new value
    p.lo(ibltz(3, 2));      // 8: taken, to 11
    p.hi(nop());            // 9
    p.lo(iaddiu(4, 0, 7));  // 10: skipped
    finish(p);
    u.run(p);

    // The first branch was taken on the old 1, skipping the write to vi2 (vi1 is 0 by the end).
    CHECK_EQ(u.vu.vi[1], 0u);
    CHECK_EQ(u.vu.vi[2], 0u);

    // vi3 went to -1 and the second branch, seeing it, was taken and skipped the write to vi4.
    CHECK_EQ(u.vu.vi[3], 0xFFFFu);
    CHECK_EQ(u.vu.vi[4], 0u);
}

/** A branch right after a flag read tests the value that read gave (seen in game code). */
void test_branch_after_a_flag_read() {
    /*
     * The instructions that read flags into an integer register are done in
     * time for a branch right after them (the games' clipping code tests two
     * flag reads this way).
     */
    Unit u;
    u.vu.set_f(1, 3, -2.0f);
    Program p;
    p.hi(add(W, 2, 1, 1));  // 0: negative: the sign flag
    p.hi(nop());            // 1
    p.hi(nop());            // 2
    p.hi(nop());            // 3
    p.lo(fsand(5, 2));      // 4: vi5 = 2
    p.lo(ibne(5, 0, 2));    // 5: tests the 2 read before: taken, to 8
    p.hi(nop());            // 6
    p.lo(iaddiu(6, 0, 7));  // 7: skipped
    finish(p);
    u.run(p);

    // The FSAND read the sign flag (status bit 1) and the branch, seeing it, skipped vi6.
    CHECK_EQ(u.vu.vi[5], 2u);
    CHECK_EQ(u.vu.vi[6], 0u);
}

/** A branch whose pair waits for a float register tests the new integer (measured). */
void test_branch_that_waits_sees_the_new_integer() {
    /*
     * A branch whose pair has to wait for a float register no longer tests the
     * old value: the write ahead of it has got through by then.
     */
    Unit u;
    u.vu.set_f(1, 0, 1.0f);
    Program p;
    p.lo(iaddiu(1, 0, 1));                    // 0
    p.hi(nop());                              // 1
    p.add(add(X, 2, 1, 1), iaddi(1, 1, -1));  // 2: vi1 becomes 0, vf2 on its way
    p.add(add(X, 3, 2, 2), ibne(1, 0, 2));    // 3: waits for vf2: tests 0, not taken
    p.hi(nop());                              // 4
    p.lo(iaddiu(4, 0, 7));                    // 5: runs
    finish(p);
    u.run(p);

    // The branch saw 0, so it was not taken and the write to vi4 ran.
    CHECK_EQ(u.vu.vi[1], 0u);
    CHECK_EQ(u.vu.vi[4], 7u);
}

/** Loads, stores and integer arithmetic move the values the instruction definitions say. */
void test_memory_and_integers() {
    Unit u;

    // Quadwords 20 and 21 hold 0x10n and 0x20n in field n, so every value tells where it came from.
    for (u32 f = 0; f < 4; f++) {
        u.set_word(20, f, 0x100 + f);
        u.set_word(21, f, 0x200 + f);
    }
    u.vu.vf[7] = {0x11, 0x22, 0x33, 0x44};
    Program p;
    p.lo(iaddiu(1, 0, 20));
    p.lo(lqi(XYZW, 1, 1));     // vf1 = [20], vi1 = 21
    p.lo(lq(X | Z, 2, 0, 1));  // vf2.xz = [21]
    p.lo(mr32(XYZW, 3, 1));
    p.lo(iaddiu(2, 0, 32));
    p.lo(sqd(XYZW, 7, 2));      // vi2 = 31, [31] = vf7
    p.lo(sq(Y, 7, 3, 2));       // [34].y = vf7.y
    p.lo(ilw(Z, 3, -1, 1));     // vi3 = low half of [20].z
    p.lo(isw(X | W, 3, 5, 2));  // [36].x and .w = vi3
    p.lo(mtir(4, 7, 3));        // vi4 = vf7.w
    p.lo(isubiu(5, 0, 2));      // vi5 = -2
    p.lo(mfir(X, 4, 5));        // vf4.x = -2 as an integer
    p.lo(iand(6, 3, 4));
    p.lo(ior(7, 3, 4));
    p.lo(isub(8, 3, 4));
    p.lo(move(Y, 5, 7));
    finish(p);
    u.run(p);

    // LQI loaded quadword 20 and moved vi1 on to 21; LQ loaded fields x and z of quadword 21.
    CHECK_EQ(u.vu.vf[1][2], 0x102u);
    CHECK_EQ(u.vu.vi[1], 21u);
    CHECK_EQ(u.vu.vf[2][0], 0x200u);
    CHECK_EQ(u.vu.vf[2][1], 0u);
    CHECK_EQ(u.vu.vf[2][2], 0x202u);

    // MR32 rotated the fields: (x, y, z, w) became (y, z, w, x).
    CHECK(u.vu.vf[3] == (std::array<u32, 4>{0x101, 0x102, 0x103, 0x100}));

    // SQD lowered vi2 to 31 and stored vf7 there; SQ then stored only y at 31 + 3.
    CHECK_EQ(u.vu.vi[2], 31u);
    CHECK_EQ(u.word(31, 3), 0x44u);
    CHECK_EQ(u.word(34, 1), 0x22u);
    CHECK_EQ(u.word(34, 0), 0u);

    // ILW took the low half of z of quadword 20; ISW stored it in x and w of quadword 36.
    CHECK_EQ(u.vu.vi[3], 0x102u);
    CHECK_EQ(u.word(36, 0), 0x102u);
    CHECK_EQ(u.word(36, 3), 0x102u);
    CHECK_EQ(u.word(36, 1), 0u);

    // MTIR took vf7.w (0x44); MFIR put -2 in vf4.x as the integer 0xFFFFFFFE.
    CHECK_EQ(u.vu.vi[4], 0x44u);
    CHECK_EQ(u.vu.vf[4][0], 0xFFFFFFFEu);

    // The integer logic and subtraction of vi3 = 0x102 and vi4 = 0x44.
    CHECK_EQ(u.vu.vi[6], 0x102u & 0x44u);
    CHECK_EQ(u.vu.vi[7], 0x102u | 0x44u);
    CHECK_EQ(u.vu.vi[8], 0x102u - 0x44u);
    CHECK_EQ(u.vu.vf[5][1], 0x22u);
    CHECK_EQ(u.vu.unknown_ops, u64{0});
}

/**
 * The four-field arithmetic either declines or gives exactly what the one-field model gives
 * (measured: by comparing the two on pseudo-random operands).
 */
void test_quad_arithmetic() {
    /*
     * The four-at-once arithmetic either declines or gives exactly what the
     * integer model gives, on ordinary numbers, zeros, numbers close together
     * and far apart, and the ends of the range.
     */
    std::fesetround(FE_TOWARDZERO);

    // A fixed seed makes the run repeat; the multiplier and increment are a common linear
    // congruential generator's.
    u32 seed = 12345;

    // Called below for every random number; steps the generator and returns its state.
    auto next = [&seed] {
        seed = seed * 1664525u + 1013904223u;
        return seed;
    };

    // Called below for every operand: picks one of five kinds of number by the top four bits.
    auto value = [&]() -> u32 {
        u32 r = next();

        // 0x807FFFFF keeps the sign and the 23 mantissa bits; the exponent goes in bits 23-30.
        switch ((r >> 28) & 15) {
            case 0:
                return next() & 0x80000000u;  // a zero
            case 1:
                return (next() & 0x807FFFFFu) | (((next() >> 8) % 3) << 23);  // tiny exponents
            case 2:
                return (next() & 0x807FFFFFu)
                       | ((253 + (next() >> 8) % 3) << 23);  // huge exponents
            case 3:
                return next();  // anything
            default:
                return (next() & 0x807FFFFFu)
                       | ((100 + (next() >> 8) % 60) << 23);  // ordinary, near each other
        }
    };
    u64 accepted = 0, wrong = 0;

    // 200,000 random pairs of operand quads, each tried with three operations.
    for (int n = 0; n < 200000; n++) {
        u32 a[4], b[4], out[4];
        for (int f = 0; f < 4; f++) {
            a[f] = value();
            b[f] = value();
        }

        // A random field mask of four bits.
        u32 dest = (next() >> 20) & 15;

        // Operation 0 is add, 1 is subtract (the `true` argument), 2 is multiply.
        for (int op = 0; op < 3; op++) {
            bool ok = op == 0   ? fp::quad_add(a, b, dest, out)
                      : op == 1 ? fp::quad_add(a, b, dest, out, true)
                                : fp::quad_mul(a, b, dest, out);

            // The fast path declined: nothing to compare.
            if (!ok) {
                continue;
            }

            accepted++;

            // Compare each selected field with the one-field model; x is bit 3 of the mask.
            for (unsigned f = 0; f < 4; f++) {
                if (!(dest & (8u >> f))) {
                    continue;
                }

                u32 problems = 0;
                u32 want = op == 0   ? fp::add(a[f], b[f], problems)
                           : op == 1 ? fp::sub(a[f], b[f], problems)
                                     : fp::mul(a[f], b[f], problems);

                // A different value, or a flag the fast path would have had to report.
                if (want != out[f] || problems) {
                    wrong++;
                }
            }
        }
    }

    std::fesetround(FE_TONEAREST);

    // No field differed from the one-field model.
    CHECK_EQ(wrong, u64{0});
#if OPENRAC_FP_QUAD
    // And it does take most of them: over 100,000 of the 600,000 tries.
    CHECK(accepted > 100000);
#endif
}

// --- with the rest of the machine ---

/**
 * Copies words into a byte vector, in memory order.
 *
 * @param words The words.
 * @return The same bytes, four to a word.
 */
std::vector<u8> bytes_of(const std::vector<u32>& words) {
    std::vector<u8> out(words.size() * 4);
    std::memcpy(out.data(), words.data(), out.size());
    return out;
}

/**
 * Encodes a VIF code.
 *
 * @param cmd The command, bits 24-30.
 * @param num The count field, bits 16-23.
 * @param imm The immediate, bits 0-15.
 * @return The code word.
 */
u32 vif(u32 cmd, u32 num, u32 imm) {
    return (cmd << 24) | (num << 16) | imm;
}

/**
 * A display list loads a program and a packet, starts the program, and XGKICK sends the packet
 * one instruction late (documented).
 */
void test_kick_through_vif() {
    /*
     * A display list loads a program and a GIF packet, sets the buffer base and
     * starts the program. The program finds the packet through XTOP, kicks it,
     * and writes the packet's colour in the instruction after the kick, which
     * the kick still picks up.
     */
    Graphics g;

    /*
     * Frame 1 is 64 pixels wide at buffer 0, scissored to 64 by 64; the coordinate offset of
     * 2048 pixels (times 16, as 12.4 fixed point) maps the sprite's coordinates onto it.
     */
    const u32 offset = 2048;
    g.gs.write(gsreg::PRMODECONT, 1);
    g.gs.write(gsreg::FRAME_1, u64{1} << 16);
    g.gs.write(gsreg::XYOFFSET_1, (offset * 16) | (u64{offset * 16} << 32));
    g.gs.write(gsreg::SCISSOR_1, (u64{63} << 16) | (u64{63} << 48));

    Program p;
    p.lo(xtop(1));            // vi1 = where the list put the packet
    p.lo(lq(XYZW, 1, 4, 1));  // the colour, stored after the packet
    p.lo(xgkick(1));
    p.lo(sq(XYZW, 1, 1, 1));  // the packet's second quadword
    finish(p);

    // The sprite's corners, 2 to 5 in x and 2 to 4 in y, in 12.4 fixed point.
    const u64 x0 = (offset + 2) * 16, y0 = (offset + 2) * 16, x1 = (offset + 5) * 16,
              y1 = (offset + 4) * 16;

    // The display list: the program, the buffer setup, the packet's data and the start.
    std::vector<u32> list;
    list.push_back(vif(0x4A, static_cast<u32>(p.words().size() / 2), 0));  // MPG at 0
    list.insert(list.end(), p.words().begin(), p.words().end());
    list.push_back(vif(0x03, 0, 100));     // BASE
    list.push_back(vif(0x02, 0, 0));       // OFFSET: TOPS = BASE
    list.push_back(vif(0x01, 0, 0x0101));  // STCYCL
    list.push_back(vif(0x6C, 5, 0x8000));  // UNPACK V4-32, five quadwords, at TOPS

    /*
     * The GIF packet: a tag (NLOOP 1, EOP, PRE, PRIM 6 = sprite, three PACKED registers), then
     * the register data. The tag's 0x551 lists RGBAQ, XYZ2, XYZ2 (documented).
     */
    const u64 packet[] = {
        1 | (u64{1} << 15) | (u64{1} << 46) | (u64{6} << 47) | (u64{3} << 60),
        0x551,  // sprite: RGBAQ, XYZ2, XYZ2
        0,
        0,  // (the program fills this in)
        x0 | (y0 << 32),
        0,
        x1 | (y1 << 32),
        0,
        0x21 | (u64{0x43} << 32),
        0x65 | (u64{0x87} << 32),  // the colour, one component a word
    };
    for (u64 quad : packet) {
        list.push_back(static_cast<u32>(quad));
        list.push_back(static_cast<u32>(quad >> 32));
    }
    list.push_back(vif(0x14, 0, 0));  // MSCAL 0
    std::vector<u8> bytes = bytes_of(list);
    g.vif.write(bytes.data(), bytes.size());

    // XTOP gave the BASE of 100, the program ran four instructions and the ending's two.
    CHECK_EQ(g.vu1.vi[1], 100u);
    CHECK(g.vu1.stopped());
    CHECK_EQ(g.vu1_instructions, u64{6});

    // The sprite is 3 pixels wide and 2 high: 6 pixels, with the colour the program wrote.
    CHECK_EQ(g.gs.stats.pixels, u64{6});
    CHECK_EQ(g.gs.memory.read(PSMCT32, 0, 1, 2, 2), 0x87654321u);
    CHECK_EQ(g.gs.memory.read(PSMCT32, 0, 1, 4, 3), 0x87654321u);

    // The pixel at x = 5 is the first one outside the sprite.
    CHECK_EQ(g.gs.memory.read(PSMCT32, 0, 1, 5, 3), 0u);
    CHECK_EQ(g.vif.unknown_codes, u64{0});
    CHECK_EQ(g.vu1.unknown_ops, u64{0});
}

/**
 * Appends a little-endian 32-bit word to a file being built.
 *
 * @param file The bytes so far.
 * @param value The word.
 */
void put32(std::vector<u8>& file, u32 value) {
    // Four bytes, the lowest first.
    for (unsigned shift = 0; shift < 32; shift += 8) {
        file.push_back(static_cast<u8>(value >> shift));
    }
}

/**
 * Writes a little-endian word of 16 or 32 bits into a file at a place that exists already.
 *
 * @param file The file.
 * @param at Where.
 * @param value The word.
 * @param bytes 2 or 4.
 */
void poke(std::vector<u8>& file, std::size_t at, u32 value, unsigned bytes) {
    // The lowest byte first.
    for (unsigned n = 0; n < bytes; n++) {
        file[at + n] = static_cast<u8>(value >> (8 * n));
    }
}

/**
 * The use of program memory is counted for each 256 pairs under the content it had, and a
 * game's own table of chunks turns the counts into counts by program.
 */
void test_use_by_program() {
    Unit u;
    Program a;

    // Program A: three pairs at address 0 (two no-operations and the end with its last pair).
    a.add(nop(), lnop());
    finish(a);

    u64 ran = u.run(a);

    // Pair 0, the pair with the E bit and the one after it.
    CHECK_EQ(ran, u64{3});

    // Program B, the same pairs, in the second 256 pairs of program memory (byte 0x800).
    std::memcpy(u.micro.data() + 0x800, a.words().data(), a.words().size() * 4);
    u.vu.program_changed();
    ran = u.vu.run(256);
    fp::want_nearest();
    CHECK_EQ(ran, u64{3});

    std::vector<Vu::Use> uses = u.vu.uses();

    // Two contents of program memory: A alone, then A with B behind it.
    CHECK_EQ(uses.size(), std::size_t{2});

    u64 first = 0, second = 0, starts = 0;

    for (const Vu::Use& use : uses) {
        // VU1's 2048 pairs are eight blocks of 256.
        CHECK_EQ(use.pairs.size(), std::size_t{8});
        first += use.pairs[0];
        second += use.pairs[1];
        starts += use.starts[0] + use.starts[1];
    }

    CHECK_EQ(first, u64{3});
    CHECK_EQ(second, u64{3});
    CHECK_EQ(starts, u64{2});

    /*
     * An executable with a table of two chunks: program 7 at byte 0 and program 9 at byte 0x800,
     * each holding A's 24 bytes. The file is a 52-byte header, one program header, the code,
     * the table, the names, and six section headers.
     */
    const std::size_t code_bytes = a.words().size() * 4;
    const u32 load_address = 0x00100000;
    const std::string name_a = ".DVP.overlay..0x0.7.10.0", name_b = ".DVP.overlay..0x800.9.20.0";
    std::string strings = std::string(1, '\0') + name_a + '\0' + name_b + '\0';
    std::string section_names = std::string(1, '\0') + ".DVP.ovlytab" + '\0' + ".DVP.ovlystrtab"
                                + '\0' + name_a + '\0' + name_b + '\0' + ".shstrtab" + '\0';
    std::vector<u8> elf(52 + 32, 0);

    std::memcpy(
        elf.data(),
        "\x7F"
        "ELF",
        4
    );

    const u32 code_at = static_cast<u32>(elf.size());

    // The code of both chunks, one after the other.
    for (int copy = 0; copy < 2; copy++) {
        const u8* bytes = reinterpret_cast<const u8*>(a.words().data());

        elf.insert(elf.end(), bytes, bytes + code_bytes);
    }

    const u32 table_at = static_cast<u32>(elf.size());

    // Two records: name offset, address in the loaded program, address in the unit.
    put32(elf, 1);
    put32(elf, load_address);
    put32(elf, 0);
    put32(elf, static_cast<u32>(1 + name_a.size() + 1));
    put32(elf, load_address + static_cast<u32>(code_bytes));
    put32(elf, 0x800);

    const u32 strings_at = static_cast<u32>(elf.size());

    elf.insert(elf.end(), strings.begin(), strings.end());

    const u32 section_names_at = static_cast<u32>(elf.size());

    elf.insert(elf.end(), section_names.begin(), section_names.end());

    const u32 headers_at = static_cast<u32>(elf.size());

    // Where each section's name starts in the section name table.
    const u32 n_table = 1, n_strings = n_table + 13, n_a = n_strings + 16;
    const u32 n_b = n_a + static_cast<u32>(name_a.size()) + 1;
    const u32 n_names = n_b + static_cast<u32>(name_b.size()) + 1;

    // One row a section: name, file offset, size. The first is the empty section 0.
    const u32 rows[6][3] = {
        {0, 0, 0},
        {n_table, table_at, 24},
        {n_strings, strings_at, static_cast<u32>(strings.size())},
        {n_a, 0, static_cast<u32>(code_bytes)},
        {n_b, 0, static_cast<u32>(code_bytes)},
        {n_names, section_names_at, static_cast<u32>(section_names.size())},
    };

    for (const auto& row : rows) {
        std::size_t header = elf.size();

        elf.resize(elf.size() + 40, 0);

        // Section header: name at byte 0, file offset at 16, size at 20.
        poke(elf, header, row[0], 4);
        poke(elf, header + 16, row[1], 4);
        poke(elf, header + 20, row[2], 4);
    }

    // File header: program headers at byte 28 (one, at 52), section headers at 32, counts.
    poke(elf, 28, 52, 4);
    poke(elf, 32, headers_at, 4);
    poke(elf, 44, 1, 2);
    poke(elf, 48, 6, 2);
    poke(elf, 50, 5, 2);

    // The one loadable segment: type 1, file offset, address, size in the file.
    poke(elf, 52, 1, 4);
    poke(elf, 52 + 4, code_at, 4);
    poke(elf, 52 + 8, load_address, 4);
    poke(elf, 52 + 16, static_cast<u32>(2 * code_bytes), 4);

    std::vector<VuChunk> chunks = vu_program_chunks(elf);

    CHECK_EQ(chunks.size(), std::size_t{2});

    // The names give program and chunk number, the records the address in the unit.
    if (chunks.size() == 2) {
        CHECK_EQ(chunks[0].program, 7u);
        CHECK_EQ(chunks[0].address, 0u);
        CHECK_EQ(chunks[1].program, 9u);
        CHECK_EQ(chunks[1].address, 0x800u);
        CHECK_EQ(chunks[1].code.size(), code_bytes);
    }

    std::vector<VuProgramUse> programs = vu_program_use(chunks, uses);

    // Both programs ran three pairs from one start each.
    CHECK_EQ(programs.size(), std::size_t{2});

    for (const VuProgramUse& program : programs) {
        CHECK(program.program == 7 || program.program == 9);
        CHECK_EQ(program.pairs, u64{3});
        CHECK_EQ(program.starts, u64{1});
    }

    // A file that is no ELF has no chunks.
    CHECK(vu_program_chunks(std::vector<u8>(100, 0)).empty());
}

}  // namespace

/**
 * Runs every test of the vector unit.
 *
 * @return 0 when every check passed, nonzero otherwise.
 */
int main() {
    const TestCase tests[] = {
        {"arithmetic and masks", test_arithmetic_and_masks},
        {"cross product", test_cross_product},
        {"numbers", test_numbers},
        {"conversions", test_conversions},
        {"a pair reads old values", test_pair_reads_old_values},
        {"immediate", test_immediate},
        {"flags arrive four later", test_flags_arrive_four_later},
        {"clip", test_clip},
        {"divider", test_divider},
        {"function unit", test_function_unit},
        {"branches", test_branches},
        {"a branch sees the older integer", test_branch_sees_the_older_integer},
        {"a branch after a flag read", test_branch_after_a_flag_read},
        {"a branch that waits sees the new integer", test_branch_that_waits_sees_the_new_integer},
        {"memory and integers", test_memory_and_integers},
        {"four-field arithmetic", test_quad_arithmetic},
        {"kick through vif", test_kick_through_vif},
        {"use by program", test_use_by_program},
    };
    return run_tests(tests);
}
