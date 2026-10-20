/* func_L23_00425568 (392 bytes, 1 place: 23:00425568) -- NEAR MISS, not integrated.
   This C compiles to 392 bytes, retail's size; five words differ, all in the `j == 4` arm of the
   sample loop. check.py (23_damosel_orbit, the -G0 unit L23_0041E418.c) reports
   "instruction at +0x110 differs". It needs the TI16 typedef above the marker (the unit declares it
   further down).
   Right: the frame and save order, every saved register (&loc[3] in $s4, &loc[2] in $s5, &loc[1] in
   $s6, &loc[4] in $s7, i + 1 in $fp), both func_L23_004260F0 calls, the call to
   func_L00_0031AA28, and the join block with the loop test.
   Two corrections to the earlier attempt made that possible: func_L00_0031AA28 returns nothing and
   writes its point through its first argument (retail reloads loc[3] after the call), and the
   loop's accesses to loc[3] go through a pointer (`out`), which gives &loc[3] the most references
   and so $s4.
   What still differs (retail on the right):
     lw  $v0,0x20($s0)        | lw  $v1,0x20($s0)
     lq  $v1,0($v0)           | lq  $v0,0($v1)
     sq  $v1,0($s4)           | sq  $v0,0($s4)
     b   -> sq $v0,0($s1)     | b   -> lq $v0,0($s4)  (the shared reload)
     lq  $v0,0x30($sp)        | nop
   Retail behaves as if each `lq; sq` copy of the loop were ONE unsplittable insn: the join's copy
   is never interleaved with j++ (sched1 always moves j++ and slti between a C TImode load and its
   store), and the arm's `b` keeps an empty slot although an `sq` stands right before it (reorg's
   backward scan stops at an asm insn). Retail puts `sq` in `b` slots elsewhere, and the same
   `lq; sq; b; nop` shape is in func_002FEE70. That points at the inline-asm quadword copy helpers
   (docs/COMPILER-NOTES.md, "Quadword copy source shapes"), which plain C cannot use.
   Measured without success: the store written once after the if/else (388 bytes, j++ scheduled
   between lq and sq, sq taken into the b slot); the store in both arms (388 bytes: CSE forwards the
   arm's value, so only the final sq is cross-jumped); a and b as scalar TI16 locals (ADDRESSOF; the
   frame layout stays right but CSE2 still forwards); a `for` loop (j and p swap registers, same
   join); the unit compiled with -g (no change).
*/

typedef unsigned char u8;

typedef int s32;

typedef int TI16 __attribute__((mode(TI)));

extern void func_00282CF0(float *, float *, float *);

extern float func_00282DF0(float *);

extern void func_L00_0031AA28(TI16 *out, float *a, float *b, TI16 *c, TI16 *d, float t);

void func_L23_004260F0(u8 *obj, float *dst, TI16 *src1, TI16 *src2, float scale);

/* blends the object's position toward its target: two control points, then five curve samples */
void func_L23_00425568(u8 *obj) {
    TI16 loc[5];
    TI16 va;
    TI16 vb;
    TI16 *out;
    TI16 *p;
    u8 *s0;
    float f20;
    s32 i;
    s32 j;
    s32 n;

    i = 0;
    s0 = *(u8 **)(obj + 0x68);
    do {
        func_00282CF0((float *)&loc[0], *(float **)(s0 + 0x20), (float *)(obj + 0x10));
        n = i + 1;
        j = 0;
        p = (TI16 *)(s0 + 0x30);
        f20 = func_00282DF0((float *)&loc[0]) * *(float *)(s0 + 0x1C);
        vb = *(TI16 *)(obj + 0x10);
        va = **(TI16 **)(s0 + 0x20);
        loc[4] = vb;
        loc[3] = va;
        func_L23_004260F0(obj, (float *)&loc[1], &loc[3], &loc[4], f20);
        loc[3] = *(TI16 *)(obj + 0x10);
        loc[4] = **(TI16 **)(s0 + 0x20);
        func_L23_004260F0(obj, (float *)&loc[2], &loc[3], &loc[4], f20);
        out = &loc[3];
        do {
            float f = (float)j * 0.2f;

            if (j == 4) {
                *out = **(TI16 **)(s0 + 0x20);
                *p = loc[3];
            } else {
                func_L00_0031AA28(out, (float *)(obj + 0x10), *(float **)(s0 + 0x20), &loc[1], &loc[2], f);
                *p = *out;
            }
            j++;
            p++;
        } while (j < 5);
        i = n;
    } while (i <= 0);
}
