/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D9438.c (matched C there) (games/rac1) through the version
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
extern char D_L00_00166DC0[] NOT_SDA;
void func_001F9A28(void *, void *, void *);
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
    float step;
    short *pv;
    int i, j, k, v;
    unsigned short y;
    s.t1 = func_001F44B8(0x15);
    s.t2 = 0x0000FF9000000260L;
    s.t3 = 0x8000000048L;
    s.t0 = 0;
    a = *(float *)&D_L00_00161B48;
    *(float *)&D_L00_00161B48 = a + 0.001f;
    if (*(float *)&D_L00_00161B48 > 7.0f) *(float *)&D_L00_00161B48 -= 7.0f;
    step = 0.3f;
    a += step;
    for (i = 1; i < 10; i++) {
        b = *(float *)&D_L00_00161B4C;
        *(float *)&D_L00_00161B4C = b + *(float *)&D_L00_00161B50;
        if (*(float *)&D_L00_00161B4C > 7.0f) *(float *)&D_L00_00161B4C -= 7.0f;
        a2 = a + step;
        for (j = 0; j < 20; j++) {
            qcopy(s.q[0], D_L00_001E6020 + (j * 16 + i * 0x140));
            qcopy(s.q[1], D_L00_001E6020 + ((j + 1) % 20 * 16 + i * 0x140));
            qcopy(s.q[2], D_L00_001E5EE0 + (j * 16 + i * 0x140));
            qcopy(s.q[3], D_L00_001E5EE0 + ((j + 1) % 20 * 16 + i * 0x140));
            func_001F9A28(vc, s.q[0], s.q[1]);
            func_001F9A28(vd, s.q[2], s.q[1]);
            func_001F9AD8(ve, vd, vc);
            func_001F9A28(vf, s.q[1], D_L00_00166DC0);
            dot = func_001F9AB0(ve, vf);
            if (0.0f <= dot) {
                b += 0.05f;
            } else {
                b2 = b + 0.05f;
                am = a - step;
                y = D_L00_001E6008[i - 1];
                pv = &D_L00_001E6008[i];
                v = *pv;
                for (k = 0; k < 2; k++) {
                    if (v < 0) v = 0;
                    if (v > 0x40) v = 0x40;
                    v = (v << 24) | 0x404040;
                    s.col[k * 2] = v;
                    s.col[k * 2 + 1] = v;
                    v = (short)y;
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
