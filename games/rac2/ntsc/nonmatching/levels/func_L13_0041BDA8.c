/* 0 of 1 places match - last line: 13_boldan: 224 bytes, expected 220
   The staged shape is recovered: the `a0[0x20] == 0` opener that sets a0[0x20] = 1 and
   a0[0x30] = 0xFF, then the eight-entry walk of the 0x68 object's u32 ids
   (`*ids == -1` and func_32A4D0(D_00189EA0, *ids) guards, the
   0x00282F20(D_00189EA0 + 0x270, D_001B2A0C + (*ids << 7), -1.0f) call and the
   10.0f store at D_00189E20 + 0x2FC).
   Retail recomputes the base inside the loop (`lui $s3,0x19` hoisted once, then
   `addiu $s1,$s3,-0x6160` per iteration, `addiu $a0,$s1,0x270`) and runs the counter
   down with the test at the loop BOTTOM (`addiu $s2,$s2,-1` inside the body,
   `bgez $s2` at the end). This cc1 hoists both halves of the address into $s4/$s5/$s6,
   rotates the loop with the test at the top and comes out 224 bytes with a different
   callee-saved set ($s6 instead of the fifth).
   Measured with three spellings (`for (i = 8; --i >= 0;)`, `i = 8; while (--i >= 0)`,
   `for (i = 8; i > 0; i--)`): all 224. A shape that keeps the counter's decrement in
   the body and the test at the bottom, plus a base that is rematerialised per
   iteration, is what it needs. */

typedef unsigned char u8;
typedef int s32;
typedef float f32;

extern u8 D_00189EA0[];
extern u8 D_00189E20[];
extern s32 D_L13_001B2A0C;
extern s32 func_32A4D0(u8 *, s32);
extern void func_2F8A20(u8 *, u8 *, f32);

void func_L13_0041BDA8(u8 *a0) {
    s32 *ids;
    s32 i;

    if (a0[0x20] == 0) {
        a0[0x20] = 1;
        a0[0x30] = 0xFF;
    }
    ids = *(s32 **)(a0 + 0x68);
    for (i = 8; --i >= 0; ) {
        if (*ids == -1)
            break;
        if (func_32A4D0(D_00189EA0, *ids) == 0)
            break;
        func_2F8A20(D_00189EA0 + 0x270, (u8 *)(D_L13_001B2A0C + (*ids << 7)), -1.0f);
        *(f32 *)(D_00189E20 + 0x2FC) = 10.0f;
        ids++;
    }
}
