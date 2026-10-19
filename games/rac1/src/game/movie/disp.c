#include "common.h"
#include "structs.h"

/*
 * movie/disp.cpp in the original source; text 0x23C5E0-0x23C9C0.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* setImageTag (recovered name), written from the retail assembly: writes a DMA/GIF tag chain into p.
 * A 4-qword header (the destination from D_0015EF84 >> 8), then one 0x60-byte entry per 16x16
 * block of a (w >> 4) x (h >> 4) grid, each pointing at the next 0x400 bytes of image, then a
 * 0x20-byte trailer and the end tag. */
extern int D_0015EF84 MACRO_ADDR;
extern char *D_0015EFB8 MACRO_ADDR;
extern char D_00151EF0[];

void func_0023C5E0(unsigned long *p, unsigned int image, int w, int h) {
    int mbx, mby;
    int i, j;

    p[0] = 0x10000003;
    p[1] = 0;
    p[2] = 0x1000000000000002;
    p[3] = 0xE;
    p[4] = ((long)(D_0015EF84 >> 8) << 32) | 0x0008000000000000;
    p[5] = 0x50;
    p[6] = 0x0000001000000010;
    p[7] = 0x52;
    p += 8;
    mbx = w >> 4;
    mby = h >> 4;
    for (i = 0; i < mbx; i++) {
        for (j = 0; j < mby; j++) {
            p[0] = 0x10000004;
            p[1] = 0;
            p[2] = 0x1000000000000002;
            p[3] = 0xE;
            p[4] = ((unsigned long)(i << 4) << 32) | ((unsigned long)(j << 4) << 48);
            p[5] = 0x51;
            p[6] = 0;
            p[7] = 0x53;
            p[8] = 0x0800000000000040;
            p[9] = 0;
            ((unsigned int *)p)[20] = 0x30000040;
            ((unsigned int *)p)[21] = image & 0x0FFFFFFF;
            p[11] = 0;
            image += 0x400;
            p += 12;
        }
    }
    ((unsigned int *)p)[0] = 0x30000009;
    ((unsigned int *)p)[1] = (unsigned int)(D_0015EFB8 + 0xC0) & 0x0FFFFFFF;
    ((unsigned int *)p)[2] = 0;
    ((unsigned int *)p)[3] = 0x50000009;
    ((unsigned int *)p)[4] = 0x30000025;
    ((unsigned int *)p)[5] = (unsigned int)D_00151EF0;
    ((unsigned int *)p)[6] = 0;
    ((unsigned int *)p)[7] = 0x50000025;
    p += 4;
    p[0] = 0x70000000;
    p[1] = 0;
}
ASM_FUNC("asm/handwritten/text", func_0023C7A8); /* vblankHandler */
ASM_FUNC("asm/handwritten/text", func_0023C910); /* handler_endimage */
extern volatile int D_001612E0 MACRO_ADDR;   /* display-active flag, shared with the vblank handler */
extern volatile int D_001612E4 MACRO_ADDR;
extern int func_00122598(int);

/* startDisplay(int) -- waits until the vertical-sync field differs from the given one, then
 * marks the display active. */
void startDisplay(int field) {
    while (func_00122598(0) == field) {
    }
    D_001612E0 = 1;
    D_001612E4 = 0;
}
/* endDisplay(void) -- clears the display-active flag. */
void endDisplay(void) {
    D_001612E0 = 0;
}
