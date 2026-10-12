#include "common.h"
#include "structs.h"

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void func_00130AB0(unsigned int);

/* sceIpuStopDMA: request the IPU-in (D4/toIPU) DMA channel to stop
 * (func_00130B18(1)), save its MADR/TADR/QWC/CHCR into env[0..3], wait for
 * the IPU's queued-command count (IPU_CTRL bits 4-7) to drain, stop the
 * IPU-out (D3/fromIPU) channel (func_00130AB0(0)) and save its
 * MADR/QWC/CHCR plus IPU_BP and IPU_CTRL into env[4..8]. */
extern void func_00130B18(unsigned int);

/* rac1-decomp src/core/0012CC90.c: func_0012CC90 */
/*
 * Write a CHCR value to IPU DMA channel 3 (fromIPU, 0x1000B000) or 4
 * (toIPU, 0x1000B400): with interrupts off (r1_func_0011D960/r1_func_0011D9A8),
 * set D_ENABLEW.CPND (0x1000F590 = D_ENABLER | 0x10000) to hold the DMAC,
 * store the value, release the hold. The final EI is a tail call.
 *
 * Exact under 2.9-ee as the plain volatile MMIO sequence. It was blocked
 * under 2.95.3 only because that compiler never emits a sibling call:
 * with a frame of its own the last call stayed a `jal` plus a full
 * epilogue, 3 words over (0x70 against 0x64).
 */
void func_00130AB0(unsigned int chcr) {
    r1_func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    r1_func_0011D9A8();
}

/* rac1-decomp src/core/0012CC90.c: func_0012CCF8 */
void func_00130B18(unsigned int chcr) {
    r1_func_0011D960();
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B400 = chcr;
    *(volatile unsigned int *)0x1000F590 = *(volatile unsigned int *)0x1000F520 & 0xFFFEFFFF;
    r1_func_0011D9A8();
}
