#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_func_00118C70(void *);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118C70();

extern int r1_D_0012FDA0;

extern int r1_D_0012FDA4;

     /* DeleteSema */
extern int r1_func_00118CB0(int);

extern s32_1B4C8 r1_func_00118CB0();

extern char r1_D_00157E80[];

extern s32 r1_func_00118C90(s32);

extern s32 r1_func_00118CB0(s32);

/* rac1-decomp src/core/00119D88.c: func_0011B710 */
/*
 * One-shot init: if r1_D_0012FDA0 is still -1, fill a semaphore descriptor
 * (the kernel's SemaParam: initCount [2] = 1, maxCount [1] = 1, option
 * [5] = 0) and create two semaphores from it via r1_func_00118C70
 * (CreateSema), storing them into r1_D_0012FDA0 and r1_D_0012FDA4.
 *
 * Under 2.9-ee the store order decides which store lands in the first
 * call's delay slot: written initCount, maxCount, option it is exact
 * (retail puts buf[2] in the slot). Under the game's 2.95.3 no order was
 * exact (8/92 at best), which is what kept this one open.
 */
void func_0011D868(void) {
    int buf[8];
    if (r1_D_0012FDA0 == -1) {
        buf[2] = 1;
        buf[1] = 1;
        buf[5] = 0;
        r1_D_0012FDA0 = r1_func_00118C70(buf);
        r1_D_0012FDA4 = r1_func_00118C70(buf);
    }
}

/* rac1-decomp src/core/00119D88.c: func_0011B770 */
/*
 * Claim the first free slot of the 32-entry, 0x10-byte table r1_D_00157E80
 * -- the table func_0011D950 below indexes -- under the
 * r1_func_00118CB0/r1_func_00118C90 lock (WaitSema/SignalSema). A slot is free
 * when its +4 word is zero; claiming it writes 0x10000000 there and
 * returns the slot, and a full table returns 0. Both exits drop the lock
 * first.
 *
 * Exact under 2.9-ee with the C that 2.95.3 compiled one instruction
 * short: retail spends a third callee-saved register ($17 holds
 * %hi(r1_D_0012FDA0) for the whole function), which is 2.9-ee's allocation;
 * 2.95.3 parked the %hi in $4 instead.
 */
void *func_0011D8C8(void) {
    char *p;

    func_0011D868();
    r1_func_00118CB0(r1_D_0012FDA0);
    for (p = r1_D_00157E80; p < r1_D_00157E80 + 0x200; p += 0x10) {
        if (*(int *)(p + 0x4) == 0) {
            *(int *)(p + 0x4) = 0x10000000;
            r1_func_00118C90(r1_D_0012FDA0);
            return p;
        }
    }
    r1_func_00118C90(r1_D_0012FDA0);
    return 0;
}
