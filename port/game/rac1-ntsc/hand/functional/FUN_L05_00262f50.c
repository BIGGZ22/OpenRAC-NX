/* Carried from the PAL decompilation's nonmatching/shared/func_L05_002638F8.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L05_002638F8 (near) was paired by place; read it against NTSC-U before trusting it. */
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
/* NON_MATCHING FUN_L05_00262f50 -- src/overlays/shared/hud_00263490.c
 * Best so far: SIZE ours 1668 / retail 1700, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   HUD element update (shared, levels 05 and 16), 1700 bytes: returns a->0x58 (int); when a[0x72] is set it draws
 *   Differences left: the prologue saves one more register in retail (sq $ra first, an extra $s7 holding a value f
 */
extern int func_00116248(char *str, const char *fmt, ...);
extern float func_001FA6C0(int);
extern int func_001F9688(int);
extern int func_001FA6E0(int, int, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern int func_001F6C20(int a, int b, int c, int d, int e);
extern void *func_001FDD10(int);
extern void func_00201200(int, int, int, int);
extern int func_L00_00235A70(char *rec, int *x, int *y);
extern int func_001FF960(int, int);
extern void func_L00_0023B6C0(int, int, int, int, int, int);
extern void func_001FFC30(int, int, int, int, int, int);
extern float func_001F9DE0(float);
extern char D_0013E533[];
extern char D_L05_0015F860[];
extern char D_L05_0015F8D0[];
extern char D_L05_0015F8D8[];
extern char D_L05_0015F8E0[];
extern char D_L05_00216C50[];
extern int D_L05_0017E918[];
extern int D_L05_0015FAF4[];
extern int D_L05_0015FAE0[];
extern int D_L05_0015FAD8;
extern int D_L05_0015FAEC;
extern int D_L05_0015FAE8;
extern int D_L05_0015FAE4;
extern int D_L05_0015FAF0;
extern int D_L05_0015FB00;
extern int D_L05_0015FAD4;
extern int D_L05_0015F5CC MACRO_ADDR;
extern short D_L05_0015F87C;
extern short D_L05_0015F8A0;
extern short D_L05_0015F894;
extern short D_L05_0015F898;
extern short D_L05_0015F8B4;
extern short D_L05_0015F8B8;
extern short D_L05_0015F8BC;
extern short D_L05_0015F8C0;
extern short D_L05_0015F8C4;
extern short D_L05_0015F8C8;
extern short D_L05_0015F8A4;
extern short D_L05_0015F8A8;
extern short D_L05_0015F8AC;
extern short D_L05_0015F8B0;

/* HUD element update: draws the numeric readouts for the level 5 counter display. */
int FUN_L05_00262f50(char *a) {
    char *p70 = a + 0x70;
    unsigned char *u = (unsigned char *)p70;
    char buf[0x40];
    int out[4];
    int xa, xb;
    float f20, f21, f0, f12;
    int r, sel, cnt, j, v, k, idx, h, acc, id, id2, i, fb90, fb8c, s16, r3, mod;
    char *E;

    f21 = (float)(unsigned char)p70[0] / 10.0f;
    if (f21 > 1.0f) {
        f21 = 1.0f;
    }
    E = D_0013E533 + 0xE1D;
    if (*(int *)(E + 0x208C) == 0x16) {
        if (u[2] != 0) {
            f20 = func_001FA6C0(u[1]);
            r = func_001F9688(*(int *)&D_L05_0015F87C);
            f0 = (float)r;
            f12 = f20 / f0;
            if (u[2] == 1 || u[2] == 3) {
                acc = func_001FA6E0(*(int *)&D_L05_0015F894, *(int *)&D_L05_0015F898, f12);
            } else {
                acc = *(int *)&D_L05_0015F8A0;
            }
            h = func_001FA898_r(f21 * 128.0f) << 24;
            acc += h;
            if ((unsigned int)(u[2] - 1) < 2) {
                func_001F6C20(*(int *)&D_L05_0015F8B4 + 1, *(int *)&D_L05_0015F8B8 + 1, h, (int)buf, -1);
                func_001F6C20(*(int *)&D_L05_0015F8B4, *(int *)&D_L05_0015F8B8, acc, (int)buf, -1);
                id = (int)func_001FDD10(D_L05_0015FAD8 == 0 ? 0x517D : 0x517C);
                func_00116248(buf, D_L05_0015F8D0, D_L05_0015FAD8, id);
                func_001F6C20(*(int *)&D_L05_0015F8BC + 1, *(int *)&D_L05_0015F8C0 + 1, h, (int)buf, -1);
                func_001F6C20(*(int *)&D_L05_0015F8BC, *(int *)&D_L05_0015F8C0, acc, (int)buf, -1);
            } else if (u[2] == 3) {
                fb90 = D_L05_0015FAD8;
                sel = -1;
                if (fb90 == 3) {
                    if (D_L05_0015FAEC == 0) {
                        sel = 0;
                    } else if (D_L05_0015FAE8 == 0) {
                        sel = 1;
                    } else {
                        sel = (D_L05_0015FAE4 != 0) ? fb90 : 2;
                    }
                } else if (fb90 == 4) {
                    sel = 4;
                }
                if (sel >= 0) {
                    fb8c = D_L05_0015FAD4;
                    k = (fb8c >= 4) ? 2 : (fb8c >= 2);
                    idx = sel + k * 5;
                    D_L05_0015FAF0 = idx;
                    D_L05_0015FB00 = D_L05_0017E918[idx];
                    cnt = 1;
                    for (i = 0; i < 3; i++) {
                        if (D_L05_0015FAF4[i] == idx) {
                            cnt++;
                        }
                    }
                    D_L05_0015FB00 = D_L05_0015FB00 / cnt;
                    func_00116248(buf, D_L05_0015F860, D_L05_0015FB00);
                    func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                    id = (int)func_001FDD10(idx + 0x5092);
                    func_00116248(buf, D_L05_0015F8D8, id);
                    func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8, acc, (int)buf);
                } else if (fb90 > 0) {
                    cnt = 0;
                    j = 0;
                    for (;;) {
                        v = D_L05_0015FAE0[j];
                        if (v == 0) {
                            j++;
                        } else {
                            out[cnt] = j;
                            cnt++;
                            j++;
                        }
                        if (!(j < 4)) {
                            break;
                        }
                        if (!(cnt < fb90)) {
                            break;
                        }
                    }
                    if (fb90 == 1) {
                        D_L05_0015FB00 = D_L05_0015FAD4 * 10;
                        func_00116248(buf, D_L05_0015F860, D_L05_0015FB00);
                        func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                        id = (int)func_001FDD10(out[0] + 0x5183);
                        id2 = (int)func_001FDD10(D_L05_0015FAD4 ^ 0x168 ? 0x50A8 : 0x50A7);
                        func_00116248(buf, D_L05_0015F8E0, id, D_L05_0015FAD4, id2);
                        func_001F6C20(*(int *)&D_L05_0015F8BC + 1, *(int *)&D_L05_0015F8C0 + 1, id, (int)buf, -1);
                        func_001F6C20(*(int *)&D_L05_0015F8BC, *(int *)&D_L05_0015F8C0, acc, (int)buf, -1);
                    } else {
                        D_L05_0015FB00 = D_L05_0015FAD4 * 20;
                        func_00116248(buf, D_L05_0015F860, D_L05_0015FB00);
                        func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                        id = (int)func_001FDD10(out[0] + 0x5183);
                        id2 = (int)func_001FDD10(out[1] + 0x5183);
                        func_00116248(buf, D_L05_00216C50, id, id2, D_L05_0015FAD4);
                        func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8, acc, (int)buf);
                    }
                }
            }
        }

        xa = *(int *)(a + 0x50);
        xb = *(int *)(a + 0x54);
        *(int *)(a + 0x58) = 0x100;
        *(int *)(a + 0x5C) = 0x40;
        func_L00_00235A70(a, &xa, &xb);
        s16 = (*(int *)(a + 0x74) * 0xDD) / *(int *)(a + 0x8) + 0x1B;
        id = func_001FF960(0x7558, 1);
        func_L00_0023B6C0(id, xa, xb, s16, 0x40, 0x80);
        id2 = func_001FF960(0x7558, 0);
        func_001FFC30(id2, xa, xb, 0x100, 0x40, 0x80);
        if (func_001F9688(0x78) < (unsigned char)E[0x8AE]) {
            r3 = func_001F9688(0x3C);
            mod = D_L05_0015F5CC % r3;
            f0 = func_001FA6C0(mod) / (float)r3;
            f0 = func_001F9DE0(f0 * 6.283180236816406f - 3.141590118408203f);
            r3 = func_001FA6E0(*(int *)&D_L05_0015F8AC, *(int *)&D_L05_0015F8B0, f0 * 0.5f + 0.5f);
            id = (int)func_001FDD10(0x531F);
            func_00116248(buf, (char *)id);
            func_00201200(*(int *)&D_L05_0015F8A4 + 1, *(int *)&D_L05_0015F8A8 + 1, 0x80000000, (int)buf);
            func_00201200(*(int *)&D_L05_0015F8A4, *(int *)&D_L05_0015F8A8, r3, (int)buf);
        }
    }
    return *(int *)(a + 0x58);
}
