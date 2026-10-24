#include "common.h"
#include "structs.h"

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void r1_func_0012D000();

typedef unsigned int u128 __attribute__((mode(TI), aligned(16)));

extern void r1_func_0012D000(unsigned int chcr);

extern volatile u128 r1_D_00133150[];

extern u128 r1_D_001331A0[];

#define IPU_CMD ((volatile unsigned int *)0x10002000)

#define IPU_CTRL ((volatile int *)0x10002010)

#define IPU_IN_FIFO ((volatile u128 *)0x10007010)

/* rac1-decomp src/core/0012CC90.c: func_0012CCF8 */
void func_00130E20(unsigned int chcr) {
    r1_func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    r1_func_0011D9A8();
}

/* rac1-decomp src/core/0012CC90.c: func_0012D068 */
/* sceIpuInit: reset the IPU, load its two fixed FIFO command tables, reset again. Adapted from Lombyte (GPL-3.0) for PAL: src/sdk/dma/sce_ipu_init.c, sceIpuInit. */
void func_00130E88(void) {
    r1_func_0012D000(1);
    *IPU_CTRL = 0x40000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0;
    while (*IPU_CTRL < 0) {
    }
    *IPU_IN_FIFO = r1_D_00133150[0];
    *IPU_IN_FIFO = r1_D_00133150[1];
    *IPU_IN_FIFO = r1_D_00133150[2];
    *IPU_IN_FIFO = r1_D_00133150[3];
    *IPU_IN_FIFO = r1_D_00133150[4];
    *IPU_IN_FIFO = r1_D_00133150[4];
    *IPU_IN_FIFO = r1_D_00133150[4];
    *IPU_IN_FIFO = r1_D_00133150[4];
    *IPU_CMD = 0x50000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0x58000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_IN_FIFO = r1_D_001331A0[0];
    *IPU_IN_FIFO = r1_D_001331A0[1];
    *IPU_CMD = 0x60000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0x90000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CTRL = 0x40000000;
    while (*IPU_CTRL < 0) {
    }
    *IPU_CMD = 0;
    while (*IPU_CTRL < 0) {
    }
}

/* func_001310C0 (e_sqrt) is transcribed in src/core/001310C0.c */

INCLUDE_ASM("asm/core", func_001313C4);
