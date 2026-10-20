/* func_L09_00403700, 152 bytes, 1 placement (level 9).  Best candidate 160 bytes (one
   instruction more in the loop tail and one extra `nop` after the `lbu`).
   Two measured gaps: (1) the `if (*(u8 *)(a0 + 0x20) != 7) return;` guard comes out as
   `bnel` with the epilogue's `ld $s0,0x0($sp)` in the annulled slot, where retail keeps
   `bne` and fills the slot with `lw $s2,0x68($s1)` hoisted out of the body; (2) the trip
   test lands in $v0 against retail's $v1.  The rest - the 0x40000001 argument, 0.17f
   (0x3E2E147B), the `cvt.s.w` of the sub-object's byte +0x34 and the `bnel` back edge -
   is instruction-exact.
   The body: */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern void func_318450(char *a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5,
                        f32 f12, f32 f13, f32 f14);

void func_L09_00403700(char *a0) {
    char *s2;
    s32 i;

    if (*(u8 *)(a0 + 0x20) != 7) {
        return;
    }
    s2 = *(char **)(a0 + 0x68);
    for (i = 0; i < 4; i++) {
        func_318450(a0, i, 0x40000001, 0, 1, 0, 0.17f, (f32)*(u8 *)(s2 + 0x34),
                    1.0f);
    }
}
