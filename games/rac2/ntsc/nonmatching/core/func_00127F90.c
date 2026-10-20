/* from src/core/00127B88.c: the catalogue row is 212 bytes but the function is 204.
   Its first two words are the linker remnant rac1-decomp keeps with LINKER_REMNANT
   (`addiu $sp,$sp,0x40`, the last word of a stripped function, and the alignment nop
   after it); the real body runs from 0x00127F98. The C below is rac1-decomp's
   func_00124340 (the SDK's libmc RPC 0x10) and reproduces the 204 bytes at 0x00127F98
   exactly: tools/check.py prints "204 bytes, expected 212" and --diff lines the
   candidate up instruction for instruction two words behind retail. Unblocked by
   splitting the row in tools/levels.py, not by more C. */
#include "common.h"

extern char r1_D_00159B00[];

extern int r1_D_00132EA8;

extern int r1_D_00132EAC;

extern void r1_func_00118C90(int);

extern int r1_func_00118CC0(int);

extern int r1_func_0011B4C8();

extern char r1_D_0015B0C0[];

extern int r1_D_00159B80;

/* rac1-decomp src/core/001236F0.c: func_00124340 */
/* RPC 0x10 with two request words, as func_00128180 in src/core/00127B88.c. */
int func_00127F90(int arg0, int arg1) {
    char *cd = r1_D_00159B00;
    int *buf;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    buf = &r1_D_00159B80;
    buf[1] = arg0;
    buf[2] = arg1;
    r = r1_func_0011B4C8(cd, 0x10, 1, buf, 0x30, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 0x10;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}
