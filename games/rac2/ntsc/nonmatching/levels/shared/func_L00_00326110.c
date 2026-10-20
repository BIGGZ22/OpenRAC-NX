/* 0 of 18 places match - last line: 00_aranos_tutorial: 180 bytes, expected 188
   The body is recovered: mode = func_00282E48(a0 + 0x10, D_00189EA0) < 7.0f ? 3 : 1
   (retail's `c.olt.s $f0,$f1` against 7.0f, `bc1f` with `addiu $s1,$zero,1` in the
   delay slot and `addiu $s1,$zero,3` after), then the clamp of D_001A79F0 with
   `sltiu`/`movz` against 25, the D_002A0D50 table load and the product written as the
   R5900 three-operand `mult $a1,$a1,$s2` (word 0x00B22818, rd = the low product)
   followed by `addiu $a1,$a1,-1`, and the tail call func_L00_00321878 with SIX integer
   arguments plus f12 (the `-1` in $t0 and the `0` in $t1 are retail's).
   The 8 bytes are one saved pair: retail keeps the mode in $s1 (callee-saved) and the
   second argument in $s2, this cc1 gives the mode $t0 and the argument $s1, so one
   `sd`/`ld` pair is missing. Same shape with the ternary or the sltiu/movn spell
   (the select comes out `movn $v0,$a3,$v1` instead of retail's `movz $v0,$a2,$a1`).
   Measured with three spellings, all 180; `-mno-split-addresses` gives 172/180. */

typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef float f32;

extern u8 D_00189EA0[];
extern u32 D_001A79F0;
extern s32 D_002A0D50[];
extern f32 func_00282E48(u8 *, u8 *);
extern void func_L00_00321878(u8 *, s32, s32, s32, s32, s32, f32);

void func_L00_00326110(u8 *a0, s32 a1, f32 f12) {
    s32 mode = func_00282E48(a0 + 0x10, D_00189EA0) < 7.0f ? 3 : 1;
    u32 i = D_001A79F0;

    func_L00_00321878(a0, D_002A0D50[i < 26 ? i : 25] * a1 - 1, a1, mode, -1, 0, f12);
}
