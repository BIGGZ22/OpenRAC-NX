/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00238C30.c (games/rac1/pal) through the version
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
typedef struct {
    char pad_00[0x38];
    unsigned short x38;
    char pad_3A[0x4C - 0x3A];
} SrcElem_c2382a0;
typedef struct {
    char pad0[0x8];
    int unk08;
    int *unk0C;
    char pad10[0x38];
    short unk48;
    short unk4A;
    char pad4C[0x4];
    int unk50;
    int unk54;
    int w;
    int h;
    int flags;
    char pad64[0x8];
    int unk6C;
    unsigned char cnt[4];
    int unk74;
    int unk78;
    int unk7C;
    void *unk80;
} HudElem_c2382a0;
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern short D_L00_0015F718;
extern short D_L00_0015F71C;
extern short D_L00_0015F71C;
extern short D_L00_0015F72C;
extern int D_L00_0015FA90 MACRO_ADDR;
extern void *D_L00_0015FAC0 MACRO_ADDR;
extern short D_L00_0015F72C;
extern int D_0014171B_i[] __asm__("D_0014161B");
extern int D_0014171B_i[] __asm__("D_0014161B");
extern SrcElem_c2382a0 D_L00_00179AC0[];

/* NON_MATCHING FUN_L00_002382a0 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 2212 / retail 2164, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   QuickSelectDraw (hq9/s13): clamps two cnt-based ratios, computes a ring angle and calls func_001FF960/func_002
 *   Still differs: i lives in $s5 (retail $s4), so the rec->unk74 == i compare and the loop registers are swapped;
 */
extern int func_001FF960(int, int);
extern void func_001FFC30(int, int, int, int, int, int);
extern void func_L00_0023B120(char *, int, int, int, int, int);
extern int func_00116248(char *, const char *, ...);
extern int func_001FA6E0(int, int, float);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F6AF0(int, int, long, void *, int);
extern void *func_001FE540_id(int) __asm__("func_001FDD10");
extern int func_001166CC(char *);
extern unsigned char D_0013E05A[];
extern char D_L00_001C43B0_c[] __asm__("D_L00_001C40B0");
extern short D_L00_0015F7B0;
extern short D_L00_0015F78C;
extern short D_L00_0015F7C4;
extern short D_L00_0015F7C0;
extern short D_L00_0015F720;
extern short D_L00_0015F724;
extern short D_L00_0015F790;
extern short D_L00_0015F794;
extern short D_L00_0015F798;
extern short D_L00_0015F79C;
extern short D_L00_0015F7A0;
extern short D_L00_0015F7A4;
extern short D_L00_0015F7A8;
extern short D_L00_0015F7AC;
extern short D_L00_0015F7B4;
extern short D_L00_0015F7B8;
extern short D_L00_0015F7BC;
extern int D_L00_0015F5CC MACRO_ADDR;
extern short D_L00_0015F780;
extern char D_L00_0015F7C8[];
extern char D_0013D407[];

/* Quick-select HUD slot 3 draw callback: lays out the eight quick-select entries around rec and returns rec->w. */
int FUN_L00_002382a0(HudElem_c2382a0 *rec) {
    char buf[0x50];
    float f20, f21, f22, f23, ang, t1, t2;
    int a, b, sx, a9, hw, hh, n, i, o1, o2, o3, lim, xx, yy, k, e2, ii, s, F, F2, I, kk, s3, v, g, t3;
    char *p, *wr;
    unsigned char *G, *s0;
    unsigned char *c;
    unsigned char byte;
    int m;

    c = rec->cnt;
    f20 = func_001FA6C0(rec->cnt[0]);
    f22 = f20 / func_001FA6C0(*(int *)&D_L00_0015F780);
    if (1.0f < f22) {
        f22 = 1.0f;
    } else if (f22 < 0.0f) {
        f22 = 0.0f;
    }
    f20 = func_001FA6C0(c[1]);
    f23 = f20 / func_001FA6C0(*(int *)&D_L00_0015F780);
    if (1.0f < f23) {
        f23 = 1.0f;
    } else if (f23 < 0.0f) {
        f23 = 0.0f;
    }

    i = 0;
    m = D_L00_0015F5CC % *(int *)&D_L00_0015F7B0;
    f20 = func_001FA6C0(m);
    f20 = f20 / func_001FA6C0(*(int *)&D_L00_0015F7B0);
    ang = f20 * 6.28318023681640625f - 3.141590118408203125f;
    f20 = func_001F9DE0(ang);
    f21 = f22 * 128.0f;
    sx = func_001FA6D0(f21);
    a9 = func_001FA6D0(f21 * (f20 * 0.125f + 0.875f));

    a = rec->unk50;
    b = rec->unk54;
    func_L00_00235A70(rec, &a, &b);

    hw = *(int *)&D_L00_0015F718 / 2;
    hh = *(int *)&D_L00_0015F71C / 2;
    func_001FFC30(func_001FF960(0xE934, 0), a, b, hw, hh, sx);
    func_001FFC30(func_001FF960(0xE934, 0), a + *(int *)&D_L00_0015F718, b + *(int *)&D_L00_0015F71C, -hw, -hh, sx);
    func_001FFC30(func_001FF960(0xE934, 0), a + *(int *)&D_L00_0015F718, b, -hw, hh, sx);
    func_001FFC30(func_001FF960(0xE934, 0), a, b + *(int *)&D_L00_0015F71C, hw, -hh, sx);

    n = D_L00_0015FA90;
    if (n > 0) {
        f21 = 3.14159265f;
        o1 = 0;
        o2 = 0;
        o3 = 0;
        for (; i < n; i++) {
            ang = ((float)i + (float)i) * f21 / (float)n - f21;
            f20 = func_001FA580(ang, 1.57079632679f);
            t1 = func_001F9DC8(f20);
            xx = a + *(int *)&D_L00_0015F720 + (int)(*(float *)&D_L00_0015F78C * t1 * *(float *)&D_L00_0015F7C4);
            t2 = func_001F9DE0(f20);
            yy = b + *(int *)&D_L00_0015F724 + (int)(*(float *)&D_L00_0015F78C * t2);

            if (rec->unk74 == i) {
                if (*(int *)(((char **)D_L00_0015FAC0)[*(int *)&D_L00_0015F72C] + o1)) {
                    func_001FFC30(func_001FF960(0xE99C, 0), xx - 0x13, yy - 0x13, 0x26, 0x26, a9);
                }
            }
            if (*(int *)(((char **)D_L00_0015FAC0)[*(int *)&D_L00_0015F72C] + o2)) {
                if (rec->unk74 == i) {
                    lim = sx;
                } else {
                    lim = func_001FA6D0((float)sx * *(float *)&D_L00_0015F7C0);
                }
                p = ((char **)D_L00_0015FAC0)[*(int *)&D_L00_0015F72C] + o3;
                v = *(int *)(p + 0x18);
                byte = D_0013E05A[0x4C6 + v];
                t3 = func_001FF960(*(int *)p, byte ? 4 : 0);
                func_L00_0023B120((char *)rec, t3, xx, yy, 1, lim);
            }
            n = D_L00_0015FA90;
            o1 += 0x1C;
            o2 += 0x1C;
            o3 += 0x1C;
        }
    }

    k = rec->unk74;
    if (k >= 0) {
        p = ((char **)D_L00_0015FAC0)[*(int *)&D_L00_0015F72C] + k * 0x1C;
        if (*(int *)p) {
            f20 = f22 * f23;
            e2 = func_001FA6E0(*(int *)&D_L00_0015F798, *(int *)&D_L00_0015F79C, f20);
            ii = *(int *)((char *)D_0014171B_i + 0x885 + k * 4);
            wr = D_L00_001C43B0_c + ii * 24;
            if (*(unsigned short *)(wr + 8)) {
                s = *(int *)(D_0013D407 + 0x21 + ii * 4);
                func_00116248(buf, D_L00_0015F7C8, s, *(unsigned short *)(wr + 0xE));
                if (s) {
                    F = func_001FA6E0(*(int *)&D_L00_0015F7A8, *(int *)&D_L00_0015F7AC, f20);
                } else {
                    F = func_001FA6E0(*(int *)&D_L00_0015F7A0, *(int *)&D_L00_0015F7A4, f20);
                }
                func_001F6AF0(a + *(int *)&D_L00_0015F720 + 1, b + *(int *)&D_L00_0015F724 + 13, e2, buf, -1);
                func_001F6AF0(a + *(int *)&D_L00_0015F720, b + *(int *)&D_L00_0015F724 + 12, F, buf, -1);
            }
            s3 = 0;
            F2 = func_001FA6E0(*(int *)&D_L00_0015F790, *(int *)&D_L00_0015F794, f20);
            p = ((char **)D_L00_0015FAC0)[*(int *)&D_L00_0015F72C] + k * 0x1C;
            v = *(int *)(p + 0x18);
            byte = D_0013E05A[0x4C6 + v];
            if (byte) {
                g = *(short *)((char *)&D_L00_00179AC0[v] + 0x48);
            } else {
                g = *(short *)((char *)&D_L00_00179AC0[v] + 0x46);
            }
            G = func_001FE540_id(g);
            if (G) {
                if (func_001166CC((char *)G)) {
                    I = func_001166CC((char *)G);
                    s0 = G;
                    if (I > 0) {
                        kk = 1;
                        s0 = G + 1;
                        if (G[1] == '-') {
                            s0 = G + 2;
                            s3 = 2;
                        } else {
                            while (kk < I) {
                                s0++;
                                if (*s0 == '-') {
                                    s3 = kk + 2;
                                    s0++;
                                    break;
                                }
                                kk++;
                            }
                        }
                    }
                    if (s3 == 0) {
                        kk = 1;
                        s0 = G + 1;
                        if (s3 < I) {
                            if (G[1] == ' ') {
                                s0 = G + 2;
                                s3 = 1;
                            } else {
                                while (kk < I) {
                                    s0++;
                                    if (*s0 == ' ') {
                                        s3 = kk + 1;
                                        s0++;
                                        break;
                                    }
                                    kk++;
                                }
                            }
                        }
                    }
                    ii = *(int *)((char *)D_0014171B_i + 0x885 + k * 4);
                    wr = D_L00_001C43B0_c + ii * 24;
                    yy = b + *(int *)&D_L00_0015F724 + (*(unsigned short *)(wr + 8) ? *(int *)&D_L00_0015F7B4 : (s3 ? *(int *)&D_L00_0015F7B8 : *(int *)&D_L00_0015F7BC));
                    if (s3 != 0) {
                        func_001F6AF0(a + *(int *)&D_L00_0015F720 + 1, yy + 1, e2, G, s3);
                        func_001F6AF0(a + *(int *)&D_L00_0015F720, yy, F2, G, s3);
                        yy += 0x10;
                        func_001F6AF0(a + *(int *)&D_L00_0015F720 + 1, yy + 1, e2, s0, -1);
                        func_001F6AF0(a + *(int *)&D_L00_0015F720, yy, F2, s0, -1);
                    } else {
                        func_001F6AF0(a + *(int *)&D_L00_0015F720 + 1, yy + 1, e2, G, -1);
                        func_001F6AF0(a + *(int *)&D_L00_0015F720, yy, F2, G, -1);
                    }
                }
            }
        }
    }
    return rec->w;
}
