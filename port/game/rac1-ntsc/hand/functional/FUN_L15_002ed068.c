/* Carried from the PAL decompilation's nonmatching/shared/func_L15_002EE458.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L15_002EE458 (near) was paired by place; read it against NTSC-U before trusting it. */
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
extern char D_0013E533[];

/* NON_MATCHING FUN_L15_002ed068 -- src/overlays/shared/vendor_002D7C00.c
 * Best so far: SIZE ours 904 / retail 912, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Giant Clank pad update: a state byte at moby+0x20 (1 -> 2; 0 -> sets the effect block from D_L15_0015FFD8 and 
 */
extern void func_0020C828(void *);
extern void func_00215130(int, int);
extern void func_L15_00208CA8(void);
extern void func_00212ED8(void *, int, int);
extern float func_00213508(void *, int, float);
extern void func_L00_00284E50(void *, void *);
extern void func_L00_00216F90(void *, void *, int, int);
extern void func_L00_00210A08(int, int, char *);
extern void func_L00_00298840(int);
extern void func_L00_002664B0__NOT_CARRIED(int, int);
extern int D_0015ED84 MACRO_ADDR;
extern int D_L15_0015F5C4 MACRO_ADDR;
extern int D_L15_00160058_m __asm__("D_L15_0015FFD8") MACRO_ADDR;
extern int D_L15_0015F594 MACRO_ADDR;
extern char D_0013A4E0[];
extern char D_0014161B[] NOT_SDA;

// Update for the giant Clank pad moby: a small state machine that sets up its effect block and its timers.
void FUN_L15_002ed068(char *m) {
    char *data = *(char **)(m + 0x78);
    char *other;
    char *o3;
    char *o2;
    char *x;
    float V[4];
    float f;
    int st;
    int idx;
    int v;

    st = ((unsigned char *)m)[0x20];
    if (st == 1) {
        m[0x20] = 2;
        return;
    }
    if (st == 0) {
        idx = *(int *)(data + 0x60);
        if (idx == -1) {
            func_0020C828(m);
            return;
        }
        other = (char *)D_L15_00160058_m + (idx << 8);
        *(unsigned short *)(other + 0x34) = *(unsigned short *)(other + 0x34) | 1;
        other[0x31] = 0;
        *(int *)(other + 0x94) = 0;
        if (D_0015ED84 == 0x12) {
            x = (char *)D_0014161B + 0xAA35 + ((unsigned char *)m)[0xB0];
            st = 3;
            if (((unsigned char *)x)[0x120] != 0xFF) st = 1;
        } else {
            st = 1;
        }
        m[0x20] = st;
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (st != 2) return;

    if (*(char **)(((char *)D_0013E533 + 0xE1D) + 0x2FC) != m) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (*(short *)(((char *)D_0013E533 + 0xE1D) + 0x30E) != 0) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (D_L15_0015F5C4 == st) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    if (*(int *)(((char *)D_0013E533 + 0xE1D) + 0x2084) == 0x1D) {
        *(int *)(data + 0x68) = -1;
        return;
    }
    v = ((unsigned char *)((char *)D_0013E533 + 0xE1D))[0x20A4];
    if (v == 0) {
        func_00215130(4, 0x3AA4);
    } else if (v == 2) {
        if (D_0015ED84 != 0x12) func_00215130(4, 0x3AA5);
    }
    if ((*(unsigned int *)(D_0013A4E0 + 0x2604) & 0x10) == 0) return;
    if (D_L15_0015F594 != 4) return;
    if (((unsigned char *)((char *)D_0013E533 + 0xE1D))[0x20A4] == 2) {
        if (D_0015ED84 == 0xF) {
            other = (char *)D_L15_00160058_m + (*(int *)(data + 0x60) << 8);
            *(unsigned short *)(other + 0x34) = *(unsigned short *)(other + 0x34) | 1;
            other[0x31] = 0;
            *(int *)(other + 0x94) = 0;
            func_L15_00208CA8();
            func_00212ED8(other, 0, 0);
            if (D_0015ED84 == 0xF) {
                func_L00_00298840(9);
                func_L00_002664B0__NOT_CARRIED(0, 5);
            }
        }
        return;
    }
    if (((unsigned char *)((char *)D_0013E533 + 0xE1D))[0x20A4] != 0) return;

    idx = *(int *)(data + 0x60);
    other = (char *)D_L15_00160058_m + (idx << 8);
    if (*(int *)(data + 0x70) != -1) {
        o2 = (char *)D_L15_00160058_m + (*(int *)(data + 0x70) << 8);
        if (o2 != 0 && ((unsigned char *)o2)[0x20] != 0xFE) {
            if (((unsigned char *)o2)[0x20] != 0xFD) {
                func_0020C828(o2);
                *(int *)(data + 0x70) = -1;
            }
        }
    }
    qcopy(V, m + 0x10);
    f = func_00213508(V, 0, 0.5f);
    f = f + 0.1f;
    *(float *)((char *)V + 8) = f;
    if (D_0015ED84 == 0xF) func_L00_00284E50(V, m + 0x40);
    func_L00_00216F90(V, m + 0x40, 0, 1);
    o3 = *(char **)(other + 0x24);
    v = *(unsigned short *)(other + 0x34) & 0xFFFE;
    other[0x31] = 1;
    *(int *)(other + 0x94) = *(int *)(o3 + 0x10);
    *(unsigned short *)(other + 0x34) = v;
    func_L00_00210A08(2, 0x5A, other);
    if (D_0015ED84 == 0xF) {
        func_L00_00298840(8);
        qcopy(D_0013E533 + 0x2B1D, D_0013E533 + 0x2B1D - 0x1C80);
        qcopy(D_0013E533 + 0x2B1D + 0x10, D_0013E533 + 0x2B1D - 0x1C70);
        func_L00_002664B0__NOT_CARRIED(2, 4);
        return;
    }
    if (D_0015ED84 != 0x12) return;
    func_L00_00298840(6);
    qcopy(D_0013E533 + 0x2B1D, D_0013E533 + 0x2B1D - 0x1C80);
    qcopy(D_0013E533 + 0x2B1D + 0x10, D_0013E533 + 0x2B1D - 0x1C70);
    m[0x20] = 3;
}
