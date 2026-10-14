/* Carried from the PAL decompilation's nonmatching/l07_umbris/func_L07_00313D28.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L07_00312948 -- src/overlays/l07_umbris/vendor_00313D28.c
 * Best so far: SIZE ours 1120 / retail 1148, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 4 runs (best p3.c, 1120 bytes vs retail 1148). Umbris flare-strip effect: a prologue that packs 
 *   tags (sd of 0xFF9000000260, 0xA000000029 and the D_0015EE88>>13 word), a func_L00_00200228 pair, then a do/whi
 *   angles (pi/2 steps, f27 += ...) wrapping a do/while over f21 that copies two vec4s and calls func_00214DB0/fun
 *   Differences left: the 64-bit constants and the sd stores are scheduled into the prologue in retail (ours build
 *   and the tail of the setup (f20 = r2 + 1, the 0x54/0x5C/0x6C/0x64 stores, sub.s f21 = fe - fd) comes out in ano
 *   The float literal -2.7443f stands in for 0xC02FEDE0; its exact bits are unverified.
 */
extern void func_00233980(int, long);
extern int func_001F44B8(int);
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern void func_L00_0024F7C8(void *, int, void *);
extern float func_L00_00200228(void *, float);
extern float func_L00_0025E310(float);
extern void func_00214DB0(void *, float, float, float);
extern float func_001FA580(float, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, int);
extern int D_0015EF88_m __asm__("D_0015EE88") MACRO_ADDR;
extern int D_L07_0015F5CC MACRO_ADDR;

// Umbris effect: builds GS packets for a flare strip from the moby's data and draws its segments
void FUN_L07_00312948(char *moby, int arg2, float fa, float fb, float fc, float fd, float fe) {
    char *data = *(char **)(moby + 0x78);
    int c = *(unsigned char *)(data + 0x1C6);
    int col = (c << 24) | (c << 16) | (c << 8) | c;
    long w1 = (long)(D_0015EF88_m >> 13) | 0x1000000L | (0x8000L << 17);
    int c17, n, a4, i;
    int ints[4];
    long q[4];
    long q78;
    float vA0[4], vB0[4], v90[4], v20[4], v30[4], v0[4], v10[4], s50[8];
    float r1, r2, f0, f1, f12, f13, f20, f21, f22, f23, f24, f25, f27, f28, f29, f30, f31;

    func_00233980(0x4E, w1);
    q78 = func_001F44B8(0x3C);
    q[1] = q78;
    q[2] = 0xFF9000000260L;
    q[3] = 0xA000000029L;
    n = func_001FA6D0(func_001FA6C0(0) * fd);
    c17 = col | 0x404040;
    a4 = D_L07_0015F5CC + arg2;
    n = (n << 24) | c17;
    ints[0] = n;
    ints[3] = n;
    ints[2] = n;
    ints[1] = n;
    q[0] = 0;
    f30 = func_001FA6C0(a4) * fa;
    func_L00_0024F7C8(moby, 9, v90);
    qcopy(vA0, v90);
    f1 = fb - 0.8f;
    f0 = vA0[2] - f1;
    vA0[2] = f0;
    *(int *)&vB0[3] = c17;
    f12 = f30 * -0.01f;
    r1 = func_L00_00200228(vB0, f12);
    f12 = f30 * -0.02f;
    f1 = r1 + 1.0f;
    s50[0] = r1;
    s50[4] = r1;
    s50[6] = f1;
    s50[2] = f1;
    r2 = func_L00_00200228(vB0, f12);
    f20 = r2 + 1.0f;
    s50[1] = r2;
    s50[3] = r2;
    s50[5] = f20;
    s50[7] = f20;
    f21 = fe - fd;
    f31 = f21 * 0.0625f;
    f28 = fc * 0.0625f;
    f20 = f20;
    f27 = -2.7443f;
    f22 = fd;

    do {
        qcopy(v20, vA0);
        qcopy(v30, vA0);
        f29 = f27 + 1.5707963f;
        f22 = fd;
        n = func_001FA6D0(func_001FA6C0(0) * f22);
        f0 = fc;
        f1 = fc - 1.5707963f;
        f0 = -1.5707963f;
        n = (n << 24) | c17;
        ints[2] = n;
        ints[3] = n;
        f21 = f28 + f0;
        if (f21 <= f1) {
            f24 = 0.6f;
            f23 = 0.375f;
            f25 = f1;
            do {
                qcopy(v0, v20);
                qcopy(v10, v30);
                f22 = f22 + f31;
                ints[0] = ints[2];
                ints[1] = ints[3];
                n = func_001FA6D0(func_001FA6C0(0) * f22);
                f12 = 0.02f;
                n = (n << 24) | c17;
                f12 = f30 * f12;
                ints[2] = n;
                ints[3] = n;
                f20 = func_L00_0025E310(f27 + f12);
                func_00214DB0(v20, fb, f20, f21);
                f13 = func_001FA580(f20, 0.7853982f);
                func_00214DB0(v30, fb, f13, f21);
                f0 = v20[2];
                f1 = v30[2];
                if (0.0f < f21) {
                    f0 = f0 + f24;
                    f1 = f1 + f24;
                } else {
                    f0 = f0 - f23;
                    f1 = f1 - f23;
                }
                v20[2] = f0;
                v30[2] = f1;
                func_001F9A10(v20, v20, v90);
                f21 = f21 + f28;
                func_001F9A10(v30, v30, v90);
                func_L00_001FD228(v0, 0, 1);
            } while (f21 <= f25);
        }
        f27 = f29;
    } while (f27 < 3.14159f);

    func_00233980(0x4E, (long)(D_0015EF88_m >> 13) | 0x1000000L);
    (void)q78;
    (void)i;
    (void)fe;
}
