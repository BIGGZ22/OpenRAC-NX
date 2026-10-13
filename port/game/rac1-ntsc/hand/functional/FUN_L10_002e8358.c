/* Carried from the PAL decompilation's nonmatching/l10_orxon/func_L10_002E9718.c (games/rac1/pal) through the version
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
extern int func_001F9740(void *);

/* NON_MATCHING FUN_L10_002e8358 -- src/overlays/l10_orxon/vendor_002E30F8.c
 * Best so far: SIZE ours 600 / retail 608, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   UpdateMoby for a bolt-cost pickup: state 0 waits for the player in range/cone and buys option 5 or 8 (subtract
 *   bolts, sets D_0015EDA0, state 1); state 1 shows a message and resets. Best p1.c/p2.c (56 bytes differ, same si
 *   Left: m/d saved regs swapped ($s0/$s1: retail m=$s1, d=$s0), the sltu result lands in $a2 not $a0, and the
 *   0x271C/0x271D select comes out as movz instead of retail's beqz + delay-slot li. Control flow otherwise identi
 *   Moving d's assignment, int* vs char* for d and if/else forms did not move the allocation.
 */
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F9B80(void *, void *);
extern void func_L00_00266858(void *, int);
extern int func_00215130(int, int);
extern void func_L00_00298840(int);
extern int func_0020B178(int, int);
extern void func_L00_00263D40(int, int);
extern int func_001F9688(int);
extern unsigned char D_0013E533[];
extern unsigned char D_0013D255[];
extern char D_0013A4E0[];
extern int D_L10_0015F5C4 MACRO_ADDR;
extern int D_0015ED98 MACRO_ADDR;
extern short D_0015EDA0;
extern short D_L10_0015F640;

// Update of a pickup-style moby: waits for the player to come near, then spends bolts on a purchase.
void FUN_L10_002e8358(char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned char *g;
    unsigned char *p;
    int c;
    int one;
    func_001F9740((int *)(d + 4));
    if (*(unsigned char *)(m + 0x20) == 0) {
        g = D_0013E533 + 0xE1D;
        if (func_001FA688(*(float *)(m + 0x48), func_L00_001FF8B0(*(float *)(g + 0x80) - *(float *)(m + 0x10), *(float *)(g + 0x84) - *(float *)(m + 0x14))) < 1.5707964f) {
            if (func_001F9B80(m + 0x10, g + 0x80) < 4.0f) {
                if (*(int *)(d + 4) == 0) {
                    if (D_L10_0015F5C4 != 2) {
                        func_L00_00266858(m, 1);
                        p = D_0013D255 + 0x13B;
                        if (p[4] == 0) {
                            if (D_0015ED98 > 4000) {
                                c = func_00215130(1, 0x271E) != 0;
                                if ((*(int *)(D_0013A4E0 + 0x2604) & 0x10) && c) {
                                    one = 1;
                                    p[4] = one;
                                    *(int *)(g + 0x22A8) = 5;
                                    D_0015ED98 = D_0015ED98 - 4000;
                                    *(int *)&D_0015EDA0 = 5;
                                    func_L00_00298840(0);
                                    m[0x20] = one;
                                }
                            } else {
                                func_00215130(1, 0x2720);
                            }
                        } else if (p[5] == 0) {
                            if (*(int *)(d + 4) == 0) {
                                if (D_0015ED98 > 30000) {
                                    c = func_00215130(1, 0x271F) != 0;
                                    if ((*(int *)(D_0013A4E0 + 0x2604) & 0x10) && c) {
                                        one = 1;
                                        p[5] = one;
                                        *(int *)(g + 0x22A8) = 8;
                                        D_0015ED98 = D_0015ED98 - 30000;
                                        *(int *)&D_0015EDA0 = 8;
                                        func_L00_00298840(1);
                                        m[0x20] = one;
                                    }
                                } else {
                                    func_00215130(1, 0x2721);
                                }
                            }
                        }
                    }
                }
            }
        }
    } else if (*(unsigned char *)(m + 0x20) == 1) {
        if (D_L10_0015F5C4 != 2) {
            func_0020B178(0, -1);
            func_L00_00263D40(D_0013D255[0x140] ? 0x271D : 0x271C, -1);
            *(int *)&D_L10_0015F640 = 0xB4;
            *(int *)(d + 4) = func_001F9688(0xF0);
            m[0x20] = 0;
        }
    }
}
