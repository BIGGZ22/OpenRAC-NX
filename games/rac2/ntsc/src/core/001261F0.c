#include "common.h"
#include "structs.h"

/* libgraph's sceGsGParam, as far as this reads it. */
typedef struct {
    short interlace; /* 0x00 */
    short omode;     /* 0x02 */
    short ffmode;    /* 0x04 */
    short version;   /* 0x06 */
    int unk_08;      /* 0x08 */
} GsGParam;

extern GsGParam *r1_func_00121D08(void);

extern void r1_func_00118ED0(void);

extern long r1_func_00118F60(void);

extern void r1_func_0011A6C8();

extern void r1_func_0011A6C8(void *);

extern char r1_D_001532D0[];

typedef struct {
    long q[12];
} GsLoadImage;

/* rac1-decomp src/core/001224B0.c: func_00122598 */
/* Returns the current field, bit 13 of the GS CSR (read directly at
   0x12001000, or through r1_func_00118F60), when the display is
   interlaced, and 1 otherwise. In the second arm the bit is taken from
   the call's result before the interlace test, as retail does. */
int func_001261F0(void) {
    GsGParam *gp = r1_func_00121D08();
    long csr;

    if (gp->unk_08 == 0) {
        r1_func_00118ED0();
        if (gp->interlace != 1) {
            return 1;
        }
        return (*(volatile unsigned long *)0x12001000 >> 13) & 1;
    }
    csr = (r1_func_00118F60() >> 13) & 1;
    if (gp->interlace != 1) {
        return 1;
    }
    return csr;
}

INCLUDE_ASM("asm/core", func_00126284);

INCLUDE_ASM("asm/core", func_0012646C);

/* rac1-decomp src/core/00122818.c: func_00122958 */
/* sceGsExecLoadImage(src, dst): waits for the GS transfer unit
 * (0x1000A000 bit 8) to go idle, starts an image load from src (its
 * 0x70000000 scratchpad segment tagged with bit 31) sized by the 15-bit
 * field in src->q[10], waits again and starts the transfer to dst.
 * Returns -1 after 0x1000000 polls in total. The abort is written out in
 * each loop, not shared through a goto: the compiler merges the two copies
 * into retail's single call, where a shared label needs an extra branch. */
int func_001265B0(GsLoadImage *src, void *dst) {
    unsigned int count = 0;

    while (*(volatile unsigned int *)0x1000A000 & 0x100) {
        if (count++ > 0x1000000) {
            r1_func_0011A6C8(r1_D_001532D0);
            return -1;
        }
    }

    *(volatile unsigned int *)0x1000A020 = 6;
    if (((unsigned int)src & 0x70000000) == 0x70000000) {
        *(volatile unsigned int *)0x1000A010 = ((unsigned int)src & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile unsigned int *)0x1000A010 = (unsigned int)src & 0x0FFFFFFF;
    }
    *(volatile unsigned int *)0x1000A000 = 0x101;

    while (*(volatile unsigned int *)0x1000A000 & 0x100) {
        if (count++ > 0x1000000) {
            r1_func_0011A6C8(r1_D_001532D0);
            return -1;
        }
    }

    *(volatile unsigned int *)0x1000A020 = (unsigned int)(int)(src->q[10] & 0x7FFF);
    if (((unsigned int)dst & 0x70000000) == 0x70000000) {
        *(volatile unsigned int *)0x1000A010 = ((unsigned int)dst & 0x0FFFFFFF) | 0x80000000;
    } else {
        *(volatile unsigned int *)0x1000A010 = (unsigned int)dst & 0x0FFFFFFF;
    }
    *(volatile unsigned int *)0x1000A000 = 0x101;
    return 0;
}

INCLUDE_ASM("asm/core", func_0012672C);

INCLUDE_ASM("asm/core", func_00126DBC);
