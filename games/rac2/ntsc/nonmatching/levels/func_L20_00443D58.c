/* near-miss: func_L20_00443D58 (20_yeedil, 152 bytes).
   Reaches retail's exact 0x60 frame and its `swc1 $f20,0x50($sp)` save once the helper
   output is re-read into a second float local (`g`) after the 0x443B60 call.
   0x2c: retail sets `$a3 = $sp+0x20` before `$a2 = $s2` for the 0x443B60 call, this cc1
   the other way round; every other word matches. 
   Third wave (20_yeedil, 2026-10-20) re-measured it again with a word-level differ that
   byte-swaps the retail's words (the disassembly file prints them big-endian): the attempt
   is 38 of 38 words and exactly TWO differ, the pair at +0x2C/+0x34.  Retail sets
   `$a3 = $sp+0x20` before the store and `$a2 = $s2` after it; this cc1 sets `$a2 = $s2`
   before the store and `$a3 = $sp+0x20` after it.  Everything else - the 0x60 frame, the
   `swc1 $f20,0x50($sp)` save, the constant's `lui/ori/mtc1` at the top, the `lwc1 $f20,
   0x20($sp)` re-read, the second call's five arguments with the `cvt.s.w` and its `nop` -
   matches.  Measured and not enough: the declaration order of the locals; `f32 f = 1.0471977f`
   as a declaration; the store written through a cast or through a pointer; `volatile` on f
   (41 words); the g re-read before the random call or merged into the second call (35
   words - the `g = f` has to sit directly after the first call); the argument types of the
   0x443B60 prototype permuted (f32*/TI128*, void*/f32*, all void*); the `&f` argument
   written as `(&f)`; and a named `TI128 *p1 = &v1` for the third argument - that one is
   worse (5 words): with a pointer variable this cc1 passes the two TI128* arguments in
   $a1/$a2 the other way round ($a1 = &v1, $a2 = &v0), and it does put the `$a3 = $sp+0x20`
   set-up first as the retail does.  Wall: one scheduling pair, the store of f between the
   two argument set-ups.
    Fourth wave (20_yeedil, 2026-10-20, worker w20-01) re-measured the wall with the
    `g = f` spelling and confirms it: the attempt is 36 of 38 words and exactly TWO differ,
    the pair at +0x2C/+0x34 - retail sets `$a3 = $sp+0x20` before the store of f and
    `$a2 = $s2` after it, this cc1 the other way round.  The call's two argument set-ups
    are emitted in the order of the call's operands (the third argument's copy of $s2
    first, the fourth argument's address of f second), and this cc1 always emits them that
    way; the retail has them the other way round, which no source spelling reaches.
    Measured and not enough: the argument set-up of the third argument written as an
    address computation instead of a register copy (it then lands in $a2 as
    `addiu $a2,$sp,0x10` and the copy of $s2 moves to $a3 - the pair swaps places but the
    rest of the body changes with it); the two locals as one array (`TI128 v[2]`); the
    locals as a struct (`struct S { TI128 a; TI128 b; float c; }`); the third argument as a
    named pointer variable (worse: the two TI128* arguments change registers); the
    prototype's argument types permuted (`void *`, `char *`, `TI128 *`); `f` written through
    a cast; `f` initialised in its declaration; the declaration order of the locals; and
    the per-file flags `-g`, `-mno-split-addresses`, `-fno-schedule-insns`,
    `-fno-schedule-insns2`, `-O1`, `-O3`, `-mcpu=5900` (all worse or unchanged).
    Wall: one scheduling pair, the store of f between the two argument set-ups.
    Second wave (20_yeedil, 2026-10-20) re-measured this one with the `g = f` spelling
   recorded above: it does reach retail's exact 0x60 frame and its `swc1 $f20,0x50($sp)`
   save, and the only residue left is the pair at +0x2C - retail sets `$a3 = $sp+0x20`
   before `$a2 = $s2` for the 0x443B60 call, this cc1 the other way round (the store
   `swc1 $f0,0x20($sp)` sits between them in both). Tried and not enough: `float f =
   1.0471977f` as a declaration instead of an assignment; a `float *pf = &f` temporary
   (much worse, 34 words differ); the declaration order of the locals. Wall: one
   scheduling pair.*/
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_L20_00443B60(char *, TI128 *, TI128 *, f32 *);
extern s32 func_002A7820(s32, s32);
extern void func_L20_00449FD0(char *, TI128 *, TI128 *, s32, f32, f32);

void func_L20_00443D58(char *arg0)
{
    char *state = *(char **)(arg0 + 0x68);
    TI128 v0;
    TI128 v1;
    f32 f;
    f32 g;
    s32 r;

    f = 1.0471977f;
    func_L20_00443B60(arg0, &v0, &v1, &f);
    g = f;
    r = func_002A7820(0x12C, 0x258);
    func_L20_00449FD0(arg0, &v0, &v1, r, g, (f32)*(u8 *)(state + 0x36));
}

/* ---- wave 3 (20_yeedil, 2026-10-20, worker 02): the scheduling pair, swept --------
 *
 * The candidate is still 36 of 38 words with the pair at +0x2C/+0x34 swapped (retail:
 * `$a3 = $sp+0x20` before the store of f and `$a2 = $s2` after it; this cc1: the
 * other way round).  The sweep of the per-file flags on this body:
 *
 *   -fno-schedule-insns           the store of f moves into the `jal`'s delay slot and
 *                                 $s1/$s2 swap - worse.
 *   -fno-schedule-insns2          the pair keeps its order; the body changes - worse.
 *   -fno-expensive-optimizations  the third argument's set-up becomes an address
 *                                 computation (`addu $6,$sp,0x10`) - worse.
 *   -fexpensive-optimizations, -fno-gcse, -fthread-jumps, -fno-thread-jumps,
 *   -foptimize-comparisons, -fno-optimize-comparisons, -fpeephole2, -fno-peephole2,
 *   -fsched-interblock, -fno-sched-interblock, -fsched-spec-load, -fbranch-count-reg,
 *   -fregmove, -fno-regmove, -foptimize-register-move, -frerun-cse-after-loop,
 *   -fdelayed-branch, -fno-delayed-branch, -fstrict-aliasing, -funaligned-pointers,
 *   -O1, -O3, -mips3, -mcpu=r5900   the pair is unchanged (only -O1 and
 *                                 -funaligned-pointers change the body, for the worse).
 *
 * So no per-file flag moves the store of f between the two argument set-ups: this
 * cc1 emits the set-ups in the order of the call's operands, and the retail has them
 * the other way round.  If a flag ever does move it, the function needs a unit of
 * its own (the flag changes the other bodies of a shared unit) - the row would be
 * `src/levels/20_yeedil/L20_00443D58.c <flags>`.
 */
