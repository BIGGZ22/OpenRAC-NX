/* Carried from port/game/rac1-pal/hand/level_liquid_grid.c through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern unsigned char D_L02_001CB9C0[];
/* the blocks' shared texture coordinates */
extern void func_L00_001FDE98(int n, unsigned char *xyz, unsigned char *colours, unsigned char *uvs, int flag);
#define LIQUID_STRIP 124



void FUN_L02_0024fa80(unsigned char *rec) {
    unsigned char *flags = (unsigned char *)0x70002000;      /* the cull's flag per block */
    const short *order = (const short *)0x70002800;          /* the strip's grid points, as byte offsets */
    float *grid = (float *)0x70000000;                       /* the block's 8 x 8 points, x y each */
    const float x0 = *(float *)(rec + 0x00);
    const float y0 = *(float *)(rec + 0x04);
    const float z = *(float *)(rec + 0x08);
    const float cx = *(float *)(rec + 0x10);
    const float cy = *(float *)(rec + 0x14);
    unsigned char *colours = *(unsigned char **)(rec + 0x34);
    float xyz[LIQUID_STRIP * 3];
    float by = y0;
    int rows = *(unsigned short *)(rec + 0x1A);
    int r, c, k;

    for (; rows > 0; rows -= 7, by += cy * 7.0f) {
        int cols = *(unsigned short *)(rec + 0x18);
        float bx = x0;
        for (; cols > 0; cols -= 7, bx += cx * 7.0f, colours += 0x200) {
            const int flag = *flags++;
            if (flag == 0) {
                continue;
            }
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    grid[(r * 8 + c) * 2] = bx + cx * (float)c;
                    grid[(r * 8 + c) * 2 + 1] = by + cy * (float)r;
                }
            }
            for (k = 0; k < LIQUID_STRIP; k++) {
                const float *p = (const float *)((unsigned char *)grid + order[k]);
                xyz[k * 3] = p[0];
                xyz[k * 3 + 1] = p[1];
                xyz[k * 3 + 2] = z;
            }
            /* Retail's entry is 12 + (flag - 1) * 2; func_L00_001FDE98 takes 14 for flag bit 0. */
            func_L00_001FDE98(LIQUID_STRIP, (unsigned char *)xyz, colours + 0x10, D_L02_001CB9C0 + 0x10,
                              flag == 2 ? 1 : 0);
        }
    }
}
