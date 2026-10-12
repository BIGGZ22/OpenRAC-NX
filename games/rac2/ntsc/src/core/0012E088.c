#include "common.h"
#include "structs.h"

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void func_0012E088(void *, int);

/* rac1-decomp src/core/00125630.c: func_0012A268 */
/* receiveDataFromIPU (libmpeg.a:mpc.o): kicks off an IPU->main-memory DMA
 * receive. r1_func_0011D960 presumably starts/queries the DMA channel; its
 * result gates whether r1_func_0011D9A8 (presumably a completion/kick) runs.
 * The three MMIO stores (0x1000B000/10/20 -- D2_CHCR/D2_MADR/D2_QWC, the
 * IPU_to_RAM DMA channel) queue the transfer: MADR = (arg0 physical addr)
 * | 0x80000000 (spr flag), QWC = arg1 >> 4 (bytes -> qwords), CHCR = 0x100
 * (start, from-memory... actually to-memory chain/normal mode).
 */
void func_0012E088(void *arg0, int arg1) {
    int r = r1_func_0011D960();
    *(volatile unsigned int *)0x1000B010 = ((unsigned int)arg0 & 0x0FFFFFFF) | 0x80000000;
    *(volatile unsigned int *)0x1000B020 = arg1 >> 4;
    *(volatile unsigned int *)0x1000B000 = 0x100;
    if (r != 0) {
        r1_func_0011D9A8();
    }
}

INCLUDE_ASM("asm/core", func_0012E10C);
