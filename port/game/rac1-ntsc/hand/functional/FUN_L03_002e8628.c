/* Carried from the PAL decompilation's nonmatching/shared/func_L03_002E99F0.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* NON_MATCHING FUN_L03_002e8628 -- src/overlays/shared/vendor_00292AC0.c
 * Best so far: SIZE ours 264 / retail 260, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Moby init: resets the data block (0x5C=1.5, 0x50=0, +0x80/+4/+8/+0xC), picks a 32-byte table entry by short at
 *   Remaining diff (p2.c, 46 words): retail hoists the 0.02f/0.1f constant loads before the 1.5f store (f0,f2,f1 l
 *   Constants/stores reordering and local-float wordings all compile to the same bytes; needs a different source s
 *   mini13 a01: initializer p8 reaches260 bytes by sharing the data pointer for final three stores; floating const
 *   Verified mapping wall: config/overlays/functions.tsv maps func_001F97B0 to both03:001F88A0 and03:002405A0; equ
 *   Stopped under lead mapping rule; unblock needs a distinct valid existing symbol/mapping repair. No numeric fun
 *   hq3 s07: p11 (the 0x70 reloads split into two groups, as retail) brings the size to 260 (48/260). Left: the ja
 */
extern void func_001F97B0(void);
extern void func_001FA050(void *, void *);
extern int D_L03_0015EF50 MACRO_ADDR;

// Initialise a moby from a table entry: reset its data block, then build its matrix.
void FUN_L03_002e8628(char *moby) {
    char *tab = (char *)D_L03_0015EF50;
    char *q = *(char **)(moby + 0x70) + 0x10;
    char *e;
    char *d;
    float a[4];
    float b[16];
    e = tab + *(short *)(moby + 0x84) * 32;
    *(float *)(q + 0x4C) = 1.5f;
    *(int *)(q + 0x40) = 0;
    d = *(char **)(moby + 0x70);
    *(int *)(d + 0x80) = 0;
    d = *(char **)(moby + 0x70);
    *(float *)(d + 4) = 0.02f;
    *(float *)(d + 8) = 0.1f;
    *(int *)(d + 0xC) = 0;
    if (*(short *)(moby + 0x84) < 0) {
        func_001F97B0();
    } else {
        *(float *)(q + 0x4C) = *(float *)(*(char **)(e + 0x1C) + 0x18);
    }
    qcopy(moby + 0x30, e);
    qcopy(a, e + 0x10);
    func_001FA050(b, a);
    qcopy(moby, b);
    qcopy(moby + 0x10, b + 4);
    qcopy(moby + 0x20, b + 8);
    qcopy(moby + 0x40, moby);
    *(short *)(moby + 0x7E) = 0;
}
