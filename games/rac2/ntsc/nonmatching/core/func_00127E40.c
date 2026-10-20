/* from src/core/00127B88.c: the catalogue row is 336 bytes but the function is 328.
   Its first two words are the linker remnant rac1-decomp keeps with LINKER_REMNANT
   (`addiu $sp,$sp,0x30`, the last word of a stripped function, and the alignment nop
   after it); the real body runs from 0x00127E48. The C below is rac1-decomp's
   func_001241F0 (the SDK's sceMcGetDir) and reproduces the 328 bytes at 0x00127E48
   exactly: tools/check.py prints "328 bytes, expected 336" and --diff lines the
   candidate up instruction for instruction two words behind retail. Unblocked by
   splitting the row in tools/levels.py, not by more C. */
#include "common.h"

extern char r1_D_00159B00[];

extern int r1_D_00132EA8;

extern int r1_D_00132EAC;

extern void r1_func_00118C90(int);

extern int r1_func_00118CC0(int);

extern void *r1_func_00116B00(void *, const void *, int);

extern int r1_func_0011AD70(void *, int);

extern int r1_func_0011B4C8();

extern char r1_D_00159BB0[];

extern char r1_D_0015B0C0[];

/* rac1-decomp src/core/001236F0.c: func_001241F0 */
/* sceMcGetDir-style RPC 0xD (libmc): name (non-empty), mode, the table
   size and the table, whose cache is written back first (r1_func_0011AD70)
   when count >= 0. Shape as sceMcOpen (func_00127888). */
int func_00127E40(int port, int slot, char *name, int mode, int count, void *table) {
    char *cd = r1_D_00159B00;
    char *fp;
    int r;

    if (*(int *)(cd + 0x24) == 0) {
        return -100;
    }
    if (r1_func_00118CC0(r1_D_00132EAC) < 0) {
        return -200;
    }
    if (name == 0 || *name == 0) {
        r1_func_00118C90(r1_D_00132EAC);
        return -210;
    }
    fp = r1_D_00159BB0;
    *(int *)(fp + 0x0) = port;
    *(int *)(fp + 0x4) = slot;
    *(int *)(fp + 0x8) = mode;
    *(int *)(fp + 0xC) = count;
    *(void **)(fp + 0x10) = table;
    r1_func_00116B00(fp + 0x14, name, 0x3FF);
    fp[0x413] = 0;
    if (count >= 0) {
        r1_func_0011AD70(table, count << 6);
    }
    r = r1_func_0011B4C8(cd, 0xD, 1, fp, 0x414, r1_D_0015B0C0, 4, 0, 0);
    if (r == 0) {
        r1_D_00132EA8 = 0xD;
    } else {
        r1_func_00118C90(r1_D_00132EAC);
    }
    return r;
}
