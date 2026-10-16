/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002DED98.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it. */
extern void func_001F9A28(void *, void *, void *);
extern char D_L00_00166DC0[] NOT_SDA;

/* NON_MATCHING FUN_L00_002dd8e8 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 956 / retail 948, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_002dd8e8 (6 of 10 runs used; best p5.c, 956 vs 948 bytes): draws a 9x20 grid of quads (func_L00_001FD
 *   scroll angles D_L00_00161B48/1C/20; per quad: 4 qcopies from tables D_L00_001E6020/6230, vector math, dot-prod
 *   D_L00_001E6008. Packet struct is 0x90 bytes at sp+0 (note: sizeof(long)==8 and long long is 128-bit here, so t
 *   Structure, loop shape, delay slots and store orders all match. Two differences remain: (1) retail has no `mov.
 *   0.3f used by the pre-loop `a += 0.3f` and by the loop is one pseudo in $f24, ours keeps a separate temp and co
 *   stack-address spill slots (D8..F8) come out in a different order (ours rotates [ve,q1] to the end).
 *   Unblock: whatever source shape makes the pre-loop add share the loop-invariant 0.3f pseudo; then slot order ma
 */
extern short D_L00_001E6008[];
extern short D_L00_00161B48;
extern short D_L00_00161B4C;
extern short D_L00_00161B50;
extern char D_L00_001E6020[];
extern char D_L00_001E5EE0[];
extern int func_001F44B8(int);
extern void func_001F9AD8(void *, void *, void *);
extern float func_001F9AB0(void *, void *);
extern void func_L00_001FD228(void *, void *, int);
extern void func_00233980(int, long);

typedef struct {
    float q[4][4];
    unsigned int col[4];
    float x0, y0, x1, y1, x2, y2, x3, y3;
    long t0, t1, t2, t3;
} Pkt_c2dd8e8;

// Draws a grid of ten rows by twenty strips, stepping the two scroll angles and coloring each strip from a per-row table.
void FUN_L00_002dd8e8(void) {
    Pkt_c2dd8e8 s;
    float vc[4];
    float vd[4];
    float ve[4];
    float vf[4];
    float a, a2, am, b, b2, dot;
    int i, j, k, v;
    unsigned short y;
    unsigned int *o;
    s.t1 = func_001F44B8(0x15);
    s.t2 = 0xFF90000000000260L;
    s.t3 = 0x8000000048L;
    s.t0 = 0;
    a = *(float *)&D_L00_00161B48;
    *(float *)&D_L00_00161B48 = a + 0.001f;
    if (*(float *)&D_L00_00161B48 > 7.0f) *(float *)&D_L00_00161B48 -= 7.0f;
    a += 0.3f;
    for (i = 1; i < 10; i++) {
        b = *(float *)&D_L00_00161B4C;
        *(float *)&D_L00_00161B4C = b + *(float *)&D_L00_00161B50;
        if (*(float *)&D_L00_00161B4C > 7.0f) *(float *)&D_L00_00161B4C -= 7.0f;
        a2 = a + 0.3f;
        for (j = 0; j < 20; j++) {
            qcopy(s.q[0], D_L00_001E6020 + (i * 0x140 + j * 16));
            qcopy(s.q[1], D_L00_001E6020 + (i * 0x140 + (j + 1) % 20 * 16));
            qcopy(s.q[2], D_L00_001E5EE0 + (i * 0x140 + j * 16));
            qcopy(s.q[3], D_L00_001E5EE0 + (i * 0x140 + (j + 1) % 20 * 16));
            func_001F9A28(vc, s.q[0], s.q[1]);
            func_001F9A28(vd, s.q[2], s.q[1]);
            func_001F9AD8(ve, vd, vc);
            func_001F9A28(vf, s.q[1], D_L00_00166DC0);
            dot = func_001F9AB0(ve, vf);
            if (0.0f <= dot) {
                b += 0.05f;
            } else {
                b2 = b + 0.05f;
                am = a - 0.3f;
                v = D_L00_001E6008[i];
                y = D_L00_001E6008[i - 1];
                o = s.col;
                for (k = 1; k >= 0; k--) {
                    if (v < 0) v = 0;
                    if (v > 0x40) v = 0x40;
                    v = (v << 24) | 0x404040;
                    o[0] = v;
                    o[1] = v;
                    v = (short)y;
                    o += 2;
                }
                if (i == 1) {
                    s.col[3] = 0x20404040;
                    s.col[2] = 0x20404040;
                } else if (i == 9) {
                    s.col[1] = 0x404040;
                    s.col[0] = 0x404040;
                }
                s.x0 = a;
                s.y0 = b;
                s.x1 = a;
                s.y1 = b2;
                s.x2 = am;
                s.y2 = b;
                s.x3 = am;
                s.y3 = b2;
                b = b2;
                func_L00_001FD228(&s, 0, 0);
            }
        }
        a = a2;
    }
    func_00233980(0x47, 0x5360B);
}
