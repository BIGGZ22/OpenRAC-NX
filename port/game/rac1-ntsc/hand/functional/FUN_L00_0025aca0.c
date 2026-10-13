/* Carried from the PAL decompilation's nonmatching/shared/func_L00_0025BCF8.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L00_0025aca0 -- src/overlays/shared/mobyutil_00258BC8.c
 * Best so far: SIZE ours 1896 / retail 1936, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - Does: weighted random pick of min(n, (int)(scale*sum+0.5)) entries from a 0x20-byte table (mask bits, type =
 *   - Best so far p5.c: 1896 bytes vs 1936 (10 instructions short). Differences: retail re-materialises `lui 0x700
 *   - Wall to decide: retail's type > 4 path adds a[k] that was never assigned (`b BFB4` with `lwc1 $f0,0($20)` in
 */
extern void func_001F9A68(void *, void *, float);
extern void func_001F9A80(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *dst, void *a, void *b);
extern float func_001F9AF0(void *);
extern float func_002132A8(float, float);
extern float func_00213308(void);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9988(float);
extern void func_001F9CF8(void *, void *, void *);
extern void func_L00_001FF378(void *, void *, float);
extern void func_00125328__NOT_CARRIED(void *, void *, float);
extern void func_L00_002531E0(void *, int, int);

typedef struct Ent25BCF8_c25aca0 {
    unsigned int flags;
    union { int idx; float f; short h[2]; } u;
    int pad8;
    float fc;
    float v[4];
} Ent25BCF8_c25aca0;

typedef struct Tab25BCF8_c25aca0 {
    int w0;
    short h2;
    short pad6;
    int count;
    int pad8[2];
} Tab25BCF8_c25aca0;

typedef struct Sub25BCF8_c25aca0 {
    char pad[0x10];
    Tab25BCF8_c25aca0 *tab;
} Sub25BCF8_c25aca0;

typedef struct Obj25BCF8_c25aca0 {
    char pad0[0x10];
    float pos[4];
    int pad20;
    Sub25BCF8_c25aca0 *sub;
    int pad28;
    float scale;
    char pad30[0x90];
    float vC0[4];
} Obj25BCF8_c25aca0;

/* Picks n weighted entries of the moby's table (by mask bits and their weights), writes an offset per pick to out (16 bytes each), returns the count. */
int FUN_L00_0025aca0(void *obj_, int n, void *out_, void *mask_, float s) {
    Obj25BCF8_c25aca0 *obj = obj_;
    float *out = out_;
    int mask = (int)mask_;
    Tab25BCF8_c25aca0 *tab;
    Ent25BCF8_c25aca0 *e;
    Ent25BCF8_c25aca0 *arr[20];
    float w[20];
    float tA[4], tB[4], tC[4], tD[4], tE[4];
    float sum, c, c2, pi2, r, f0, f1, f2, f3, f20, f21, len, prev;
    float *wp, *pw;
    Ent25BCF8_c25aca0 **ap;
    int i, nValid, cnt, m, j, idx, nv, type, last;
    unsigned int flags;

    tab = obj->sub->tab;
    if (tab == 0) {
        return 0;
    }
    if (tab->h2) {
        func_L00_002531E0(obj, tab->h2, 0);
    }
    if (tab->count <= 0) {
        return 0;
    }
    e = (Ent25BCF8_c25aca0 *)((char *)tab + 0x10);

    c = 0.0009765625f;
    pi2 = 1.57079637f;
    sum = 0.0f;
    nValid = 0;
    wp = w;
    ap = arr;
    i = 0;

    do {
        flags = e->flags;
        last = flags >> 31;
        if ((((mask >> i) & 1) != 0) && (flags & 0x20000)) {
            *ap = e;
            type = flags & 0xFF;
            switch (type) {
            case 0:
            case 1:
            case 2:
                if (type < 2) {
                    qcopy(tA, e->v);
                    func_001F9A80(tA, tA, obj->scale * c);
                    f1 = tA[3];
                } else {
                    qcopy(tA, (char *)0x70000000 + e->u.idx * 16);
                    qcopy(tB, e->v);
                    func_001F9A10(tA, tA, tB);
                    func_001F9A68(tA, tA, obj->scale * c);
                    f0 = e->fc * obj->scale;
                    f1 = f0 * c;
                }
                f0 = f1 * 4.18879032f;
                f0 = f0 * f1;
                *wp = f0 * f1;
                break;
            case 3:
                qcopy(tA, e->v);
                func_001F9A68(tA, tA, obj->scale);
                f0 = obj->scale;
                f1 = tA[3] * f0;
                f0 = f0 * c;
                f1 = f1 * c;
                f3 = e->u.f * f0;
                f2 = f1 * 4.18774319f;
                f2 = f2 * f1;
                f2 = f2 * f1;
                f0 = f1 * pi2;
                f0 = f0 * f1;
                f0 = f0 * f3;
                *wp = f2 + f0;
                break;
            case 4:
                func_00125328__NOT_CARRIED(tA, (char *)0x70000000 + e->u.h[0] * 16, obj->scale * c);
                func_00125328__NOT_CARRIED(tB, (char *)0x70000000 + e->u.h[1] * 16, obj->scale * c);
                func_001F9A28(tC, tA, tB);
                len = func_001F9AF0(tC);
                f2 = e->fc * obj->scale;
                f2 = f2 * c;
                f1 = f2 * pi2;
                f1 = f1 * f2;
                f1 = f1 * len;
                *wp = f1;
                break;
            }
            sum += *wp;
            wp++;
            ap++;
            nValid++;
        }
        i++;
        e++;
    } while (!last);

    cnt = (int)(s * sum + 0.5f);
    m = n;
    if (!(m < cnt)) {
        m = cnt;
    }
    if (m <= 0) {
        return m;
    }

    c2 = 0.0009765625f;
    for (j = 0; j < m; j++) {
        r = func_002132A8(0.0f, sum);
        nv = nValid;
        idx = 0;
        if (nv > 0) {
            pw = w;
            if (!(r < *pw)) {
                prev = *pw;
                idx = 1;
                for (;;) {
                    pw++;
                    if (!(idx < nv)) {
                        r -= prev;
                        break;
                    }
                    r -= prev;
                    prev = *pw;
                    if (r < prev) {
                        break;
                    }
                    idx++;
                }
            }
        }
        e = arr[idx];
        type = *(unsigned char *)e;

        switch (type) {
        case 0:
        case 1:
        case 2:
            if (type < 2) {
                qcopy(tA, e->v);
                func_001F9A68(tA, tA, obj->scale);
                f1 = tA[3];
            } else {
                qcopy(tA, (char *)0x70000000 + e->u.idx * 16);
                qcopy(tB, e->v);
                func_001F9A10(tA, tA, tB);
                func_001F9A68(tA, tA, obj->scale);
                f1 = e->fc;
            }
            f20 = f1 * obj->scale;
            f20 = f20 * c2;
            func_L00_001FF378(tA, tA, 1024.0f);
            f21 = -f20;
            do {
                tB[0] = func_002132A8(f21, f20);
                tB[1] = func_002132A8(f21, f20);
                tB[2] = func_002132A8(f21, f20);
            } while (!(func_001F9AF0(tB) <= f20));
            func_001F9A10(tB, tB, tA);
            func_001F9CF8(tB, tB, obj->vC0);
            func_001F9A10((char *)out + j * 16, tB, obj->pos);
            break;
        case 3:
            qcopy(tA, e->v);
            func_001F9A68(tA, tA, obj->scale);
            f20 = tA[3];
            f0 = obj->scale;
            f21 = e->u.f;
            f20 = f20 * f0;
            f21 = f21 * f0;
            func_L00_001FF378(tA, tA, 1024.0f);
            f20 = f20 * c2;
            f21 = f21 * c2;
            r = func_002132A8(0.0f, f20 * f20);
            len = func_001F9988(r);
            f21 = func_002132A8(0.0f, f21);
            f20 = func_00213308();
            tB[0] = func_001F9DC8(f20) * len;
            tB[1] = func_001F9DE0(f20) * len;
            tB[2] = f21;
            func_001F9A10(tB, tB, obj->pos);
            func_001F9A10((char *)out + j * 16, tA, tB);
            break;
        case 4:
            func_00125328__NOT_CARRIED(tA, (char *)0x70000000 + e->u.h[0] * 16, obj->scale);
            func_00125328__NOT_CARRIED(tB, (char *)0x70000000 + e->u.h[1] * 16, obj->scale);
            func_001F9A28(tC, tB, tA);
            r = func_002132A8(0.0f, 1.0f);
            func_001F9A68(tC, tC, r);
            func_001F9A10(tD, tC, tA);
            func_L00_001FF378(tD, tD, 1024.0f);
            f1 = e->fc;
            f20 = f1 * obj->scale;
            f20 = f20 * c2;
            f21 = -f20;
            do {
                tE[0] = func_002132A8(f21, f20);
                tE[1] = func_002132A8(f21, f20);
                tE[2] = func_002132A8(f21, f20);
            } while (!(func_001F9AF0(tE) <= f20));
            func_001F9A10(tE, tE, tD);
            func_001F9CF8(tE, tE, obj->vC0);
            func_001F9A10((char *)out + j * 16, tE, obj->pos);
            break;
        }
    }
    return m;
}
