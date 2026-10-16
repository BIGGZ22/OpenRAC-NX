/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002A5158.c (games/rac1) through the version
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
/* functional: high. Same calls (VU0 sqrt func_001F9988, func_001F99C0), loads and stores as retail; only the address arithmetic differs and one redundant re-read of the patch's neighbour byte +0xD is folded, which cannot change behaviour (plain memory, nothing writes it in between). */
/* FUN_L00_002a3ec8 -- src/overlays/shared/vendor_002A5138.c (functional C for the port, not a match)
 * Pokes a round disturbance into the level's water height grids (a ripple/splash impulse).
 * buf is an array of `count` 0x1190-byte grid patches; each patch has its origin at +0/+4,
 * eight neighbour patch indices at +0xC..+0x13 (0xFF = none) and two 0x5C0-byte height buffers
 * from +0x50 (selected by the frame indices D_L00_00161218 = current, D_L00_00161220 = other): a 16x16
 * float field (rows of 0x40) followed by copies of the neighbours' edge rows/columns/corners.
 * For each patch, the cells within `radius` cells of (x, y) (in cell units from the
 * D_L00_001CA880 grid origin/cell size) get amount / dist (amount when dist < 1):
 *   mode 0: written to the current buffer unless the cell's current value has a larger magnitude;
 *   mode != 0: added to the cell's value from the other buffer and written to the current one.
 * Cells on the patch border are also copied into the matching edge slots of the neighbours.
 * Called by the hero's water code (help_002180F8, help_00221A98) with (grids, count, 0, x, y, r, a).
 * equiv: DIFFERENT by shape only: retail folds +0x50 and the row/column offsets into strength-
 * reduced base registers (addiu 0x3C/0x3C0/0x4/0x50, lwc1 r+0) where this uses r+0x50/0x8C/0x410;
 * retail reloads %hi of D_L00_00161220, re-reads byte +0xD once more and repeats n*0x1190 in both
 * arms of the r<2 test (by the counts, merged here); the compiler's a0 at the func_001F99C0 call is not an argument. */
extern float func_001F9B50_A5158(float) __asm__("func_001F9988"); /* VU0 sqrt */
extern float func_001F9B88_A5158(float) __asm__("func_001F99C0"); /* fabsf */
extern float D_L00_001CAB80_A5158[] __asm__("D_L00_001CA880");
#define d D_L00_001CAB80_A5158
extern int D_001612D8_A5158 __asm__("D_L00_00161218") NOT_SDA;
extern int D_001612E0_A5158 __asm__("D_L00_00161220") NOT_SDA;

#define F(p) (*(float *)(p))
#define CELL(g, buf_, r, c) F((g) + 0x50 + (buf_) * 0x5C0 + (r) * 0x40 + (c) * 4)
#define NB(n, buf_) (base + (n) * 0x1190 + (buf_) * 0x5C0)

void FUN_L00_002a3ec8(char *buf, int count, int mode, float x, float y, float radius, float amount) {
    char *base = buf;
    char *g = buf;
    int i;

    for (i = 0; i < count; i++) {
        float oy = F(g + 4) + d[3];
        float fy = (y - oy) / d[5];
        float fx = (x - (F(g + 0) + d[2])) / d[4];
        int r0 = (int)(fy - radius);
        int r1 = (int)(fy + radius);
        int c1 = (int)(fx + radius);
        int c0 = (int)(fx - radius);
        float py;
        int r;

        if (r0 < 0) r0 = 0;
        if (r1 >= 16) r1 = 15;
        if (c1 >= 16) c1 = 15;
        if (c0 < 0) c0 = 0;
        py = oy + d[5] * (float)r0;

        for (r = r0; r <= r1; r++) {
            float px = (F(g + 0) + d[2]) + d[4] * (float)c0;
            float dy2 = (y - py) * (y - py);
            int c;

            for (c = c0; c <= c1; c++) {
                float dx = x - px;
                float d2 = dx * dx + dy2;
                float v = amount;
                int cur, n;

                if (!(d2 < 1.0f)) {
                    v = amount / func_001F9B50_A5158(d2);
                }
                if (mode == 0) {
                    float a = func_001F9B88_A5158(v);
                    if (a < func_001F9B88_A5158(CELL(g, D_001612D8_A5158, r, c))) {
                        v = CELL(g, D_001612D8_A5158, r, c);
                    }
                } else {
                    v += CELL(g, D_001612E0_A5158, r, c);
                }
                cur = D_001612D8_A5158;
                CELL(g, cur, r, c) = v;

                if (r < 2 && (n = (unsigned char)g[0xC]) != 0xFF) {
                    if (r == 0) {
                        F(NB(n, cur) + c * 4 + 0x4D0) = CELL(g, cur, 0, c);
                    } else {
                        F(NB(n, cur) + c * 4 + 0x550) = v;
                    }
                }
                if (c < 2 && (unsigned char)g[0xD] != 0xFF) {
                    cur = D_001612D8_A5158;
                    n = (unsigned char)g[0xD];
                    if (c == 0) {
                        F(NB(n, cur) + r * 4 + 0x510) = CELL(g, cur, r, 0);
                    } else {
                        F(NB(n, cur) + r * 4 + 0x590) = CELL(g, cur, r, c);
                    }
                }
                if (r == 15 && (unsigned char)g[0xE] != 0xFF) {
                    cur = D_001612D8_A5158;
                    n = (unsigned char)g[0xE];
                    F(NB(n, cur) + c * 4 + 0x450) = CELL(g, cur, 15, c);
                }
                if (c == 15 && (unsigned char)g[0xF] != 0xFF) {
                    cur = D_001612D8_A5158;
                    n = (unsigned char)g[0xF];
                    F(NB(n, cur) + r * 4 + 0x490) = CELL(g, cur, r, 15);
                }
                n = (unsigned char)g[0x10];
                if (n != 0xFF) {
                    if (c == 0 && r == 0) {
                        cur = D_001612D8_A5158;
                        F(NB(n, cur) + 0x5DC) = CELL(g, cur, 0, 0);
                    }
                    if (c == 1 && r == 0) {
                        cur = D_001612D8_A5158;
                        F(NB((unsigned char)g[0x10], cur) + 0x5E0) = CELL(g, cur, 0, 1);
                    }
                    if (c == 0 && r == 1) {
                        cur = D_001612D8_A5158;
                        F(NB((unsigned char)g[0x10], cur) + 0x5E4) = CELL(g, cur, 1, 0);
                    }
                }
                n = (unsigned char)g[0x11];
                if (n != 0xFF && c == 15 && r == 0) {
                    cur = D_001612D8_A5158;
                    F(NB(n, cur) + 0x5D8) = CELL(g, cur, 0, 15);
                }
                n = (unsigned char)g[0x12];
                if (n != 0xFF && c == 0 && r == 15) {
                    cur = D_001612D8_A5158;
                    F(NB(n, cur) + 0x5D4) = CELL(g, cur, 15, 0);
                }
                n = (unsigned char)g[0x13];
                if (n != 0xFF && c == 15 && r == 15) {
                    cur = D_001612D8_A5158;
                    F(NB(n, cur) + 0x5D0) = CELL(g, cur, 15, 15);
                }
                px += d[4];
            }
            py += d[5];
        }
        g += 0x1190;
    }
}
