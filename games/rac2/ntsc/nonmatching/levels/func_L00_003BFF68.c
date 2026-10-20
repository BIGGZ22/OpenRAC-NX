/* func_L00_003BFF68 (200 bytes, 12 places) -- NEAR MISS. Closest row of the wave-7 tour 2: the
   size is exact (200 bytes) and every instruction is present; the first difference is the
   prologue's save order, at +0x4.
     003BFF6C  sd $s0,0x50   |  sd $s1,0x58
     003BFF70  sd $s1,0x58   |  sd $s2,0x60
     003BFF74  sd $s2,0x60   |  daddu $s1,$a0,$zero
     003BFF78  daddu $s1,$a0 |  swc1 $f20,0x70
     003BFF7C  sd $ra,0x68   |  daddu $s2,$a1,$zero
     003BFF80  daddu $s2,$a1 |  sd $s0,0x50
     003BFF84  swc1 $f20,0x70|  daddu $a1,$sp,$zero
     003BFF88  daddu $a1,$sp |  sd $ra,0x68
   Retail emits the callee-saved saves in liveness order (s1, s2, f20, s0, ra) with the two
   register moves interleaved between them; this cc1 groups them in ascending register number
   (s0, s1, s2, ra, f20) and then runs the moves. Everything downstream follows from that: the
   `a1 = s2+0xB4` delay-slot fill of the func_002AAA00 call and the `swc1 $f0,0x8($sp)` fill of the
   func_002833F0 call both sit one slot away too.

   Tried and not enough: `-mno-split-addresses` (identical output, byte for byte).

   What is measured and right (all of it below):
   - the frame is 0x80 with the scratch area at sp+0x00..0x4F exactly as retail: the 16-byte
     copied block at 0x00, the float at +8 inside it, the 16-byte pair-helper output at +0x10 and
     the zeroed word at +0x40 - one `char scratch[0x50]` reproduces the whole layout;
   - the 16-byte copy takes the offset-0 form (`lq $v0,0x0($a2)` / `sq $v0,0x0($sp)`), which is the
     case the TI128 destination wall does NOT block - offsets must be zero for this (see
     func_L22_0040DD40, where the +0x10 folds);
   - the first call is `f(a0, scratch, a3, a3)` (both a2 and a3 hold the fourth argument), and the
     last statement is `func_00282C48(f) - func_00282C48(*(float *)(a1 + 0xB4))`: the second call's
     `mov.s $f20,$f0` lands in its own delay slot, so retail subtracts the TWO CALLS' results, not
     the same value twice. */

typedef unsigned char u8;
typedef float f32;

typedef int Rac2TI128 __attribute__((mode(TI)));

extern f32 func_002B04E0(char *a0, void *out, int a2, int a3);

extern void func_002AAA00(void *out, char *in, int a2, f32 f12, f32 f13, f32 f14, f32 f15);

extern void func_002833F0(void *out, void *in);

extern void func_00283738(char *a0, char *a1, void *a2);

extern f32 func_00282C48(f32 f);

/* Copies the caller's 16-byte block into a stack scratch area, runs the four-argument starter on
 * it, then fills the scratch area's upper half from the state block's 0xB4 float and the pair
 * helper before folding the starter's result against the state's own reading of that float. */
f32 func_L00_003BFF68(char *a0, char *a1, char *a2, int a3)
{
    char scratch[0x50];
    f32 f;

    *(Rac2TI128 *)scratch = *(Rac2TI128 *)a2;
    f = func_002B04E0(a0, scratch, a3, a3);
    *(int *)(scratch + 0x40) = 0;
    func_002AAA00(scratch + 0x40, a1 + 0xB4, 0, f, *(f32 *)(a1 + 0x14), *(f32 *)(a1 + 0x18),
                  3.14159274f);
    *(Rac2TI128 *)scratch = 0;
    *(f32 *)(scratch + 8) = *(f32 *)(a1 + 0xB4);
    func_002833F0(scratch + 0x10, scratch);
    func_00283738(a0 + 0xC0, a0 + 0xC0, scratch + 0x10);
    return func_00282C48(f) - func_00282C48(*(f32 *)(a1 + 0xB4));
}
