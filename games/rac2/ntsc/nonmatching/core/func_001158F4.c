/* from src/core/00115200.c: two things, both about the row itself.
 *
 * 1. The row starts four bytes early, in the linker fill. The first word of
 *    the row is 0xCDCDCDCD (the retail linker's gap pattern); the function's
 *    real entry is 0x001158F8, which symbol_addrs/usa-v2 already pins:
 *    `func_001158F8 = 0x001158f8; // type:func`. The preceding strlen ends at
 *    0x1158F4, so the fill is exactly those four bytes and the body runs
 *    0x1158F8..0x115ABF (456 bytes). A `tools/levels.py` split unblocks the
 *    row; the same shape as the recorded func_00127218 and func_0011FB8C rows.
 *
 * 2. The body itself is the SDK's hand-written strncmp, not C. Proved by
 *    name, not by shape: the vendored newlib source is in this repo's own
 *    build tree at
 *      build/sdk-compiler-build/source/ee/newlib/libc/machine/r5900/strncmp.S
 *    and it opens with exactly the two instructions at 0x1158F8:
 *      bne  $6,$0,$L3     ->  +0  bnez $a2,+0x0C
 *      or   $3,$4,$5      ->  +4  or   $v1,$a0,$a1
 *    followed by `jr $ra` / `daddu $v0,$zero,$zero` for the n == 0 early
 *    return, which is +8/+0x0C in the row. (strncat.S, the other candidate at
 *    this size, opens with `move $8,$4` instead, so it is not this one.)
 *
 * What no C spelling reaches (the reason it is not a C candidate):
 *   - the 16-byte path loads with lq (+0x50/+0x58) and compares with MMI
 *     opcode 0x1C word instructions (`psubb`/`pand`/`pcpyud` class, funct
 *     0x09 visible in 0x71295389 at +0x58) that rabbitizer prints as
 *     `.word # INVALID`, building the 0x80808080... mask with an
 *     ori/dsll chain;
 *   - the medium path is ld/dsubu/nor/and doubleword zero detection, and the
 *     tail is the scalar `subu $v0,$v1,$v0` return in the jr delay slot;
 *   - there is no prologue and the paths are stitched with bnel/beql, exactly
 *     the .S file's label-free macro shape.
 * symbol_addrs records that this group assembled by the SN Ps2EeAs reproduces
 * the ROM byte-exact (FACT 9816) while EE gas gives objects two words shorter
 * (the dli expansion), which is the same caveat as its strcmp/strncpy twins.
 *
 * Recommendation (maintainer decision): ASM_FUNC for the body once the row is
 * split; the two changes are independent and the split helps 0x115ABF's
 * neighbour strncpy too.
 */
