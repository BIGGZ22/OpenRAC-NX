/* Carried from the PAL decompilation's src/overlays/l10_orxon/vendor_002E30F8.c (matched C there) (games/rac1) through the version
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
extern int func_001F9688(int);
extern int func_001F9740(void *);
extern float func_001F9B80(void *, void *);
extern float func_001FA688(float, float);
extern unsigned char D_0013E533[];
extern float func_L00_001FF8B0(float, float);
extern void func_L00_00266858(void *, int);
extern int func_00215130(int, int);
extern void func_L00_00298840(int);
extern int func_0020B178(int, int);
extern void func_L00_00263D40(int, int);
extern unsigned char D_0013D255[];
extern char D_0013A4E0[];
extern int D_L10_0015F5C4 MACRO_ADDR;
extern int D_0015ED98 MACRO_ADDR;
extern short D_0015EDA0;
extern short D_L10_0015F640;
static inline short msgSel_c2e8358(void) {
    if (D_0013D255[0x140] != 0) {
        return 0x271D;
    }
    return 0x271C;
}



// Update of a pickup-style moby: waits for the player to come near, then spends bolts on a purchase.
void FUN_L10_002e8358(char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned char *g;
    unsigned char *p;
    int one;
    func_001F9740((int *)(d + 4));
    switch (*(unsigned char *)(m + 0x20)) {
    case 0:
        g = D_0013E533 + 0xE1D;
        if (func_001FA688(*(float *)(m + 0x48), func_L00_001FF8B0(*(float *)(g + 0x80) - *(float *)(m + 0x10), *(float *)(g + 0x84) - *(float *)(m + 0x14))) < 1.5707964f) {
            if (func_001F9B80(m + 0x10, g + 0x80) < 4.0f) {
                if (*(int *)(d + 4) == 0) {
                    if (D_L10_0015F5C4 != 2) {
                        func_L00_00266858(m, 1);
                        p = D_0013D255 + 0x13B;
                        if (p[4] == 0) {
                            if (D_0015ED98 > 4000) {
                                int c = func_00215130(1, 0x271E) != 0;
                                if ((*(int *)(D_0013A4E0 + 0x2604) & 0x10) && c) {
                                    d = (char *)1;
                                    p[4] = (int)d;
                                    *(int *)(g + 0x22A8) = 5;
                                    D_0015ED98 = D_0015ED98 - 4000;
                                    *(int *)&D_0015EDA0 = 5;
                                    func_L00_00298840(0);
                                    m[0x20] = (int)d;
                                }
                            } else {
                                func_00215130(1, 0x2720);
                            }
                        } else if (p[5] == 0) {
                            if (*(int *)(d + 4) == 0) {
                                if (D_0015ED98 > 30000) {
                                    int c = func_00215130(1, 0x271F) != 0;
                                    if ((*(int *)(D_0013A4E0 + 0x2604) & 0x10) && c) {
                                        d = (char *)1;
                                        p[5] = (int)d;
                                        *(int *)(g + 0x22A8) = 8;
                                        D_0015ED98 = D_0015ED98 - 30000;
                                        *(int *)&D_0015EDA0 = 8;
                                        func_L00_00298840(1);
                                        m[0x20] = (int)d;
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
        break;
    case 1:
        if (D_L10_0015F5C4 != 2) {
            int msg;
            func_0020B178(0, -1);
            func_L00_00263D40(msgSel_c2e8358(), -1);
            *(int *)&D_L10_0015F640 = 0xB4;
            *(int *)(d + 4) = func_001F9688(0xF0);
            m[0x20] = 0;
        }
        break;
    }
}
