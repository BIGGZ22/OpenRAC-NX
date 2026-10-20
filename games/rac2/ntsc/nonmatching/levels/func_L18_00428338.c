/* 0 of 3 places match - candidate is 176 bytes, retail 180 (one instruction).
   File: src/levels/shared/L18_00427BE0-g8.c (-O2 -G8 -ffunction-sections).
   Everything the body needs is present and correct: the `lbu 0x42 != 10` early
   return that leaves the success path without writing `$v0`, `$s2` in the `bne`
   delay slot, both `func_32C5E0(self, 2|3, buf)` calls with their buffers at
   `$sp`/`$sp+0x10`, the `mtc1` + separator `nop` + `cvt.s.w` pair, the
   `xori $v0,$v0,0x1` + `movz $a3,$v1,$v0` select between 0x40000001 and
   0x40100000, and the 9-argument `func_3370F0` call.
   The one instruction is in the select group: retail evaluates `lbu $v0,0xBC($s2)`,
   `lui $v1,0x401` and `ori $a3,0x40000001` AFTER the float conversion, this cc1
   hoists `lbu 0xBC` and `ori $a3` above the `mtc1`, and the group then costs one
   instruction less. Four spellings (ternary inline, if/else into a local, `(f32)`
   vs an int argument, one buffer vs two) all give 176. */

#include "include_asm.h"

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern f32 D_L18_gp22D0 __attribute__((sda));

extern void func_0032C5E0(char *, int, void *);
extern void func_003370F0(char *, void *, void *, int, f32, f32, int, int, int);

s32 func_L18_00428338(char *self) {
    u8 buf[16];
    u8 buf2[16];
    char *s2;
    int mode;

    if (*(u8 *)(self + 0x42) != 10)
        return 0;
    s2 = *(char **)(self + 0x68);
    func_0032C5E0(self, 2, buf);
    func_0032C5E0(self, 3, buf2);
    if (*(u8 *)(self + 0xBC) == 1)
        mode = 0x40100000;
    else
        mode = 0x40000001;
    func_003370F0(self, buf, buf2, mode, (f32)*(u8 *)(s2 + 0x34), D_L18_gp22D0,
                  2, 3, 0);
}
