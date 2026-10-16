/* Carried from the PAL decompilation's src/overlays/l13_gemlik/vendor_002C2638.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern float func_00213308(void);
extern float func_001F9B48(void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_0020C828(void *);
extern int func_001F9740(void *);
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern int func_001F9688(int);
extern void func_00212F90(void *, int, int, int);
/* NON_MATCHING FUN_L13_002c25b8 -- src/overlays/l13_gemlik/vendor_002C2638.c
 * Best so far: SIZE ours 1900 / retail 1912, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Blarg space fighter moby update (class 101, level 13): state machine on moby+0x20 (9 cases, jtbl_L13_001F4920)
 *   Still differs: retail rematerialises -1 for the D4 compare and the store (ours keeps one li $16,-1), and case 
 */
extern void func_L13_002C2D30(unsigned char *);
extern int func_L13_003058A8(void *, int);
extern void func_L13_00305988(char *, char *, int, float);
extern void func_001F9A28(void *, void *, void *);
extern float func_00213F38(float *, float, float *, float, float, float);
extern void func_0022DA68(int, int, int);
extern void func_001F99F8(void *);
extern short D_L13_0016149C;
extern char *D_L13_001600EC MACRO_ADDR;



/* Moby update for class 101 (blarg space fighter) on level 13: a state machine on moby+0x20, with a second switch on moby+0xBC. */
void FUN_L13_002c25b8(char *m) {
    char *d;
    char *q;
    char v20[16];
    char v30[16];
    float z40;
    float f20;
    float r;
    float fa;
    char *p10;
    char *p10b;
    float *p48b;
    float *p48;
    char *qp;
    char *pv;
    int k;
    extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);

    d = *(char **)(m + 0x78);
    func_L13_002C2D30((unsigned char *)m);
    q = *(char **)(d + 0xB0);
    k = *(unsigned char *)(m + 0x20);
    if ((unsigned)k >= 9) {
        return;
    }
    switch (k) {
    case 0:
        qcopy(d + 0xC0, m + 0x10);
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x5000;
        *(float *)(d + 0x20) = 4.0f;
        *(short *)(d + 0x24) = 4;
        *(unsigned char *)(d + 0x28) = 3;
        *(float *)(d + 0xEC) = 15.0f;
        *(unsigned char *)(d + 0x58) = 0x32;
        *(unsigned char *)(d + 0x5A) = 0;
        *(unsigned short *)(m + 0x32) = 0xFF;
        *(float *)(m + 0x18) = *(float *)(m + 0x18) + 1.0f;
        if (*(unsigned char *)(m + 0x53)) {
            func_00212F90(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(int *)(d + 0xFC) = func_L13_003058A8(m, 0);
        *(int *)(d + 0xF8) = func_L13_003058A8(m, 1);
        if (*(int *)(d + 0xD4) != -1) {
            *(unsigned char *)(m + 0x20) = 5;
        } else {
            *(unsigned char *)(m + 0x20) = 2;
        }
        *(float *)(d + 0x100) = func_00213308();
        *(float *)(d + 0x108) = func_00213308();
        *(float *)(d + 0x10C) = func_00213308();
        *(int *)(d + 0x110) = -1;
        return;
    case 5:
        if (*(int *)(d + 0xB4) == 2) {
            return;
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(unsigned char *)(m + 0x20) = 6;
        return;
    case 6:
        {
        int idx = *(int *)(d + 0xD4);
        p10 = m + 0x10;
        z40 = 0.0f;
        f20 = func_001F9B48(p10, D_L13_001600EC + (idx << 7) + 0x30);
        }
        func_001F9A28(v20, D_L13_001600EC + (*(int *)(d + 0xD4) << 7) + 0x30, p10);
        func_00213F38(&z40, f20, (float *)(d + 0xF0), D_0015ED70 * 10.0f, D_0015ED70 * 10.0f, D_0015ED6C * 20.0f);
        func_L00_001FF500(v20, v20, *(float *)(d + 0xF0));
        func_001F9A10(p10, p10, v20);
        if (!(f20 < 0.1f)) {
            return;
        }
        if (!(*(float *)(d + 0xF0) < 0.01f)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53)) {
            func_00212F90(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(unsigned char *)(m + 0x20) = 2;
        return;
    case 2:
        p48 = (float *)(m + 0x48);
        r = func_L00_001FF8B0(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        func_L00_0025BE00(p48, (float *)(d + 0xDC), r, D_0015ED70 * 4.71238899230957f, D_0015ED70 * 4.71238899230957f, D_0015ED6C * 10.122909545898438f);
        if (*(int *)(d + 0xB4) == 2) {
            return;
        }
        *(float *)(d + 0xEC) = 15.0f;
        *(unsigned char *)(m + 0xBC) = 1;
        if (*(unsigned char *)(m + 0x53) != 1) {
            func_00212F90(m, 1, 0, 0);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 4:
        p48b = (float *)(m + 0x48);
        r = func_L00_001FF8B0(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        func_L00_0025BE00(p48b, (float *)(d + 0xDC), r, D_0015ED70 * 12.566370964050293f, D_0015ED70 * 25.132741928100586f, D_0015ED6C * 6.2831854820251465f);
        switch (*(unsigned char *)(m + 0xBC)) {
        case 1:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00212F90(m, 2, 0, 2);
            }
            *(int *)(d + 0xE8) = 0;
            return;
        case 2:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 3;
            if (*(unsigned char *)(m + 0x53) != 3) {
                func_00212F90(m, 3, 0, 2);
            }
            return;
        case 3:
            if (func_001F9740((int *)(d + 0xE8)) != 0) {
                *(int *)(d + 0xE8) = func_001F9688(60);
                if (*(int *)(d + 0xF4)) {
                    func_L00_0024F7C8(m, 1, v20);
                } else {
                    func_L00_0024F7C8(m, 0, v20);
                }
                pv = v30;
                func_001F9A28(pv, q + 0x10, v20);
                func_L00_001FF500(pv, pv, *(float *)&D_L13_0016149C * D_0015ED6C);
                if (*(int *)(d + 0xF4) != 0) {
                    if (*(int *)(d + 0xF8)) {
                        func_L13_00305988(*(char **)(d + 0xF8), pv, (int)q, *(float *)(d + 0xEC));
                        func_0022DA68(0, 0, (int)m);
                    }
                    *(int *)(d + 0xF8) = func_L13_003058A8(m, 1);
                    *(int *)(d + 0xF4) = 0;
                } else {
                    if (*(int *)(d + 0xFC)) {
                        func_L13_00305988(*(char **)(d + 0xFC), pv, (int)q, *(float *)(d + 0xEC));
                        func_0022DA68(0, 0, (int)m);
                    }
                    *(int *)(d + 0xFC) = func_L13_003058A8(m, 0);
                    *(int *)(d + 0xF4) = 1;
                }
                *(float *)(d + 0xEC) = *(float *)(d + 0xEC) + 5.0f;
                p10b = m + 0x10;
                r = func_001F9B80(p10b, q + 0x10);
                if (r < *(float *)(d + 0xEC)) {
                    *(float *)(d + 0xEC) = func_001F9B80(p10b, q + 0x10);
                }
            }
            if (*(int *)(d + 0xB4) != 2) {
                return;
            }
            if (*(unsigned char *)(m + 0x53) != 4) {
                func_00212F90(m, 4, 0, 2);
            }
            *(unsigned char *)(m + 0xBC) = 4;
            return;
        case 4:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 5;
            if (*(unsigned char *)(m + 0x53) != 5) {
                func_00212F90(m, 5, 0, 2);
            }
            return;
        case 5:
            if (*(int *)(d + 0xB4) == 2) {
                if (*(unsigned char *)(m + 0x53) != 6) {
                    func_00212F90(m, 6, 0, 2);
                }
                *(unsigned char *)(m + 0xBC) = 6;
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00212F90(m, 2, 0, 2);
            }
            return;
        case 6:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            if (*(int *)(d + 0xB4) != 2) {
                *(unsigned char *)(m + 0xBC) = 1;
                if (*(unsigned char *)(m + 0x53) != 1) {
                    func_00212F90(m, 1, 0, 2);
                }
                return;
            }
            if (*(unsigned char *)(m + 0x53)) {
                func_00212F90(m, 0, 0, 2);
            }
            *(unsigned char *)(m + 0x20) = 2;
            *(unsigned char *)(m + 0xBC) = 0;
            return;
        case 0:
            *(unsigned char *)(m + 0xBC) = 1;
            if (*(unsigned char *)(m + 0x53) != 1) {
                func_00212F90(m, 1, 0, 2);
            }
            return;
        default:
            return;
        }
    case 8:
        func_001F99F8(v20);
        func_L00_0025E450(m, v20, m + 0x10, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 2, 15.0f, 1, 1, -1, 0);
        if (*(int *)(d + 0xF8)) {
            func_0020C828((void *)*(int *)(d + 0xF8));
        }
        if (*(int *)(d + 0xFC)) {
            func_0020C828((void *)*(int *)(d + 0xFC));
        }
        func_0020C828(m);
        return;
    case 7:
        if (!(*(unsigned char *)(m + 0x70) & 2)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53) != *(unsigned char *)(m + 0xBC)) {
            func_00212F90(m, *(unsigned char *)(m + 0xBC), 0, 10);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 1:
    case 3:
    default:
        return;
    }
}
