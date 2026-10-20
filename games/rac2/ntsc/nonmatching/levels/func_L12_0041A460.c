/* 0 of 2 places match - last line: 12_todano: 168 bytes, expected 204
   Retail's body is decoded and the ordinary half of it is reproduced: the 0x110 index into
   the 0x1FEB40 table, the 0x224 offset scaled by 16, the two position differences from the
   0x10/0x14 pairs, the two-float call, the second call with (s1, slot+0x10, angle), and the
   `& 0x8F` / 0x224-vs-0x220 advance tail.
   THE BLOCKER is the 16-byte copy the frame carries. Retail reserves a 0x30 frame (0x20
   here) and, between the slot pointer and the floats, runs
   `addiu $v1,$v1,0x10 ; lq $at,0x0($v1) ; sq $v0,0x0($sp)` - a quadword load out of
   slot+0x10 stored to $sp+0 - and NOTHING in the 204 bytes reads $sp+0 again (the floats are
   re-read from the slot pointer and the second call gets slot+0x10 as its address). An
   unused `TI128 v = *(TI128 *)(p + 0x10);` local is dropped by this cc1 and leaves the 168
   bytes below, so the copy needs the source idiom that makes retail keep a 16-byte value
   live across the first call; the frame difference is the same 16 bytes.
   Also measured: 0x001FEB40 must be spelled as a SYMBOL - a bare literal emits
   `lui $a0,0x1F ; ori $a0,$a0,0xEB40` where retail has `lui $v1,0x20 ; addiu $v1,$v1,-0x1540`. */
#include "include_asm.h"

typedef signed int s32;
typedef float f32;

extern f32 func_00283248(f32 a, f32 b);
extern int func_002B50E0(char *a, char *b, f32 c);

/* Aim pass over the slot the 0x110 record names: measure the two position differences,
   turn them into an angle, hand the slot to the shared pass, and step the 0x224 cursor
   when the pass reports one of the 0x8F bits. */
s32 func_L12_0041A460(char *a0)
{
    char *s1 = a0;
    char *s2 = *(char **)(s1 + 0x68);
    char *tbl = (char *)0x001FEB40;
    s32 s0 = *(s32 *)(tbl + *(s32 *)(s2 + 0x110) * 4);
    s32 off = *(s32 *)(s2 + 0x224);
    char *p = s0 + (off << 4);
    f32 dx;
    f32 dy;
    f32 r;
    s32 n;

    dx = *(f32 *)(p + 0x10) - *(f32 *)(s1 + 0x10);
    dy = *(f32 *)(p + 0x14) - *(f32 *)(s1 + 0x14);
    r = func_00283248(dx, dy);
    if ((func_002B50E0(s1, p + 0x10, r) & 0x8F) != 0) {
        n = *(s32 *)(s2 + 0x224);
        if (n != *(s32 *)(s2 + 0x220))
            *(s32 *)(s2 + 0x224) = n + 1;
    }
    return 0;
}
