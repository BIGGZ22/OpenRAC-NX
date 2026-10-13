/* Carried from the PAL decompilation's nonmatching/l08_batalia/func_L08_002D36F8.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L08_002d23f0 -- src/overlays/l08_batalia/vendor_002B9438.c
 * Best so far: SIZE ours 1760 / retail 1792, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Bomb (class 248) update on level 08: a state machine on moby[0x20] (0x37 skip, 1 = fuse/arm with random-spread
 *   Differences: the prologue saves a different set of $s registers (ours saves $s4/$s5 where retail saves $s0/$s1
 *   Unblock: a form of the select that compiles to a branch with a single shared store (the movn/movz tie), then t
 */
typedef struct { char b[16]; } blk16_c2d23f0;

extern int func_L00_001F0D60(float, void *, int, void *);
extern void func_L00_001FF660(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9AF0(void *a);
extern float func_001FA580(float, float);
extern float func_002132A8(float, float);
extern s32 func_00213260(s32);
extern int func_001F9688(int);
extern void func_L00_00269958(void *, void *, int, int, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_001EFC70(void *, void *, int, void *, int);
extern int func_0022DA68(int, int, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_001F99F8(void *);
extern int func_001F9740(void *);
extern void func_L00_0025D458(void *m, short *p);
extern void func_L00_0025D538(void *, void *);
extern int func_L00_001F2BE8_2FB898(float, void *, int, void *, void *) __asm__("func_L00_001F2868");
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_L00_0025F090(void *, void *, int, float, float);
extern void func_0020C828(void *);
extern void func_L00_00259FE8(unsigned char *, float);
extern float D_0015EE6C_d __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern char D_L08_00174640[];
extern float D_L08_0015F580[];
extern char D_L08_00174660[];
extern char D_L08_00161868[];
extern short D_L08_00161860;
extern int D_L08_00174658 MACRO_ADDR;

/* Bomb (class 248) update: fuse and blast state machine on the moby's state byte */
void FUN_L08_002d23f0(char *moby) {
    char *d;
    char *p;
    char *dv;
    char *tbl;
    unsigned char st;
    int r, g, v, n, hit;
    float t, f20, f21;
    blk16_c2d23f0 t20, t70, t80, t90, t70b;
    char s30[0x34];
    char buf[0x24];

    d = *(char **)(moby + 0x78);
    dv = d + 0x10;
    st = *(unsigned char *)(moby + 0x20);
    if (st != 0x37) {
        p = moby + 0x10;
        r = func_L00_001F0D60(0.5f, p, 0x11, moby);
        if (r) {
            g = D_L08_00174658;
            if (g != 0 && (g != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0)) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
                qcopy(p, D_L08_00174660);
                func_L00_001FF660(dv, dv, D_L08_00174660 + 0x20);
                func_L00_001FF500(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
            }
        }
        st = *(unsigned char *)(moby + 0x20);
    }

    if (st == 0x62 || st == 0x63) {
        func_0022DA68(0, 0, moby);
        *(char **)(buf + 0x10) = moby;
        *(int *)(buf + 0x14) = 0x810001;
        *(float *)(buf + 0x1C) = 1.0f;
        func_001F99F8(buf);
        *(int *)(buf + 0x20) = 0;
        func_L00_001F2BE8_2FB898(1.0f, moby + 0x10, 0x10, moby, buf);
        if (st == 0x62) {
            func_L00_0025E450(moby, D_L08_0015F580, 0, 0.0f, 0.0f, 7, 10, 20, 3.0f, 1.7f, 4.0f, 1.0f, -1, 7.0f, 0, 7, -1, 0);
        } else {
            func_L00_0025F090(moby, moby + 0x10, -1, 0.5f, 13.0f);
        }
        func_0020C828(moby);
        return;
    }

    if (st == 2) {
        if (func_001F9740(d + 0x20)) {
            *(unsigned char *)(moby + 0x20) = 0x63;
        } else {
            hit = 0;
            if (func_001F9688(60) == *(int *)(d + 0x20)) hit = 1;
            else if (func_001F9688(40) == *(int *)(d + 0x20)) hit = 1;
            else if (func_001F9688(20) == *(int *)(d + 0x20)) hit = 1;
            if (hit) {
                *(unsigned char *)(d + 7) = 0xFA;
                func_L00_0025D458(moby, (short *)d);
            }
        }
        func_L00_0025D538(moby, d);
    } else if (st == 1) {
        p = moby + 0x10;
        t = func_001F9AF0(dv);
        *(float *)(moby + 0x40) = func_001FA580(*(float *)(moby + 0x40), t * 0.5f);
        if (*(int *)(d + 0x24) == 0) {
            f21 = -1.0f;
            f20 = 1.0f;
            *(blk16_c2d23f0 *)t70.b = *(blk16_c2d23f0 *)D_L08_00161868;
            qzero(t90.b);
            *(float *)(t90.b + 0) = func_002132A8(f21, f20);
            *(float *)(t90.b + 4) = func_002132A8(f21, f20);
            *(float *)(t90.b + 8) = func_002132A8(f21, f20);
            qcopy(t80.b, t90.b);
            func_L00_001FF500(t80.b, t80.b, D_0015EE6C_d);
            {
                int idx = func_00213260(4) * 4;
                int *q = (int *)(t70.b + idx);
                int s = func_001F9688(45);
                func_L00_00269958(p, t80.b, *q, 0x4FFF, s, 0x32, 0x78, 1);
            }
        }
        /* L392C */
        *(blk16_c2d23f0 *)t20.b = *(blk16_c2d23f0 *)p;
        f20 = 0.16f;
        *(float *)(t20.b + 8) = *(float *)(t20.b + 8) - f20;
        func_001F9A10(p, p, dv);
        *(float *)(d + 0x18) = *(float *)(d + 0x18) - *(float *)&D_L08_00161860 * D_0015ED70;
        *(blk16_c2d23f0 *)s30 = *(blk16_c2d23f0 *)p;
        *(float *)(s30 + 8) = *(float *)(s30 + 8) - f20;
        *(blk16_c2d23f0 *)(s30 + 0x10) = *(blk16_c2d23f0 *)dv;
        *(char **)(s30 + 0x20) = moby;
        *(int *)(s30 + 0x24) = 0x810000;
        *(float *)(s30 + 0x2C) = 1.0f;
        *(int *)(s30 + 0x30) = st;
        r = func_L00_001EFC70(t20.b, s30, 0, moby, 0);
        if (r == 0) {
            if (func_001F9740(d + 0x20)) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
            }
        } else {
            tbl = D_L08_00174640;
            v = *(int *)(tbl + 0x18);
            if (v == 0) {
                if (*(int *)(tbl + 0x1C) > 0) {
                    if (*(unsigned char *)(moby + 0xBC) & 1) {
                        func_0022DA68(1, 0, moby);
                        *(blk16_c2d23f0 *)p = *(blk16_c2d23f0 *)(tbl + 0x20);
                        t = *(float *)(moby + 0x18) + f20;
                        func_L00_001FF660(dv, dv, tbl + 0x40);
                        *(float *)(moby + 0x18) = t;
                        func_001F9AD8(t70b.b, dv, tbl + 0x40);
                        {
                            float k = D_0015EE6C_d * 1.5f;
                            float res = func_002132A8(-k, k);
                            func_L00_001FF500(t70b.b, t70b.b, res);
                        }
                        func_001F9A10(dv, dv, t70b.b);
                        func_001F9A68(dv, dv, 0.4f);
                        *(int *)(d + 0x24) += 1;
                        *(int *)(d + 0x20) = func_001F9688(180);
                        t = func_001F9AF0(dv);
                        if (t < D_0015EE6C_d) {
                            *(unsigned char *)(moby + 0x20) = 2;
                            r = func_001F9688(120);
                            *(int *)(d + 0x20) = r;
                            *(float *)(d + 0x28) = 254.0f / (float)r;
                            func_001F99F8(dv);
                        }
                    } else {
                        *(unsigned char *)(moby + 0x20) = 0x62;
                        qcopy(p, tbl + 0x20);
                        func_L00_001FF660(dv, dv, tbl + 0x40);
                        func_L00_001FF500(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
                    }
                }
            } else if (v != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
                qcopy(p, D_L08_00174660);
                func_L00_001FF660(dv, dv, D_L08_00174660 + 0x20);
                func_L00_001FF500(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
            }
        }
    }
    func_L00_00259FE8((unsigned char *)moby, 0.2f);
}
