/* 0 of 7 places match - candidate is 148 bytes, retail 152 (one instruction).
   File: src/levels/shared/L02_002B5F50-g8.c (-O2 -G8 -ffunction-sections).
   Structure, the 0xFF store, the `slti 0xE` guard chain, the three absolute members,
   both calls and the trailing store are all right. The residue:
     +0x04 mine daddu $a1,$a0,$zero | retail (nothing - self stays in $a0)
     the stores then use $a1 in mine and $a0 in retail, and retail keeps a `nop` in
     the `bne $v1,$v0,->end` delay slot that this cc1 does not emit.
   So this cc1 introduces a copy of the frame argument and drops one delay-slot nop:
   net one instruction short, and the two errors cancel in length but not in bytes.
   Eight spellings were measured (char*/u8*, the flag read as `lb` vs `lbu`, the two
   store orders, nosda vs plain vs array-typed members, nested guards vs early
   returns): all give 148, or 144 with the members gp-relative. */

#include "include_asm.h"

typedef unsigned char u8;
typedef signed int s32;

extern s32 D_L02_gp5FA8 __attribute__((sda));
extern s32 D_001A9048 __attribute__((nosda));
extern s32 D_001A79F0 __attribute__((nosda));
extern u8 D_001A7A6B __attribute__((nosda));
extern s32 D_001A903C __attribute__((nosda));

extern void func_002E5E58(int, int, int);
extern void func_002B0A18(int, int);

void func_L02_00389F70(char *self) {
    s32 v;

    if (*(u8 *)(self + 0x20) == 0) {
        *(u8 *)(self + 0x20) = 1;
        *(u8 *)(self + 0x30) = 0xFF;
        v = D_001A9048;
    } else {
        v = D_L02_gp5FA8;
    }
    if (v < 14)
        return;
    if (D_001A79F0 != 16)
        return;
    if (D_001A7A6B != 0)
        return;
    D_001A7A6B = 1;
    func_002E5E58(1, 0, 0);
    func_002B0A18(0x1233, -1);
    D_001A903C = 1;
}
