/* Carried from the PAL decompilation's src/overlays/shared/hud_00263490.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern int func_001F9688(int);
extern char D_0013E533[];
extern int D_L05_0015FAD4 MACRO_ADDR;
extern int D_L05_0015FAEC MACRO_ADDR;
extern int D_L05_0015FAD8 MACRO_ADDR;
extern int D_L05_0015FAE0 MACRO_ADDR;
extern int D_L05_0015FB00 MACRO_ADDR;
extern int D_L05_0015FAF0 MACRO_ADDR;
extern short D_L05_0015F87C;
extern int func_00116248_002670A0(char *, const char *, ...) __asm__("func_00116248");
extern float func_001FA6C0(int);
extern int func_001FA6E0(int, int, float);
extern int func_001F6FD8_x(int, int, u64, int, int) __asm__("func_001F6C20");
extern char *func_001FE540_002670A0(int) __asm__("func_001FDD10");
extern void func_00201200(int, int, int, int);
extern int func_L00_00235A70(void *rec, int *x, int *y);
extern int func_001FF960(int, int);
extern void func_L00_0023B6C0(int, int, int, int, int, int);
extern void func_001FFC30(int, int, int, int, int, int);
extern float func_001F9DE0(float);
extern char D_L05_0015F860[];
extern char D_L05_0015F8D0[];
extern char D_L05_0015F8D8[];
extern char D_L05_0015F8E0[];
extern char D_L05_00216C50[];
extern int D_L05_0017E918[];
extern int D_L05_0015FAE8 MACRO_ADDR;
extern int D_L05_0015FAE4 MACRO_ADDR;
extern int D_L05_0015F5CC MACRO_ADDR;
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



/* HUD element update: draws the level 5 challenge readouts and the progress bar; returns the element's width. */
int FUN_L05_00262f50(char *a) {
    unsigned char *u = (unsigned char *)(a + 0x70);
    char buf[0x80];
    int out[4];
    int xa, xb;
    float f20, f21, f0, f12;
    char *E;
    int r, sel, cnt, k, idx, h, acc, id, id2, i, s16, r3, t;

    f21 = (float)u[0] / 10.0f;
    if (f21 > 1.0f) {
        f21 = 1.0f;
    }
    E = D_0013E533 + 0xE1D;
    if (*(int *)(E + 0x208C) != 0x16) {
        return *(int *)(a + 0x58);
    }
    if (u[2] != 0) {
        func_00116248_002670A0(buf, D_L05_0015F860, D_L05_0015FAD4 * 0x168);
        f20 = func_001FA6C0(u[1]);
        r = func_001F9688(*(int *)&D_L05_0015F87C);
        f12 = f20 / (float)r;
        if (u[2] == 1 || u[2] == 3) {
            acc = func_001FA6E0(*(int *)&D_L05_0015F894, *(int *)&D_L05_0015F898, f12);
        } else {
            acc = *(int *)&D_L05_0015F8A0;
        }
        h = func_001FA898_r(f21 * 128.0f) << 24;
        acc += h;
        if (u[2] == 1 || u[2] == 2) {
            func_001F6FD8_x(*(int *)&D_L05_0015F8B4 + 1, *(int *)&D_L05_0015F8B8 + 1, h, (int)buf, -1);
            func_001F6FD8_x(*(int *)&D_L05_0015F8B4, *(int *)&D_L05_0015F8B8, acc, (int)buf, -1);
            func_00116248_002670A0(buf, D_L05_0015F8D0, D_L05_0015FAD8,
                                   func_001FE540_002670A0(D_L05_0015FAD8 == 1 ? 0x517C : 0x517D));
            func_001F6FD8_x(*(int *)&D_L05_0015F8BC + 1, *(int *)&D_L05_0015F8C0 + 1, h, (int)buf, -1);
            func_001F6FD8_x(*(int *)&D_L05_0015F8BC, *(int *)&D_L05_0015F8C0, acc, (int)buf, -1);
        } else if (u[2] == 3) {
            sel = -1;
            if (D_L05_0015FAD8 == 3) {
                sel = 0;
                if (D_L05_0015FAEC != 0) {
                    sel = 2;
                    if (D_L05_0015FAE8 == 0) {
                        sel = 1;
                    } else if (D_L05_0015FAE4 != 0) {
                        sel = 3;
                    }
                }
            } else if (D_L05_0015FAD8 == 4) {
                sel = 4;
            }
            if (sel >= 0) {
                if (D_L05_0015FAD4 >= 4) {
                    k = 2;
                } else {
                    k = D_L05_0015FAD4 >= 2;
                }
                idx = sel + k * 5;
                D_L05_0015FAF0 = idx;
                D_L05_0015FB00 = D_L05_0017E918[idx];
                cnt = 1;
                for (i = 1; i < 4; i++) {
                    if ((&D_L05_0015FAF0)[i] == D_L05_0015FAF0) {
                        cnt++;
                    }
                }
                D_L05_0015FB00 = D_L05_0015FB00 / cnt;
                func_00116248_002670A0(buf, D_L05_0015F860, D_L05_0015FB00);
                func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                func_00116248_002670A0(buf, D_L05_0015F8D8, func_001FE540_002670A0(idx + 0x5092));
                func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8, acc, (int)buf);
            } else if (D_L05_0015FAD8 > 0) {
                int n = 0;
                int m;
                for (m = 0; m < 4 && n < D_L05_0015FAD8; m++) {
                    if ((&D_L05_0015FAE0)[m] != 0) {
                        out[n] = m;
                        n++;
                    }
                }
                if (D_L05_0015FAD8 == 1) {
                    D_L05_0015FB00 = D_L05_0015FAD4 * 10;
                    func_00116248_002670A0(buf, D_L05_0015F860, D_L05_0015FB00);
                    func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                    func_00116248_002670A0(buf, D_L05_0015F8E0, func_001FE540_002670A0(out[0] + 0x5183), D_L05_0015FAD4,
                                           func_001FE540_002670A0(D_L05_0015FAD4 == 0x168 ? 0x50A7 : 0x50A8));
                    func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8, acc, (int)buf);
                } else {
                    D_L05_0015FB00 = D_L05_0015FAD4 * 20;
                    func_00116248_002670A0(buf, D_L05_0015F860, D_L05_0015FB00);
                    func_00201200(*(int *)&D_L05_0015F8C4, *(int *)&D_L05_0015F8C8 - 0x18, acc, (int)buf);
                    func_00116248_002670A0(buf, D_L05_00216C50, func_001FE540_002670A0(out[0] + 0x5183),
                                           func_001FE540_002670A0(out[1] + 0x5183), D_L05_0015FAD4);
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
    E = D_0013E533 + 0xE1D;
    t = (unsigned char)E[0x8AE];
    if (func_001F9688(0x78) < t) {
        r3 = func_001F9688(0x3C);
        f0 = func_001FA6C0(D_L05_0015F5CC % r3) / (float)r3;
        f0 = func_001F9DE0(f0 * 6.283180236816406f - 3.141590118408203f);
        r3 = func_001FA6E0(*(int *)&D_L05_0015F8AC, *(int *)&D_L05_0015F8B0, f0 * 0.5f + 0.5f);
        func_00116248_002670A0(buf, func_001FE540_002670A0(0x531F));
        func_00201200(*(int *)&D_L05_0015F8A4 + 1, *(int *)&D_L05_0015F8A8 + 1, 0x80000000, (int)buf);
        func_00201200(*(int *)&D_L05_0015F8A4, *(int *)&D_L05_0015F8A8, r3, (int)buf);
    }
    return *(int *)(a + 0x58);
}
