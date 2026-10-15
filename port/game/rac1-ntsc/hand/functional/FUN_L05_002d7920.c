/* Carried from the PAL decompilation's nonmatching/l05_rilgar/func_L05_002D8B68.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L05_002d7920 -- src/overlays/l05_rilgar/vendor_002D28D0.c
 * Best so far: SIZE ours 940 / retail 960, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Builds 16 sprite records (0x90 each, 4 float pairs and GS-style u64 words) from D_L05_001CDD70 and the func_00
 *   Best so far p4.c: 940 of 960 bytes, first-loop instruction order and the moby spill (retail keeps moby in 0x93
 */
extern int func_001F44B8(int);
extern float func_001FA6C0(int);
extern int func_001FA6E0(int, int, float);
extern void *func_001153FC_c(void *, int, unsigned int) __asm__("func_001153FC");
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_002002E0(void *, float);
extern void func_L00_001FD228(void *, void *, int);
extern unsigned char D_0015EDB0[];
extern int D_L05_0015F5CC MACRO_ADDR;
extern short *D_L05_001ABCC0[];
extern char *D_L05_00160098_0D3F0 __asm__("D_L05_0015FFD8") MACRO_ADDR;
extern float D_L05_001CDD70[];
extern short D_L05_001613E0;
extern short D_L05_001613F0;
extern short D_L05_001613F4;
extern short D_L05_001613F8;
extern short D_L05_001613FC;
extern short D_L05_00161400;
extern short D_L05_00161404;
extern short D_L05_00161408;
extern short D_L05_0016140C;

typedef int u128_D8B68_c2d7920 __attribute__((mode(TI)));

// Builds 16 sprite records from the table at D_L05_001CDD70, then walks the moby's draw list and calls the helpers for each matching entry
void FUN_L05_002d7920(unsigned char *moby) {
    char recs[0x900];
    float buf[8];
    float vec[4];
    int i, j, k, m, q, r, n, jj0;
    short *p;
    unsigned short id;
    char *e;
    char *d;
    char *rec;
    char *rp;
    float *s;
    float *vp;
    char *bp;
    float *pa;
    float *pb;
    int *pi;
    float f0v, f20, t;

    if (*(int *)&D_L05_001613F0 != 0) {
        return;
    }
    if (D_0015EDB0[2] == 0) {
        *(float *)&D_L05_001613E0 = 5.0f;
    } else {
        *(float *)&D_L05_001613E0 = 16.0f;
    }

    for (i = 0; i < 16; i++) {
        func_001F44B8(0x13);
        rec = recs + i * 0x90;
        *(s64 *)(rec + 0x78) = (s64)(int)(recs + 0x50);
        *(s64 *)(rec + 0x80) = 0xFF9000000260LL;
        *(s64 *)(rec + 0x88) = (s64)*(int *)&D_L05_001613F8
            | ((s64)*(int *)&D_L05_001613FC << 2)
            | ((s64)*(int *)&D_L05_00161400 << 4)
            | ((s64)*(int *)&D_L05_00161404 << 6)
            | ((s64)0x8000 << 24);
        *(s64 *)(rec + 0x70) = 0;
        f0v = func_001FA6C0((D_L05_0015F5CC + 3) & 3);
        f20 = f0v / (*(float *)&D_L05_001613E0 * 4.0f);
        s = D_L05_001CDD70;
        pa = (float *)(rec + 0x50);
        pb = (float *)(rec + 0x54);
        pi = (int *)(rec + 0x40);
        for (j = 0; j < 4; j++) {
            *pa = s[0];
            *pb = s[1];
            f0v = func_001FA6C0(i - ((j >> 1) - 1));
            t = f0v / (*(float *)&D_L05_001613E0 + f20);
            if (t > 1.0f) {
                t = 1.0f;
            } else if (t < 0.0f) {
                t = 0.0f;
            }
            *pi = func_001FA6E0(*(int *)&D_L05_00161408, *(int *)&D_L05_0016140C, t);
            s += 2;
            pa += 2;
            pb += 2;
            pi += 1;
        }
    }

    bp = (char *)buf;
    vp = vec;
    func_001153FC_c(bp, 0, 0x20);
    ((float *)bp)[2] = 0.15f;
    ((float *)bp)[6] = -0.15f;
    p = D_L05_001ABCC0[moby[0x21]];

    for (;;) {
        id = *(unsigned short *)p;
        e = (char *)D_L05_00160098_0D3F0 + ((id & 0x7FFF) << 8);
        if (*(short *)(e + 0xA6) == 0x4F && (unsigned char)e[0x20] == 1) {
            d = *(char **)(e + 0x78);
            *(u128_D8B68_c2d7920 *)vp = *(u128_D8B68_c2d7920 *)(e + 0x10);
            vp[3] = 20.0f;
            r = func_L00_002002E0(vp, 512.0f);
            if (r != -1) {
                n = *(short *)(d + 0x320);
                for (k = 0; k < n; k++) {
                    rec = recs + k * 0x90;
                    jj0 = (*(short *)(d + 0x322) - k + 15) & 15;
                    for (m = 0; m < 3; m++) {
                        if (*(int *)&D_L05_001613F4 == 0) {
                            rp = recs + k * 0x90;
                            for (q = 0; q < 4; q++) {
                                func_001F9A10(rp, d + m * 256 + 0x20 + ((jj0 + (q >> 1)) & 15) * 16, bp + ((q & 1) << 4));
                                rp += 0x10;
                            }
                            func_L00_001FD228(rec, 0, 0);
                        }
                    }
                }
            }
        }
        if (*p < 0) {
            break;
        }
        p++;
    }
}
