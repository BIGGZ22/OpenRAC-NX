/* func_L23_00418650 (124 bytes, 1 place, L23_00416388.c, -O2 -G0) -- measured 0 of 1 with
   the C below: exact size, TWO pairs of words swapped (four words), both scheduling.
   (Wave 6 recorded three pairs; the third, the float constant, is fixed here.)

   Retail builds a sixteen-byte vector in a stack local through three shared helpers and
   copies it back over the argument with lq/sq.

   Fixed since wave 6: the float global is read through `lui $at` because it is declared
   sda in this -G0 unit (cc1 emits the bare symbol, the assembler expands it through $at).
   Plain, it splits into `lui $v0` + `lwc1`. (D_L23_001B36A4 is level data, not D_001B36A4.)

   Left:
   1) +0x14/+0x18: this cc1 emits `daddu $s1,$a0,$zero` before `addiu $s0,$s0,%lo(D_00189ED0)`;
      retail has the address half first. The sched2 dump (-dR -fsched-verbose-4) shows the two
      insns tied: same priority, four dependents each, so the LUID decides, and sched1 always
      issues the parameter copy in its first cycle (beside the `lui`), ahead of the lo_sum.
   2) +0x60/+0x64: `lq $v0,0($sp); ld $s0,0x10($sp); sq $v0,0($s1)` against retail's
      `lq; sq; ld $s0`. The restores are RTX_UNCHANGING frame loads, which true_dependence never
      orders after an ordinary store, so sched2 fills the lq delay with `ld $s0`. Retail never
      does: across the boot and all 27 levels, 64 functions end in `lq; sq; <restore>` and
      none in `lq; <restore>; sq` (none of the 64 is C yet). -fno-schedule-insns2 gives
      retail's tail but loses the prologue interleave, so this looks compiler-side (how the
      retail scheduler orders TI stores against the restores), not a source shape.

   Tried without change to the two pairs: a TI local instead of char[16], a `V4 *dst` or a
   `char *base` local, a V4 * parameter, a local pointer to the vector, the sum in a float
   local, and the flags -fno-schedule-insns, -fno-gcse, -fno-strength-reduce,
   -fno-rerun-cse-after-loop, -fno-caller-saves and -g. -mno-split-addresses fixes the float
   too, but makes D_00189ED0 one `la` (lui/addiu adjacent, where retail splits them).
*/

typedef int V4_L23_418650 __attribute__((mode(TI)));

extern char D_00189ED0[];
extern float D_L23_001B36A4 __attribute__((sda));
extern void func_00282CF0(char *dst, char *src, char *arg);
extern void func_00282F20(char *dst, char *src, float arg);
extern void func_00282CC0(char *dst, char *src, char *arg);

void func_L23_00418650(char *arg0, float arg1) {
    char local[16];

    func_00282CF0(local, arg0, D_00189ED0);
    func_00282F20(local, local, arg1 + D_L23_001B36A4);
    func_00282CC0(local, local, D_00189ED0);
    *(V4_L23_418650 *)arg0 = *(V4_L23_418650 *)local;
}
