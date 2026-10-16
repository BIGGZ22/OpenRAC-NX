/* Carried from the PAL decompilation's src/overlays/l08_batalia/vendor_002B9438.c (matched C there) (games/rac1) through the version
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
extern void func_L00_001FF4B0_2B94B0(void *, void *, float) __asm__("func_L00_001FF500");
extern float D_0015EE70_2B94B0 __asm__("D_0015ED70") MACRO_ADDR;
extern int D_L08_001746D8_2B94B0i __asm__("D_L08_00174658");
extern int func_001F9688(int);
extern float func_002132A8(float, float);
typedef int v128_36F8_c2d23f0 __attribute__((mode(TI)));
typedef struct { char b[16]; } blk16_c2d23f0;
struct Probe_36F8_c2d23f0 { float v[4]; void *moby; int flags; int pad; float f; int st; };
struct Ray_36F8_c2d23f0 { float start[4]; float end[4]; struct Probe_36F8_c2d23f0 x; };
union Coll_36F8_c2d23f0 { struct Ray_36F8_c2d23f0 r; struct Probe_36F8_c2d23f0 p; };
extern int func_L00_001F0D60(float, void *, int, void *);
extern void func_L00_001FF660(void *, void *, void *);
extern float func_001F9AF0(void *);
extern float func_001FA580(float, float);
extern int func_00213260(int);
extern void func_L00_00269958(void *, void *, int, int, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_001EFC70(void *, void *, int, void *, int);
extern int func_0022DA68(int, int, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_001F99F8(void *);
extern int func_001F9740(void *);
extern void func_L00_0025D458(void *, short *);
extern void func_L00_0025D538(void *, void *);
extern int func_L00_001F2868(void *, float, int, void *, void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_L00_0025F090(void *, void *, int, float, float);
extern void func_0020C828(void *);
extern void func_L00_00259FE8(void *, float);
extern float D_0015EE6C_36F8 __asm__("D_0015ED6C") MACRO_ADDR;
extern char D_L08_001746C0_36F8[] __asm__("D_L08_00174640");
extern float D_L08_0015F660_36F8[] __asm__("D_L08_0015F580") MACRO_ADDR;
extern char D_L08_001746E0_36F8[] __asm__("D_L08_00174660");
extern blk16_c2d23f0 D_L08_00161868;
extern short D_L08_00161860;



/* Bomb (class 248) update: fuse, bounce and blast state machine on the moby's state byte */
void FUN_L08_002d23f0(char *moby) {
    union Coll_36F8_c2d23f0 c;
    blk16_c2d23f0 t70;
    float t80[4];
    float t90[4];
    char *d;
    int st;
    int two;
    float f20;

    d = *(char **)(moby + 0x78);
    if (*(unsigned char *)(moby + 0x20) != 0x37) {
        if (func_L00_001F0D60(0.5f, moby + 0x10, 0x11, moby)) {
            int g = D_L08_001746D8_2B94B0i;
            if (g != 0 && (g != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0)) {
                if (!(*(unsigned char *)(moby + 0xBC) & 2)) *(unsigned char *)(moby + 0x20) = 0x62; else *(unsigned char *)(moby + 0x20) = 0x63;
                qcopy((moby + 0x10), D_L08_001746E0_36F8);
                func_L00_001FF660(d + 0x10, d + 0x10, D_L08_001746E0_36F8 + 0x20);
                func_L00_001FF4B0_2B94B0(d + 0x10, d + 0x10, D_0015EE6C_36F8 + D_0015EE6C_36F8);
            }
        }
    }

    st = *(unsigned char *)(moby + 0x20);
    two = 2;
    switch (st) {
    case 1: {
        float t;
        t = func_001F9AF0((d + 0x10));
        *(float *)(moby + 0x40) = func_001FA580(*(float *)(moby + 0x40), t * 0.5f);
        if (*(int *)(d + 0x24) == 0) {
            float lo = -1.0f;
            float hi = 1.0f;
            int *q;
            t70 = D_L08_00161868;
            *(v128_36F8_c2d23f0 *)t90 = 0;
            t90[0] = func_002132A8(lo, hi);
            t90[1] = func_002132A8(lo, hi);
            t90[2] = func_002132A8(lo, hi);
            *(v128_36F8_c2d23f0 *)t80 = *(v128_36F8_c2d23f0 *)t90;
            func_L00_001FF4B0_2B94B0(t80, t80, D_0015EE6C_36F8);
            q = (int *)t70.b + func_00213260(4);
            func_L00_00269958((moby + 0x10), t80, *q, 0x4FFF, func_001F9688(0x2D), 0x32, 0x78, 1);
        }
        qcopy(c.r.start, moby + 0x10);
        f20 = 0.16f;
        c.r.start[2] -= f20;
        func_001F9A10((moby + 0x10), (moby + 0x10), (d + 0x10));
        *(float *)(d + 0x18) -= *(float *)&D_L08_00161860 * D_0015EE70_2B94B0;
        qcopy(c.r.end, moby + 0x10);
        c.r.end[2] -= f20;
        c.r.x.moby = moby;
        c.r.x.flags = 0x810000;
        c.r.x.st = st;
        c.r.x.f = 1.0f;
        qcopy(c.r.x.v, d + 0x10);
        if (func_L00_001EFC70(c.r.start, c.r.end, 0, moby, 0)) {
            char *tbl = D_L08_001746C0_36F8;
            int v = *(int *)(tbl + 0x18);
            if (v != 0) {
                if (v != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0) {
                    if (!(*(unsigned char *)(moby + 0xBC) & 2)) *(unsigned char *)(moby + 0x20) = 0x62; else *(unsigned char *)(moby + 0x20) = 0x63;
                    qcopy((moby + 0x10), D_L08_001746E0_36F8);
                    {
                        char *v = d + 0x10;
                        func_L00_001FF660(v, v, D_L08_001746E0_36F8 + 0x20);
                        func_L00_001FF4B0_2B94B0(v, v, D_0015EE6C_36F8 + D_0015EE6C_36F8);
                    }
                }
            } else if (*(int *)(tbl + 0x1C) > 0) {
                if (*(unsigned char *)(moby + 0xBC) & 1) {
                    float k;
                    func_0022DA68(1, 0, moby);
                    qcopy((moby + 0x10), tbl + 0x20);
                    *(float *)(moby + 0x18) += f20;
                    func_L00_001FF660((d + 0x10), (d + 0x10), tbl + 0x40);
                    func_001F9AD8(&t70, (d + 0x10), tbl + 0x40);
                    k = D_0015EE6C_36F8 * 1.5f;
                    func_L00_001FF4B0_2B94B0(&t70, &t70, func_002132A8(-k, k));
                    func_001F9A10((d + 0x10), (d + 0x10), &t70);
                    func_001F9A68((d + 0x10), (d + 0x10), 0.4f);
                    *(int *)(d + 0x24) += 1;
                    *(int *)(d + 0x20) = func_001F9688(0xB4);
                    if (func_001F9AF0((d + 0x10)) < D_0015EE6C_36F8) {
                        int r;
                        *(unsigned char *)(moby + 0x20) = two;
                        r = func_001F9688(0x78);
                        *(int *)(d + 0x20) = r;
                        *(float *)(d + 0x28) = 254.0f / (float)r;
                        func_001F99F8((d + 0x10));
                    }
                } else {
                    *(unsigned char *)(moby + 0x20) = 0x62;
                    qcopy((moby + 0x10), tbl + 0x20);
                    func_L00_001FF660((d + 0x10), (d + 0x10), tbl + 0x40);
                    func_L00_001FF4B0_2B94B0((d + 0x10), (d + 0x10), D_0015EE6C_36F8 + D_0015EE6C_36F8);
                }
            }
        } else if (func_001F9740(d + 0x20)) {
            if (*(unsigned char *)(moby + 0xBC) & 2) {
                *(unsigned char *)(moby + 0x20) = 0x63;
            } else {
                *(unsigned char *)(moby + 0x20) = 0x62;
            }
        }
        break;
    }
    case 2:
        if (func_001F9740(d + 0x20)) {
            *(unsigned char *)(moby + 0x20) = 0x63;
        } else if (*(int *)(d + 0x20) == func_001F9688(0x3C)
                   || *(int *)(d + 0x20) == func_001F9688(0x28)
                   || *(int *)(d + 0x20) == func_001F9688(0x14)) {
            *(unsigned char *)(d + 7) = 0xFA;
            func_L00_0025D458(moby, (short *)d);
        }
        func_L00_0025D538(moby, d);
        break;
    case 0x62:
        func_0022DA68(0, 0, moby);
        c.p.moby = moby;
        c.p.flags = 0x810001;
        c.p.f = 1.0f;
        func_001F99F8(&c.p);
        c.p.st = 0;
        func_L00_001F2868(moby + 0x10, 1.0f, 0x10, moby, &c.p);
        func_L00_0025E450(moby, D_L08_0015F660_36F8, 0, 0.0f, 0.0f, 7, 10, 20, 3.0f, 1.7f, 4.0f, 1.0f, -1, 7.0f, 0, 7, -1, 0);
        func_0020C828(moby);
        return;
    case 0x63: {
        char *p;
        func_0022DA68(0, 0, moby);
        p = moby + 0x10;
        c.p.moby = moby;
        c.p.flags = 0x810001;
        c.p.f = 1.0f;
        func_001F99F8(&c.p);
        c.p.st = 0;
        func_L00_001F2868(p, 1.0f, 0x10, moby, &c.p);
        func_L00_0025F090(moby, p, -1, 0.5f, 13.0f);
        func_0020C828(moby);
        return;
    }
    }
    func_L00_00259FE8(moby, 0.2f);
}
