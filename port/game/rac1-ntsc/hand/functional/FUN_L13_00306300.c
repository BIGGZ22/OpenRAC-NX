/* Carried from the PAL decompilation's nonmatching/l13_gemlik/func_L13_00307798.c (games/rac1/pal) through the version
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
/* What the PAL file this was written in declares before it. */
typedef int u128_c306300 __attribute__((mode(TI)));
extern char D_0013E533[];

/* NON_MATCHING FUN_L13_00306300 -- src/overlays/l13_gemlik/vendor_002EBD00.c
 * Best so far: SIZE ours 1440 / retail 1480, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Gemlik moby update (class 1238): copies pos and data into stack vectors, a six-entry table loop over D_L13_001
 *   Still differing: the success tail of the table loop (retail keeps it inline after a beql; ours goes out of lin
 *   Unblock: a source shape for the table loop that gets the inline success tail, and a local order that puts v40 
 */
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF550(void *, void *, float);
extern float func_001F9B48(void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_00259888(void *, void *, int, float, void *);
extern void func_L00_00259A88(void *, void *);
extern float func_001F9B80(void *, void *);
extern int func_001F9740(void *);
extern float func_002132A8(float, float);
extern float func_001F9AF0(void *);
extern unsigned char *func_L07_0029A2A0(char *pos, char *vec);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern void func_L00_001FF660(void *, void *, void *);
extern int func_001F9688(int);
extern int func_L00_00257B90(int, int);
extern char *func_L00_0026DD20(char *pos, char *vel, int c, int d, float f);
extern int func_0022DA68(int, int, int);
extern void func_0020C828(void *);
extern char D_L13_00178400[];
extern float D_0015ED6C MACRO_ADDR;
extern char D_L13_00174160[];
extern char D_L13_00174140[];
extern char D_L13_00174180[];

// Gemlik moby (class 1238) update: attaches to a nearby moby and steers its vector
void FUN_L13_00306300(char *moby) {
    char *pos = moby + 0x10;
    char *data = *(char **)(moby + 0x78);
    float v0[4], v10[4], v40[4], v50[4], v60[4];
    float f0, f1, f20, f21, f22, f23;
    int flags, flag16, i, k, r16, r2, q, v;
    char *ent;
    char **tbl;
    char *e;
    char *base;
    int cls;
    struct { int w; int f; unsigned char a; unsigned char b; unsigned short c; int g; } s28;

    *(u128_c306300 *)v0 = *(u128_c306300 *)pos;
    func_001F9A10(pos, pos, data);
    *(u128_c306300 *)v40 = *(u128_c306300 *)data;
    f20 = 1.0f;
    v40[2] = 0.0f;
    func_L00_001FF550(v40, v40, f20);
    v40[2] = 1.0f;

    base = (char *)D_0013E533 + 0xE1D;
    if (base[0x1FF6] != 0) {
        if (*(float *)(data + 0xC) != 31337.6015625f) {
            if (func_001F9B48(pos, base + 0x80) < 2.0f) {
                func_L00_001FF500(data, data, 0.5f);
                func_001F9A28(pos, pos, data);
                qzero(data);
                *(int *)(data + 0x2C) = 5;
                *(float *)(data + 0xC) = 31337.6015625f;
                flags = 0x10001;
                tbl = (char **)(base + 0x2020);
                for (i = 0; i < 6; i++) {
                    ent = tbl[i];
                    if (ent == 0) continue;
                    func_L00_00259888(v10, moby, flags, 1.0f, v40);
                    s28.a = 1;
                    s28.c = *(unsigned short *)(moby + 0xA6);
                    s28.b = 1;
                    cls = (unsigned char)ent[0xA4];
                    if (cls != 0xFF) {
                        e = (char *)D_L13_00178400 + (cls << 6);
                        if (*(char **)(e + 0x34) == ent) {
                            *(float *)(e + 0x2C) = *(float *)(e + 0x2C) + 1.0f;
                            e[0x28] = 1;
                            e[0x29] = 1;
                            *(unsigned short *)(e + 0x2A) = *(unsigned short *)(moby + 0xA6);
                            *(int *)(e + 0x24) = flags;
                            break;
                        }
                    }
                    func_L00_00259A88(ent, v10);
                    break;
                }
            }
        }
    }

    func_L00_00259888(v10, moby, 0x10001, 1.0f, v40);
    s28.c = *(unsigned short *)(moby + 0xA6);
    s28.b = 1;
    s28.a = 1;
    pos = moby + 0x10;
    flag16 = (*(int *)(data + 0x24) == 0);
    f0 = func_001F9B80(pos, data + 0x10);
    f1 = *(float *)(data + 0x28);
    if (f1 < f0 || func_001F9740(data + 0x2C) != 0) {
        v = moby[0x31];
        if (v == 0) {
            func_0020C828(moby);
            return;
        }
        f21 = -1.0f;
        f23 = 0.1f;
        f22 = 4.0f;
        for (k = 2; k >= 0; k--) {
            qzero(v60);
            v60[0] = func_002132A8(f21, f20);
            v60[1] = func_002132A8(f21, f20);
            v60[2] = func_002132A8(f21, f20);
            *(u128_c306300 *)v50 = *(u128_c306300 *)v60;
            func_L00_001FF500(v50, v50, func_001F9AF0(v60) * f23);
            func_001F9A10(v50, v60, v50);
            func_L00_001FF500(v50, v50, func_002132A8(D_0015ED6C + D_0015ED6C, D_0015ED6C * f22));
            func_L07_0029A2A0(pos, (char *)v50);
        }
        func_0020C828(moby);
        return;
    }

    if (func_L00_001EFC70(v0, pos, flag16, *(void **)(data + 0x20), v10) == 0) {
        float x = *(float *)(moby + 0x10);
        float y = *(float *)(moby + 0x14);
        float z = *(float *)(moby + 0x18);
        if (x < 2.0f || 1021.0f < x || y < 2.0f || 1021.0f < y || z < 2.0f || 1021.0f < z) {
            func_0020C828(moby);
        }
        return;
    }
    *(u128_c306300 *)pos = *(u128_c306300 *)D_L13_00174160;
    if (moby[0x31] != 0) {
        f21 = -1.0f;
        for (k = 4; k >= 0; k--) {
            qzero(v60);
            v60[0] = func_002132A8(f21, f20);
            v60[1] = func_002132A8(f21, f20);
            v60[2] = func_002132A8(f21, f20);
            *(u128_c306300 *)v50 = *(u128_c306300 *)v60;
            func_L00_001FF660(v60, data, D_L13_00174180);
            func_L00_001FF500(v50, v50, func_001F9AF0(v60) * 0.5f);
            func_001F9A10(v50, v60, v50);
            f0 = func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f);
            func_L00_001FF500(v50, v50, f0);
            r16 = func_001F9688(10);
            r2 = func_001F9688(15);
            q = func_L00_00257B90(r16, r2);
            func_L00_0026DD20(pos, (char *)v50, 0x7F2F4F6F, q, 30000.0f);
        }
    }
    {
        char *pp = *(char **)(D_L13_00174140 + 0x18);
        if (pp != 0 && *(short *)(pp + 0xA6) == 0) {
            func_0022DA68(0, 0, (int)moby);
        }
    }
    func_0020C828(moby);
}
