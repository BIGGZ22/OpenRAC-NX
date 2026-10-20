/* func_L10_00425798 (240 bytes, 1 place) - the body is byte-identical; only the prologue
 * block is permuted.  Measured with scratch/wslcmp.py: size EXACT at 240, first difference
 * at +0x4 ("instruction at +0x4 differs").  check.py not run (not a match).
 *
 * Body, exact: walk the +0x28 chain from the head pointer at 0x001B2528, compare the +0xAA
 * halfword against nine ids (0x1315 first, then 0x1327, 0x1328, 0x10F9, 0x946, 0x10CF,
 * 0x117C, 0x117D, 0x117F), call func_32AF78(p) on a hit, advance with the load in the
 * annulled delay slot of the last `bnel`, and loop on `bnel $s0,$zero`.  This cc1 lays that
 * out byte for byte, including which eight constants it hoists into $s1-$s7/$fp and which
 * one (0x1315) it leaves as an in-loop `addiu`.
 * What is left is the 12-word prologue: retail emits `sd $s0,0x0($sp)` first, then
 * `lui $s0,0x1B` / `lw $s0,0x2528($s0)`, then the other seven saves and finally the loop's
 * `beqz $s0` with `sd $ra` in its delay slot.  This cc1 emits the seven saves first, then
 * `sd $s0`, then the `lui`/`lw`, and the `beqz` after them.  Five spellings measured - the
 * head as a literal, as a symbol, a declared-then-assigned local, a `u8 **head`
 * intermediate, an early `if (p == 0) return;` - all land on the same permutation, so the
 * length is right and only the order and the `$hi`/value register pair differ.
 * To land it: a way to keep the `$s0` save and the head load in retail's order (the same
 * prologue-interleave wall as func_L01_00394148 below).
 */
typedef unsigned char u8;
typedef signed short s16;

extern void func_0032AF78(void *a0);

void func_L10_00425798(void) {
    u8 *p = *(u8 **)0x001B2528;

    while (p != 0) {
        s16 id = *(s16 *)(p + 0xAA);

        if (id == 0x1315 || id == 0x1327 || id == 0x1328 || id == 0x10F9 ||
            id == 0x946 || id == 0x10CF || id == 0x117C || id == 0x117D ||
            id == 0x117F)
            func_0032AF78(p);
        p = *(u8 **)(p + 0x28);
    }
}
