/* Carried from the PAL decompilation's src/overlays/l13_gemlik/vendor_002B2020.c (matched C there) (games/rac1) through the version
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
extern void func_001FA030(void *, void *);
extern void func_L00_002EA9D8(void *);
extern void func_L00_002EAA30(void *);
extern int func_0022DA68(int, int, int);
extern void func_L00_0028D918(int);
extern void func_L00_002EAAA0(void *, void *, int, int, int);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, int, float, float, int, int, int, int);
extern float func_001F9B20(void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9AF0(void *);
extern unsigned char D_0013A4E0[];
extern unsigned char D_0013E533[];
void func_L13_002B97F8(char *moby);
extern float D_0015ED6C MACRO_ADDR;
extern void func_00214DB0(void *, float, float, float);
void func_L13_002B9DF8(char *moby, char *obj, float p2, float p3);
void func_L13_002B9F58(char *moby, char *obj);
extern float D_0015ED60 MACRO_ADDR;
extern float D_0015ED64 MACRO_ADDR;
extern char *D_L13_0016016C_c __asm__("D_L13_001600EC") MACRO_ADDR;
extern char *D_L13_00167180_c __asm__("D_L13_00167100");
extern short D_L13_00161438;
extern char D_L13_00174140[];
extern char D_L13_001F4450[];
extern float D_L13_0016CDF0;
extern float func_L00_0025B8C0(float *, float *, float, float, float, float);
extern float func_L00_0025BC98(char *, char *, int, float, float, float, float);
extern float func_L13_002BA088(int, int, float, float, float, float, float);
extern void func_001F9CF8(void *, void *, void *);
extern void func_L00_001FF660(void *, void *, void *);
typedef float W4_3E8_c2ba178[4] __attribute__((aligned(16)));
typedef struct {
    char pad00[0x10];
    W4_3E8_c2ba178 pos;
    unsigned char state;
    char pad21[0x13];
    unsigned short flags;
    char pad36[0xA];
    float r40;
    float r44;
    float r48;
    char pad4C[0x70];
    unsigned char bc;
} Moby_3E8_c2ba178;
typedef struct {
    char pad00[0x10];
    W4_3E8_c2ba178 v10;
    float f20, f24, f28, f2C;
    float f30, f34, f38, f3C;
    char pad40[0x20];
    unsigned char sel;
    unsigned char flip;
    short s62;
    float speed;
    short count;
    short s6A;
    float f6C;
    float f70;
    char pad74[0x1C];
    float f90, f94, f98, f9C;
    float fA0, fA4, fA8, fAC;
    float fB0, fB4, fB8, fBC;
    float fC0, fC4, fC8, fCC;
    float fD0, fD4, fD8;
    int iDC;
    char padE0[0x2C];
    int i10C;
    int i110;
} Obj_3E8_c2ba178;



/* Flies the player's fighter: throttle and stick input, steering toward the path, camera, and collision response. */
void FUN_L13_002ba178(void *a0, void *a1) {
    Moby_3E8_c2ba178 *m = a0;
    Obj_3E8_c2ba178 *o = a1;
    W4_3E8_c2ba178 old;
    W4_3E8_c2ba178 vel = { 0.0f, 0.0f, D_0015ED6C * 8.0f, 0.0f };
    W4_3E8_c2ba178 dir;
    W4_3E8_c2ba178 side;
    W4_3E8_c2ba178 up = { 0.0f, 0.0f, 1.0f, 0.0f };
    W4_3E8_c2ba178 ang;
    W4_3E8_c2ba178 target;
    W4_3E8_c2ba178 diff;
    float a;
    float b;
    float ka;
    float kb;
    float lenA;
    float lenB;
    float d;
    float e1, e2;
    float h, tn;
    float ratio;
    char *pad;
    char *g;
    char *e;
    char *p;
    int idx;
    int hc;

    m->flags &= 0xFEFF;
    if (m->bc == 0) {
        if (o->count > func_001F9688(120)) {
            pad = (char *)D_0013A4E0 + 0x2460;
            if (*(int *)(pad + 0x1B4) & 0x40) {
                idx = o->i10C;
                if (idx != -1) {
                    e = (char *)D_0013E533 + 0x1D + idx * 0x70;
                    if (*(char **)(e + 0x88) == (char *)m && *(unsigned char *)(e + 0x74) != 0) {
                        func_L00_0028D918(idx);
                    }
                }
                o->i10C = -1;
                o->i10C = func_0022DA68(1, 4, (int)m);
            } else if (*(int *)(pad + 0x1B8) & 0x40) {
                idx = o->i10C;
                if (idx != -1) {
                    e = (char *)D_0013E533 + 0x1D + idx * 0x70;
                    if (*(char **)(e + 0x88) == (char *)m && *(unsigned char *)(e + 0x74) != 0) {
                        func_L00_0028D918(idx);
                    }
                }
                o->i10C = -1;
            }
            pad = (char *)D_0013A4E0 + 0x2460;
            if (*(int *)(pad + 0x1B0) & 0x40) {
                float f = o->speed + D_0015ED60 * 0.035f;
                float max = D_0015ED6C * 30.0f;
                o->speed = f;
                if (max < f) {
                    o->speed = max;
                }
            } else {
                float hi = D_0015ED6C * 24.0f;
                float step = D_0015ED60 * 0.025f;
                float cur = o->speed;
                if (cur < hi - step) {
                    o->speed = cur + step;
                } else if (hi < cur) {
                    o->speed = cur + (hi - cur) * (D_0015ED60 * 0.1f);
                } else {
                    o->speed = hi;
                }
            }
        } else {
            o->count += 1;
            o->speed += D_0015ED60 * 0.003f;
        }
    } else {
        o->speed = 0.0001f;
    }

    if (o->count > func_001F9688(5)) {
        char *h = (char *)D_0013E533 + 0xE1D;
        ka = D_0015ED60 * 0.1f;
        kb = D_0015ED60 * 0.25f;
        a = *(float *)(h + 0x1D20);
        b = *(float *)(h + 0x1D24);
        if (*(int *)(D_0013A4E0 + 0x2600) & 0x200) {
            a = a / (D_0015ED60 * 0.012271847f);
            a = a * (D_0015ED60 * 0.015707964f);
            b = b / (D_0015ED60 * 0.012271847f);
            b = b * (D_0015ED60 * 0.015707964f);
            ka = D_0015ED60 * 0.08f;
            kb = D_0015ED60 * 0.08f;
        }
        func_L00_0025B8C0(&o->f6C, &o->f90, a, D_0015ED64 * 0.03f, D_0015ED64, ka);
        func_L00_0025B8C0(&o->f70, &o->f94, b, D_0015ED64 * 0.03f, D_0015ED64, kb);
        func_L00_0025BC98((char *)&o->f30, (char *)&o->f98, 0, o->f6C, D_0015ED64 * 0.05f, D_0015ED64, D_0015ED60);
        o->f34 = func_001FA5C8(o->f34, o->f70 * (D_0015ED60 * 0.012271847f));
        o->f38 = func_001FA5C8(o->f38, o->f6C * (D_0015ED60 * 0.012271847f));
        if (o->f34 > 1.3962634f) {
            o->f34 = 1.3962634f;
        } else if (o->f34 < -1.3962634f) {
            o->f34 = -1.3962634f;
        }
        if (m->r44 > 1.3962634f) {
            m->r44 = 1.3962634f;
        } else if (m->r44 < -1.3962634f) {
            m->r44 = -1.3962634f;
        }
    }

    lenA = 310.0f;
    lenB = 500.0f;
    if (o->i110 != -1) {
        int off = o->i110 << 7;
        char *base = D_L13_0016016C_c;
        qcopy(target, ((char (*)[128])base)[o->i110] + 0x30);
        lenA = func_001F9AF0(base + off + 0x20);
        func_001F9A10(diff, (o->i110 << 7) + D_L13_0016016C_c, (o->i110 << 7) + D_L13_0016016C_c + 0x10);
        lenB = func_001F9AF0(diff);
    } else {
        qcopy(target, &D_L13_00161438);
    }

    d = func_001F9B80(m->pos, &D_L13_00161438);
    o->f38 = func_L13_002BA088(1, 5, d, o->f38, func_L00_001FF8B0(target[0] - m->pos[0], target[1] - m->pos[1]), 50.0f, lenB);
    m->r48 = func_L13_002BA088(1, 5, d, m->r48, func_L00_001FF8B0(target[0] - m->pos[0], target[1] - m->pos[1]), 50.0f, lenB);
    e1 = func_001F99C0(target[2] - m->pos[2]);
    o->f34 = func_L13_002BA088(1, 5, e1, o->f34, -func_L00_001FF8B0(func_001F9B80(m->pos, target), target[2] - m->pos[2]), 50.0f, lenA);
    e2 = func_001F99C0(target[2] - m->pos[2]);
    m->r44 = func_L13_002BA088(1, 5, e2, m->r44, -func_L00_001FF8B0(func_001F9B80(m->pos, target), target[2] - m->pos[2]), 50.0f, lenA);
    func_00214DB0(dir, o->speed, o->f38, -o->f34);
    func_L00_001FF500(o->v10, dir, 1.0f);
    qcopy(old, m->pos);
    if (m->bc == 0) {
        func_001F9A10(m->pos, m->pos, dir);
    }
    {
        float lo = D_0015ED6C * 24.0f;
        float sp = o->speed - lo;
        float range = D_0015ED6C * 30.0f - lo;
        if (sp < 0.0f) {
            ratio = 0.0f / range;
        } else {
            ratio = sp / range;
        }
    }
    o->fC4 = ratio;
    {
        W4_3E8_c2ba178 tbl[2] = {
            { (-15.0f - o->f70 - ratio * 4.0f) * 0.25f,
              o->f6C * -3.0f * 0.25f,
              (4.0f + o->f70 * 2.0f) * 0.25f },
            { -1.275f,
              0.0f,
              0.65f }
        };
        float mat[12];
        W4_3E8_c2ba178 w;
        W4_3E8_c2ba178 r;
        if (m->state == 2) {
            o->fB4 = tbl[o->sel][0];
            o->fB8 = tbl[o->sel][1];
            o->fBC = tbl[o->sel][2];
        } else {
            func_L00_0025B8C0(&o->fB4, &o->fC8, tbl[o->sel][0], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
            func_L00_0025B8C0(&o->fB8, &o->fCC, tbl[o->sel][1], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
            func_L00_0025B8C0(&o->fBC, &o->fD0, tbl[o->sel][2], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
        }
        diff[0] = o->fB4;
        diff[1] = o->fB8;
        diff[2] = o->fBC;
        diff[3] = 0.0f;
        func_001FA030(mat, (char *)&o->f30);
        func_001F9CF8(target, diff, mat);
        func_001F9A10(w, m->pos, target);
        func_L00_002EA9D8(w);
        r[0] = func_001FA580(o->f30 * 0.5f, o->f6C * 0.03125f);
        r[1] = func_001FA5C8(func_001FA580(o->f34, 0.09817477f), o->f70 * (D_0015ED60 * 0.012271847f));
        r[2] = o->f38;
        if (m->state == 2) {
            qcopy(&o->f20, r);
        } else {
            func_L00_0025BC98((char *)&o->f20, (char *)&o->fA0, 0, r[0], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
            func_L00_0025BC98((char *)&o->f24, (char *)&o->fA4, 0, r[1], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
            func_L00_0025BC98((char *)&o->f28, (char *)&o->fA8, 0, r[2], D_0015ED64 * 0.15f, D_0015ED64 * 0.8f, D_0015ED60);
        }
        ang[0] = o->f20;
        ang[2] = o->f28;
        ang[1] = o->f24;
        func_L00_002EAA30(ang);
        if (*(short *)(D_L13_00167180_c + 0x86) != 5) {
            func_001E93B0(D_L13_001F4450);
            func_L00_002EAAA0(w, ang, 0, 0, 0);
        }
    }
    func_L00_0025B8C0(&o->fB0, &o->fC0, 1.134464f + (1.4835298f - 1.134464f) * ratio, 0.05f, 1.0f, 1.0f);
    h = o->fB0 * 0.5f;
    tn = func_001F9DE0(h) / func_001F9DC8(h);
    D_L13_0016CDF0 = tn;
    func_001F2D98();
    m->r40 = func_001FA580(o->f30, o->f6C * 0.5f);
    m->r44 = func_001FA580(o->f34, -o->f70 * 0.5f);
    m->r48 = func_001FA580(o->f38, -o->f6C * 0.0f);
    if (func_L00_001F0D60(1.2f, m->pos, 0, m)) {
        p = D_L13_00174140;
        if (*(char **)(p + 0x18) == 0 || (hc = *(short *)(*(char **)(p + 0x18) + 0xA6), hc != 0 && hc != 0x192 && hc != 0x4C2 && hc != 0x4C3 && hc != 0x52 && hc != 0x53)) {
            qcopy(m->pos, p + 0x30);
            func_L00_001FF660(dir, dir, p + 0x40);
            o->f38 = func_001FA580(o->f38, func_001FA5C8(func_L00_001FF8B0(dir[0], dir[1]), o->f38) * (D_0015ED60 * 0.1f));
            o->f34 = func_001FA580(o->f34, func_001FA5C8(-func_L00_001FF8B0(func_001F9B20(dir), dir[2]), o->f34) * (D_0015ED60 * 0.1f));
            o->speed = o->speed * 0.7f;
            if (*(char **)(p + 0x18) != 0) {
                if (o->iDC == 0) {
                    g = (char *)D_0013E533 + 0xE1D;
                    *(float *)(g + 0x15FC) = *(float *)(g + 0x15FC) - 5.0f;
                    if (*(float *)(g + 0x15FC) < 0.0f) {
                        func_001F9A28(side, p + 0x50, p + 0x60);
                        func_L00_001FF500(side, side, 1.0f);
                        func_L00_001FF660(vel, dir, p + 0x40);
                        func_L00_001FF500(vel, vel, D_0015ED6C + D_0015ED6C);
                        func_L00_001FF500(up, p + 0x40, 1.0f);
                        func_L00_0025E450(m, vel, 0, 8.0f, 100.0f, 0x1E, 0xA, 0x18, 16.0f, 8.0f, 9.0f, 6, 1.0f, 50.0f, 1, 1, -1, 0);
                        o->s62 = func_001F9688(240);
                        m->state = 6;
                    }
                    o->iDC = func_001F9688(10);
                }
            } else {
                *(float *)(D_0013E533 + 0x2419) = 0.0f;
                func_L00_0025E450(m, vel, 0, 8.0f, 100.0f, 0x1E, 0xA, 0x18, 16.0f, 8.0f, 9.0f, 6, 1.0f, 50.0f, 1, 1, -1, 0);
                m->flags |= 0x41;
                o->s62 = func_001F9688(240);
                m->state = 6;
            }
        }
    }
    func_001F4600(func_L13_002B97F8, m);
    func_L13_002B9F58((char *)m, (char *)o);
    func_L13_002B9DF8((char *)m, (char *)o, func_001FA580(o->f28, -o->f6C / 7.0f),
                      -func_001FA580(o->f24, -(o->f70 / 7.0f) - 0.1f));
}
