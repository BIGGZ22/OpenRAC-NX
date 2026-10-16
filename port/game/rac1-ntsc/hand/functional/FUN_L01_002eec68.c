/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B90A8.c (matched C there) (games/rac1) through the version
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
extern float func_001FA580(float, float);
extern char D_0013E533[];
extern char *D_L01_001B0930[];
extern float D_0015ED6C MACRO_ADDR;
extern float func_001F9B48(void *, void *);
extern float func_001F9B80(void *, void *);
extern float func_001F99C0(float);
extern int func_001F9688(int);
extern int func_L00_001EFFF0_t(void *, void *, int, void *, void *) __asm__("func_L00_001EFC70");
extern int func_001F9740(void *);
extern void func_001F99F8(void *);
extern void func_0020C828(void *);
extern f32 func_001F9DE0(f32);
extern void func_L01_002EF770(unsigned char *, float *);
extern int func_L00_00257470(void *, int, int);
extern void func_0022DA68(int, int, char *);
extern int func_001FA6D0(float);
typedef int u128_2F0040_c2eec68 __attribute__((mode(TI)));
typedef union { u128_2F0040_c2eec68 q; float f[4]; } V_2F0040_c2eec68;
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern char *func_L01_002EF350(void *);
extern char *func_L01_002EF478(char *, int);
extern void func_L00_0025AB48(void *, float *, void *, void *);
extern float func_001FA5C8(float, float);
extern void func_L00_0025C558(void *, void *, float, int, int, int);
extern int func_L00_0025FF38(float, char *, void *, int, int, void *, int);
extern int func_001FA6E0(float, int, int);
extern int D_L01_001612D0 MACRO_ADDR;
extern int D_L01_00160058_x __asm__("D_L01_0015FFD8") MACRO_ADDR;
extern short D_L01_00161998;
extern short D_L01_0016199C;
extern const float D_L01_00161A30_c SDATA(D_L01_001619A0);
extern const float D_L01_00161A34_c SDATA(D_L01_001619A4);
extern const float D_0015EE6C_c __asm__("D_0015ED6C") MACRO_ADDR;


/* Moby update: hit test against a target, hurt/debris on a hit, else path search and bob. */
void FUN_L01_002eec68(char *m) {
    V_2F0040_c2eec68 v;
    V_2F0040_c2eec68 w;
    int flag;
    float amount;
    float a1;
    float a2;
    char *d;
    char *t;
    char *p;
    char *x;
    char *e;
    char *t2;
    char *q;
    char *cand;
    u128_2F0040_c2eec68 *wp;
    int c;
    char *p2;
    char *x2;
    char *b;
    int c258;
    int c260;
    float f20;
    float f21;
    float f22;
    float f24;
    float dist;
    float ang;
    float k;
    float f7;

    amount = 0.0f;
    d = *(char **)(m + 0x78);
    t = func_L00_0025A420(m, 0x330000, 0);
    if (t != 0) {
        qcopy(&v, m + 0x10);
        v.f[2] = v.f[2] + 0.5f;
        p = *(char **)(t + 0x20);
        if (p != 0) {
            short cls = *(short *)(p + 0xA6);
            if (cls == 0x47) {
                qcopy(&w, D_0013E533 + 0xEED);
                wp = &w.q;
            } else {
                x = *(char **)(p + 0x24);
                if ((x != 0 && *(short *)(x + 0x46) == 5) || cls == 0xBA) {
                    qcopy(&w, &v);
                    wp = &w.q;
                } else {
                    qcopy(&w, p + 0x10);
                    wp = &w.q;
                }
            }
        } else {
            qcopy(&w, t);
            wp = &w.q;
        }
        if (func_001F9B48(&v, wp) < 24.0f) {
            if (func_L00_001EFFF0_t(&v, wp, 2, m, 0) != 0) {
                t = 0;
            }
        }
    }
    func_L00_0025A478(m, t, d + 0x20, 0, &flag, &amount, 0, 4);
    if (flag != 1 && amount != 0.0f && *(unsigned char *)(m + 0x20) != 0xB && *(unsigned char *)(m + 0x20) != 0xA && *(unsigned char *)(m + 0x20) != 8) {
        f20 = 0.8f;
        func_0022DA68(0, 0, m);
        f24 = 1.0f;
        f22 = 1024.0f;
        f21 = 0.008f;
        *(float *)(d + 0x20) = *(float *)(d + 0x20) - f24;
        d[0x9D] = 0;
        *(float *)(d + 0x70) = f21;
        *(int *)(d + 0x84) = 8;
        *(int *)(d + 0x80) = func_001FA6D0(*(float *)(d + 0x250) * f20 * f22);
        *(float *)(d + 0x88) = *(float *)(d + 0x250) * f20;
        func_L01_002EF770((unsigned char *)m, (float *)(t + 0x10));
        if (0.0f < *(float *)(d + 0x20)) {
            t2 = func_L01_002EF350(m);
            if (t2 != 0) {
                q = *(char **)(t2 + 0x78);
                q[0x9D] = 3;
                *(float *)(q + 0x70) = f21;
                *(int *)(q + 0x84) = 8;
                *(int *)(q + 0x80) = func_001FA6D0(*(float *)(d + 0x250) * f20 * f22);
                k = D_0015EE6C_c;
                f7 = 7.0f;
                *(float *)(q + 0x88) = *(float *)(d + 0x250) * f20;
                *(float *)(q + 0x78) = D_L01_00161A30_c * k;
                *(float *)(q + 0x7C) = D_L01_00161A34_c * k;
                *(int *)(q + 0x84) = *(int *)(q + 0x84) | 4;
                *(float *)(q + 0xB0) = f7;
                *(float *)(q + 0xB4) = 13.0f;
                v.q = *(u128_2F0040_c2eec68 *)(t + 0x10);
                func_L00_0025AB48(&v, &a1, q + 0x78, q + 0x7C);
                a1 = func_001FA5C8(a1, 0.7853982f);
                func_L00_0025C558(t2, q + 0x60, a1, 4, 8, 0);
                t2[0x20] = 8;
                *(float *)(t2 + 0x58) = f24;
            }
            t2 = func_L01_002EF350(m);
            if (t2 != 0) {
                q = *(char **)(t2 + 0x78);
                *(float *)(q + 0x70) = f21;
                q[0x9D] = 3;
                *(int *)(q + 0x84) = 8;
                *(int *)(q + 0x80) = func_001FA6D0(*(float *)(d + 0x250) * f20 * f22);
                k = D_0015EE6C_c;
                f7 = 7.0f;
                *(float *)(q + 0x88) = *(float *)(d + 0x250) * f20;
                *(float *)(q + 0x78) = D_L01_00161A30_c * k;
                *(float *)(q + 0x7C) = D_L01_00161A34_c * k;
                *(int *)(q + 0x84) = *(int *)(q + 0x84) | 4;
                *(float *)(q + 0xB0) = f7;
                *(float *)(q + 0xB4) = 13.0f;
                v.q = *(u128_2F0040_c2eec68 *)(t + 0x10);
                func_L00_0025AB48(&v, &a2, q + 0x78, q + 0x7C);
                a2 = func_001FA580(a2, 0.7853982f);
                func_L00_0025C558(t2, q + 0x60, a2, 4, 8, 0);
                t2[0x20] = 8;
                *(float *)(t2 + 0x58) = f24;
            }
            p2 = *(char **)(t + 0x20);
            if (p2 != 0 && (x2 = *(char **)(p2 + 0x24)) != 0 && *(short *)(x2 + 0x46) == 5) {
                func_L00_00257470(m, 0x800, -1);
            } else {
                func_L00_00257470(m, 0, -1);
            }
            func_0020C828(m);
            return;
        }
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
        func_0020C828(m);
        return;
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    c258 = *(int *)(d + 0x258);
    if (c258 != 0 && *(unsigned char *)(m + 0x20) != 0
        && *(float *)(m + 0x18) < *(float *)(c258 * 0x1190 + D_L01_001612D0 + 8)) {
        func_001F99F8(&v);
        func_L01_002EF770((unsigned char *)m, (float *)&v);
        func_0020C828(m);
        return;
    }
    if (*(int *)(d + 0x38) != 0) {
        *(int *)(d + 0x22C) = func_001F9688(0xF0);
        *(int *)(d + 0x38) = 0;
    }
    if (func_001F9740(d + 0x22C) != 0) {
        *(float *)(d + 0x228) = *(float *)(d + 0x224);
    } else {
        *(float *)(d + 0x228) = *(float *)(d + 0x224) + 20.0f;
    }
    c260 = *(int *)(d + 0x260);
    if (c260 <= 0 || *(unsigned char *)((c260 << 8) + D_L01_00160058_x + 0xBC) != 0) {
        e = D_L01_001B0930[*(int *)(d + 0x24C)];
        b = d + 0x1C0;
        if (func_L00_0025FF38(*(float *)(d + 0x228), m, b, 0, 0, e + 0x10, *(int *)e) != 2) {
            if (*(float *)(d + 0x228) < func_001F9B80(d + 0x210, b)
                || 3.0f < func_001F99C0(*(float *)(m + 0x18) - *(float *)(d + 0x1C8))) {
                *(int *)(d + 0x204) = 2;
            }
        }
    }
    c = *(int *)(d + 0x248);
    if (c >= 0) {
        cand = func_L01_002EF478(m, c);
        if (cand != 0) {
            dist = func_001F9B80(m + 0x10, cand + 0x10);
            if (dist < *(float *)(d + 0x228)) {
                if (*(int *)(d + 0x204) == 2
                    || dist < func_001F9B80(m + 0x10, *(char **)(d + 0x200) + 0x10)) {
                    *(char **)(d + 0x200) = cand;
                    *(int *)(d + 0x204) = 1;
                }
            }
        }
    }
    if (*(int *)(d + 0x200) == 0) {
        *(int *)(d + 0x200) = *(int *)(D_0013E533 + 0x2E9D);
    }
    ang = func_001FA580(*(float *)(d + 0x254), D_0015ED6C * 9.424778f);
    *(float *)(d + 0x254) = ang;
    *(int *)(m + 0x90) = func_001FA6E0((func_001F9DE0(ang) + 1.0f) * 0.5f,
                                           *(int *)&D_L01_00161998, *(int *)&D_L01_0016199C);
}
