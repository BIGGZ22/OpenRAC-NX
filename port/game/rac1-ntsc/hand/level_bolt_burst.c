/* Carried from port/game/rac1-pal/hand/level_bolt_burst.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL bolt burst, FUN_L00_00260a88 (0x8CC bytes), which the decompilation
 * still has as assembly: the bolts a broken crate or a dead creature throws out,
 * between `b` and `c` of them in value, as bolts worth 50, 20, 5 and 1, each thrown
 * from the moby on its own arc (func_L00_002A7438 makes one). Not a PS2 match: this
 * is the decompilation's own near miss for the function
 * (nonmatching/shared/FUN_L00_00260a88.c, 2284 bytes against retail's 2252), used
 * here as it is until the function is matched. */
extern void func_001F99F8(void *);
extern unsigned char *func_L00_0025C338(int);
extern int func_00213260(int);
extern int func_001F9688(int);
extern int func_001E93B0();
extern float func_001FA6C0(int);
extern int func_00120478(float);
extern float func_00213308(void);
extern float func_001F9DC8(float);
extern float func_002132A8(float, float);
extern float func_001F9DE0(float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_002595A0(float *, float *, float, float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_L00_00261B48(int, void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_002A7438(void *, void *, void *, int, int, int);
extern int D_L00_0015F598 MACRO_ADDR;
extern unsigned char D_L00_0015FC88[];
extern int D_0015EEA0 MACRO_ADDR;
extern int D_0015EE20 MACRO_ADDR;
extern int D_0015EE28 MACRO_ADDR;
extern int D_0015EE24[] MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern char *D_L00_001B04B0[];
extern char D_L00_001600A0[];
extern char D_L00_001600B0[];
extern char D_L00_001600C0[];

/* Per-moby effect update: builds a short vector set from the moby's
   0x10 vector and calls the helper chain for each queued unit. */
void FUN_L00_00260a88(char *a, int b, int c, int flags, int idx)
{
    float s0[4], s10[4], s20[4], s30[4], s40[4], s50[4], s60[4];
    float s70[2];
    int f84 = 0;
    int n, k, w, sum, m, rv;
    int c4, c5, c6, c8, c10, c11, c12, c13, c14, c15, c16, c17, c20, c22, c23, c30;
    unsigned char *r;
    char *p;
    float f0, f12, f13, f14, f20, f21;

    c22 = 0;
    c23 = 0;
    c30 = 0;
    func_001F99F8(s0);
    if (!(flags & 0x20)) {
        if (*(unsigned char *)(a + 0xB0) != 0xFF
            && (unsigned)(*(unsigned short *)(a + 0xA6) - 0x1F4) >= 0x29) {
            D_L00_0015F598 = (D_L00_0015FC88[*(unsigned char *)(a + 0xB0)] ^ 0xFF) == 0;
        }
    }
    if (D_L00_0015F598) {
        f84 = !(flags & 0x20);
    }
    if (flags == 0) {
        return;
    }
    if (b == 0 && c == 0) {
        return;
    }
    if (flags & 4) {
        r = func_L00_0025C338((int)a);
        if (r) {
            *(int __attribute__((mode(TI))) *)s0 = *(int __attribute__((mode(TI))) *)(r + 0x20);
        }
    }
    if (b <= 0) {
        b = 1;
    }
    n = func_00213260(c - b + 1) + b;
    if (D_0015EEA0 || D_0015EE20) {
        n <<= 1;
    }
    if (n < 0x1F4) {
        if (f84 && D_0015EE28) {
            m = (int)((float)func_001F9688(0x3C) * 60.0f) * D_0015EE24[2] / D_0015EE24[1];
            if (D_0015EEA0 || D_0015EE20) {
                m /= 2;
            }
            if (m < 0x96) {
                if (m < 0x5B) {
                    func_001E93B0(D_L00_001600A0);
                    n *= 5;
                } else {
                    b = (0xBE - m) / 20;
                    func_001E93B0(D_L00_001600B0, b);
                    n *= b;
                }
                if (D_0015EEA0) {
                    if (n > 0x3E8) n = 0x3E8;
                } else {
                    if (n > 0x1F4) n = 0x1F4;
                }
            } else if (m >= 0x12D) {
                if (m >= 0x160) {
                    func_001E93B0(D_L00_001600C0, 0x3FB99999A0000000LL);
                    n /= 10;
                } else {
                    b = 0x168 - m;
                    f20 = func_001FA6C0(b);
                    rv = func_00120478(f20 / 20.0f);
                    func_001E93B0(D_L00_001600C0, rv);
                    b = b / 80;
                    n *= b;
                }
                if (n <= 0) {
                    n = 1;
                }
            }
        }
    }
    k = n;
    if (n >= 0x12C) {
        do {
            k -= 50;
            c30++;
        } while (k >= 0x12C);
    }
    while (k >= 0xC8) {
        k -= 20;
        c22++;
    }
    if (k >= 0x1E) {
        do {
            k -= 25;
            c22++;
            c23++;
        } while (k >= 0x1E);
    }
    if (k >= 7) {
        do {
            k -= 5;
            c23++;
        } while (k >= 7);
    }
    sum = c30 + c22 + c23 + k;
    if (sum >= 4 && (flags & 8)) {
        c17 = 0;
        c14 = 0;
        c13 = 0;
        c16 = 3;
        c15 = 3;
        c10 = 0;
        c12 = 0;
        c20 = 0;
        c11 = 0x96;
        c5 = 0;
        if (n < 0x96) {
            c6 = 0x96;
            c4 = 0;
            do {
                if (c16) {
                    c6 -= 50;
                    c16--;
                    c4 += 20;
                    c13++;
                } else if (c13) {
                    c4 -= 20;
                    c13--;
                    c5 += 5;
                    c14++;
                } else if (c14) {
                    c5 -= 5;
                    c14--;
                    c17++;
                }
                c11 = c6 + c4 + c5 + c17;
                if (!(n < c11)) {
                    break;
                }
            } while (c11 >= 4);
        }
        c5 = 3;
        if (3 < n) {
            c6 = c12 * 20;
            c8 = c20 * 50;
            c4 = c10 * 5;
            do {
                if (c15) {
                    c15--;
                    c4 += 5;
                    c10++;
                } else if (c10) {
                    c4 -= 5;
                    c10--;
                    c6 += 20;
                    c12++;
                } else if (c12) {
                    c6 -= 20;
                    c12--;
                    c8 += 50;
                    c20++;
                }
                c5 = c8 + c6 + c4 + c15;
            } while (c5 < n && c5 < 0x96);
        }
        if ((c11 - n) < (n - c5)) {
            k = c17;
            c23 = c14;
            c22 = c13;
            c30 = c16;
        } else {
            k = c15;
            c23 = c10;
            c22 = c12;
            c30 = c20;
        }
    }
    while (k || c23 || c22 || c30) {
        *(int __attribute__((mode(TI))) *)s10 = *(int __attribute__((mode(TI))) *)(a + 0x10);
        s10[2] = s10[2] + 0.5f;
        f21 = func_00213308();
        f20 = func_001F9DC8(f21);
        f0 = func_002132A8(0.0f, 3.0f) * D_0015ED6C;
        f20 = f20 * f0;
        s20[0] = f20;
        f20 = func_001F9DE0(f21);
        f0 = func_002132A8(0.0f, 3.0f) * D_0015ED6C;
        f20 = f20 * f0;
        s20[1] = f20;
        s20[2] = 0.0f;
        f0 = func_002132A8(3.7f, 6.0f) * D_0015ED6C;
        s20[2] = f0;
        s30[0] = s20[0];
        s30[1] = s20[1];
        s30[2] = 0.0f;
        func_L00_001FF500(s30, s30, 0.5f);
        func_001F9A10(s10, s10, s30);
        if (idx >= 0) {
            f12 = D_0015ED70 * 10.8f * -0.5f;
            f13 = s20[2] - f12;
            p = D_L00_001B04B0[idx];
            f14 = s10[2] - *(float *)(p + 0x18);
            f20 = 120.0f;
            rv = func_L00_002595A0(s70, s70 + 1, f12, f13, f14);
            if (1.0f <= (float)rv) {
                if (0.0f < s70[0]) {
                    if (s70[0] < f20) {
                        f20 = s70[0];
                    }
                }
            }
            func_001F9A68(s40, s20, f20);
            func_001F9A10(s40, s40, a + 0x10);
            rv = func_L00_00261B48(idx, a + 0x10, s40, s50);
            func_001F9A28(s40, s50, s10);
            s40[2] = 0.0f;
            func_L00_001FF500(s60, s40, 0.25f);
            func_001F9A28(s40, s40, s60);
            func_001F9A68(s40, s40, 1.0f / f20);
            s20[0] = s40[0];
            s20[1] = s40[1];
        }
        func_001F9A10(s20, s20, s0);
        if (c30) {
            c30--;
            func_L00_002A7438(a, s10, s20, flags, 50, f84);
        } else if (c22) {
            c22--;
            func_L00_002A7438(a, s10, s20, flags, 20, f84);
        } else if (c23) {
            c23--;
            func_L00_002A7438(a, s10, s20, flags, 5, f84);
        } else {
            k--;
            func_L00_002A7438(a, s10, s20, flags, 1, f84);
        }
    }
}
