/* NON_MATCHING func_L00_00261B00 -- src/overlays/shared/mobyutil_00261B00.c
 * Best so far: SIZE ours 2284 / retail 2252, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Per-moby effect update: sets up three 16-byte vectors from the moby's 0x10 vector, runs a two-loop count split
 *   Stopped at size: ours 2148 bytes vs retail 2252 (about 26 instructions short). The shortfall is in the unit-de
 *   Also not reproduced: retail's dead re-test of the unit sum at the start of the body (same sum as the check bef
 */
extern void func_001F9BC0(void *);
extern unsigned char *func_L00_0025D390(int);
extern int func_002140B0(int);
extern int func_001F9850(int);
extern int func_001E9730();
extern float func_001FA888(int);
extern int func_00120778(float);
extern float func_00214158(void);
extern float func_001F9F90(float);
extern float func_002140F8(float, float);
extern float func_001F9FA8(float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_L00_0025A5D8(float *, float *, float, float, float);
extern void func_001F9C30(void *, void *, float);
extern int func_L00_00262BC0(int, void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_002A86D8(void *, void *, void *, int, int, int);
extern int D_L00_0015F678 MACRO_ADDR;
extern unsigned char D_L00_0015FD48[];
extern int D_0015EFA0 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern int D_0015EF28 MACRO_ADDR;
extern int D_0015EF24[] MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern char *D_L00_001B0830[];
extern char D_L00_00160160[];
extern char D_L00_00160170[];
extern char D_L00_00160180[];

/* Per-moby effect update: builds a short vector set from the moby's
   0x10 vector and calls the helper chain for each queued unit. */
void func_L00_00261B00(char *a, int b, int c, int flags, int idx)
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
    func_001F9BC0(s0);
    if (!(flags & 0x20)) {
        if (*(unsigned char *)(a + 0xB0) != 0xFF
            && (unsigned)(*(unsigned short *)(a + 0xA6) - 0x1F4) >= 0x29) {
            D_L00_0015F678 = (D_L00_0015FD48[*(unsigned char *)(a + 0xB0)] ^ 0xFF) == 0;
        }
    }
    if (D_L00_0015F678) {
        f84 = !(flags & 0x20);
    }
    if (flags == 0) {
        return;
    }
    if (b == 0 && c == 0) {
        return;
    }
    if (flags & 4) {
        r = func_L00_0025D390((int)a);
        if (r) {
            *(int __attribute__((mode(TI))) *)s0 = *(int __attribute__((mode(TI))) *)(r + 0x20);
        }
    }
    if (b <= 0) {
        b = 1;
    }
    n = func_002140B0(c - b + 1) + b;
    if (D_0015EFA0 || D_0015EF20) {
        n <<= 1;
    }
    if (n < 0x1F4) {
        if (f84 && D_0015EF28) {
            m = (int)((float)func_001F9850(0x3C) * 60.0f) * D_0015EF24[2] / D_0015EF24[1];
            if (D_0015EFA0 || D_0015EF20) {
                m /= 2;
            }
            if (m < 0x96) {
                if (m < 0x5B) {
                    func_001E9730(D_L00_00160160);
                    n *= 5;
                } else {
                    b = (0xBE - m) / 20;
                    func_001E9730(D_L00_00160170, b);
                    n *= b;
                }
                if (D_0015EFA0) {
                    if (n > 0x3E8) n = 0x3E8;
                } else {
                    if (n > 0x1F4) n = 0x1F4;
                }
            } else if (m >= 0x12D) {
                if (m >= 0x160) {
                    func_001E9730(D_L00_00160180, 0x3FB99999A0000000LL);
                    n /= 10;
                } else {
                    b = 0x168 - m;
                    f20 = func_001FA888(b);
                    rv = func_00120778(f20 / 20.0f);
                    func_001E9730(D_L00_00160180, rv);
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
        f21 = func_00214158();
        f20 = func_001F9F90(f21);
        f0 = func_002140F8(0.0f, 3.0f) * D_0015EE6C;
        f20 = f20 * f0;
        s20[0] = f20;
        f20 = func_001F9FA8(f21);
        f0 = func_002140F8(0.0f, 3.0f) * D_0015EE6C;
        f20 = f20 * f0;
        s20[1] = f20;
        s20[2] = 0.0f;
        f0 = func_002140F8(3.7f, 6.0f) * D_0015EE6C;
        s20[2] = f0;
        s30[0] = s20[0];
        s30[1] = s20[1];
        s30[2] = 0.0f;
        func_L00_001FF4B0(s30, s30, 0.5f);
        func_001F9BD8(s10, s10, s30);
        if (idx >= 0) {
            f12 = D_0015EE70 * 10.8f * -0.5f;
            f13 = s20[2] - f12;
            p = D_L00_001B0830[idx];
            f14 = s10[2] - *(float *)(p + 0x18);
            f20 = 120.0f;
            rv = func_L00_0025A5D8(s70, s70 + 1, f12, f13, f14);
            if (1.0f <= (float)rv) {
                if (0.0f < s70[0]) {
                    if (s70[0] < f20) {
                        f20 = s70[0];
                    }
                }
            }
            func_001F9C30(s40, s20, f20);
            func_001F9BD8(s40, s40, a + 0x10);
            rv = func_L00_00262BC0(idx, a + 0x10, s40, s50);
            func_001F9BF0(s40, s50, s10);
            s40[2] = 0.0f;
            func_L00_001FF4B0(s60, s40, 0.25f);
            func_001F9BF0(s40, s40, s60);
            func_001F9C30(s40, s40, 1.0f / f20);
            s20[0] = s40[0];
            s20[1] = s40[1];
        }
        func_001F9BD8(s20, s20, s0);
        if (c30) {
            c30--;
            func_L00_002A86D8(a, s10, s20, flags, 50, f84);
        } else if (c22) {
            c22--;
            func_L00_002A86D8(a, s10, s20, flags, 20, f84);
        } else if (c23) {
            c23--;
            func_L00_002A86D8(a, s10, s20, flags, 5, f84);
        } else {
            k--;
            func_L00_002A86D8(a, s10, s20, flags, 1, f84);
        }
    }
}
