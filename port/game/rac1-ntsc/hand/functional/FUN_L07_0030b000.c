/* Carried from the PAL decompilation's nonmatching/functional/func_L07_0030C3E0.c (games/rac1) through the version
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
/* functional: high. Built from the near miss with the retail behaviour restored (state byte +0x20 always set to 1 after the init branch, path count re-read each loop pass); remaining differences are register, stack-slot and scheduling choices of the compiler. */
/* Level 07 (Umbris) update for moby class 871: on its first frame it measures the segments of its path,
 * finds its point on the path and places the chain of mobys it leads there; afterwards it moves each of
 * them along the path; while its +0x94 flag is set it takes damage
 * from hits (health in +0x20 of its data) and explodes when that runs out. */
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern void func_L01_0026E0E0(int list, int state);
extern void func_001F99F8(float *);
extern void func_L00_0025F4A8_alt(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025E450");
extern void func_0020C828(void *);
extern int func_001F9688(int);
extern void func_L00_0025D458(void *, short *);
extern void func_L00_0025D538(void *, void *);
extern int func_L00_002591D0(int *, int, int, int);
extern int func_001E93B0();
extern int func_L01_0028B510(void *, void *, float);
extern int func_L00_002592B8(int *, int, int, int);
extern int func_L00_0025D808(void *, void *, int *, float *, float, int);
extern void func_L00_00259FE8(unsigned char *, float);
extern void func_001F9A28(void *, void *, void *);
extern int func_001F9740(int *arg0);
extern float func_001F9B48(void *, void *);
extern char *D_L07_001B0530[];
extern float D_0015ED6C MACRO_ADDR;
extern char D_L07_00211550[];
extern char D_L07_00211580[];
extern char D_L07_002115D0[];

// Level 07 update for moby class 871: samples points along its path and steers it.
void FUN_L07_0030b000(char *m) {
    char *p;
    char *t;
    char *r;
    float *f;
    float vec[4];
    int st30;
    int st34;
    int st38;
    int st3C;
    int st40;
    float st44;
    int n;
    int i;
    int k;
    int cnt;
    int state;
    char *e;
    float fv;

    if (m == 0) return;
    p = *(char **)(m + 0x78);
    if (p == 0) return;
    if (*(unsigned char *)(p + 0x94) != 0) {
        f = (float *)(p + 0x20);
        r = func_L00_0025A420(m, 0x230000, 0);
        n = func_L00_0025A478(m, r, f, 0, &st30, 0, 0, 4);
        switch (n) {
        case 0:
            break;
        case 1:
        case 2:
            *f = 0;
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            break;
        }
        if (st30 >= 2) {
            if (*f <= *(float *)(r + 0x2C)) {
                *f = 0;
                func_L01_0026E0E0(*(int *)(p + 0x74), 3);
                func_001F99F8(vec);
                func_L00_0025F4A8_alt(m, vec, (void *)0, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
                func_0020C828(m);
                return;
            }
            *f = *f - *(float *)(r + 0x2C);
            *(unsigned char *)(p + 0x67) = 0xFA;
            *(short *)(p + 0x26) = func_001F9688(0x3C);
            func_L00_0025D458(m, (short *)(p + 0x60));
        }
        *(unsigned char *)(m + 0xA4) = 0xFF;
    }

    func_L00_0025D538(m, p + 0x60);
    n = *(int *)(p + 0x70);
    if (n == -1) return;
    t = D_L07_001B0530[n];
    state = *(unsigned char *)(m + 0x20);

    switch (state) {
    case 0: {
        st34 = 0;
        st3C = 0;
        qcopy(vec, t + 0x10);
        cnt = *(int *)t;
        i = 0;
        if (cnt > 0) {
            do {
                *(float *)(t + i * 16 + 0x1C) = func_001F9B48(t + i * 16 + 0x10, t + ((i + 1) % cnt) * 16 + 0x10);
                i = i + 1;
                cnt = *(int *)t;
            } while (i < cnt);
        }
        if (func_L00_002591D0(&st34, *(int *)(p + 0x74), 1, 0)) {
            func_001E93B0(D_L07_00211550, *(int *)(p + 0x74));
            return;
        }
        n = func_L01_0028B510(m + 0x10, t, 0.0f);
        *(int *)(p + 0x90) = n;
        st38 = n;
        k = 0;
        if (*(int *)(p + 0x78) > 0) {
            do {
                e = *(char **)(st34 + 0x78);
                *(int *)(e + 0xA0) = (int)m;
                qcopy((char *)st34 + 0x10, vec);
                qcopy(e + 0x80, vec);
                qcopy(e + 0x70, vec);
                *(int *)(e + 0x90) = st38;
                *(int *)(e + 0x94) = st38 + 1;
                if (func_L00_002592B8(&st34, st34, 1, 0) != 0) {
                    cnt = *(int *)(p + 0x78);
                    if (k < cnt - 1) {
                        func_001E93B0(D_L07_00211580, k + 1, *(int *)(p + 0x74));
                    } else if (k == cnt - 1) {
                        func_001E93B0(D_L07_002115D0, *(int *)(p + 0x74), cnt);
                    }
                    *(int *)(p + 0x78) = k + 1;
                    break;
                } else {
                    func_L00_0025D808(t, vec, &st38, (float *)&st3C, *(float *)(p + 0x80), 1);
                }
                k = k + 1;
            } while (k < *(int *)(p + 0x78));
        }
        if (*(unsigned char *)(p + 0x94) == 0) {
            *(int *)(m + 0x94) = 0;
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
        }
        if (*(unsigned char *)(p + 0x95) == 0) {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 1;
        }
        *(unsigned char *)(m + 0x20) = 1;
        break;
    }
    case 1: {
        st40 = 0;
        func_L00_002591D0(&st40, *(int *)(p + 0x74), 1, 0);
        if (*(int *)(p + 0x78) > 0) {
            k = 0;
            do {
                e = *(char **)(st40 + 0x78);
                fv = func_001F9B48(e + 0x80, t + *(int *)(e + 0x90) * 16 + 0x10);
                st44 = fv;
                func_L00_0025D808(t, e + 0x80, (int *)(e + 0x90), &st44, *(float *)(p + 0x7C) * D_0015ED6C, 1);
                *(int *)(e + 0x94) = *(int *)(e + 0x90) + 1;
                if (st40 == 0 || *(unsigned char *)(st40 + 0x20) == 0xFE || *(unsigned char *)(st40 + 0x20) == 0xFD) {
                    if (*(int *)(p + 0x84) != -1) {
                        func_001F9A28(e + 0x60, e + 0x80, (char *)st40 + 0x10);
                        qcopy((char *)st40 + 0x10, e + 0x80);
                        if (func_001F9740((int *)(e + 0x98))) {
                            if (*(int *)(e + 0x90) == *(int *)(p + 0x90)) {
                                func_L07_0030AF40((void *)st40);
                            }
                        }
                    }
                }
                func_L00_002592B8(&st40, st40, 1, 0);
                k = k + 1;
            } while (k < *(int *)(p + 0x78));
        }
        break;
    }
    }

    func_L00_00259FE8(m, 1.5f);
}
