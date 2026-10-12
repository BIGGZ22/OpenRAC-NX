#include "common.h"
#include "structs.h"

/* sceIpuStopDMA: request the IPU-in (D4/toIPU) DMA channel to stop
 * (func_00130B18(1)), save its MADR/TADR/QWC/CHCR into env[0..3], wait for
 * the IPU's queued-command count (IPU_CTRL bits 4-7) to drain, stop the
 * IPU-out (D3/fromIPU) channel (func_00130AB0(0)) and save its
 * MADR/QWC/CHCR plus IPU_BP and IPU_CTRL into env[4..8]. */
extern void func_00130B18(unsigned int);

extern void func_00130AB0(unsigned int);

/* rac1-decomp src/core/0012CC90.c: func_0012CD60 */
void func_00130B80(unsigned int *env) {
    func_00130B18(1);
    env[0] = *(volatile unsigned int *)0x1000B410;
    env[1] = *(volatile unsigned int *)0x1000B430;
    env[2] = *(volatile unsigned int *)0x1000B420;
    env[3] = *(volatile unsigned int *)0x1000B400;
    while ((*(volatile unsigned int *)0x10002010 & 0xF0) != 0) {
    }
    func_00130AB0(0);
    env[4] = *(volatile unsigned int *)0x1000B010;
    env[5] = *(volatile unsigned int *)0x1000B020;
    env[6] = *(volatile unsigned int *)0x1000B000;
    env[7] = *(volatile unsigned int *)0x10002020;
    env[8] = *(volatile unsigned int *)0x10002010;
}
