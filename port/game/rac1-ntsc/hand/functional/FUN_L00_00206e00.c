/* Carried from the PAL decompilation's src/overlays/shared/help_00203E98.c (matched C there) (games/rac1) through the version
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
extern s32 func_001F9688(s32);
extern unsigned char D_0013E533[] NOT_SDA;
/* NON_MATCHING FUN_L00_00206e00 -- src/overlays/shared/help_00203E98.c
 * Best so far: SIZE ours 716 / retail 720, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Cheat-code/input-sequence recorder: decides a code s (1..0xC) from game state words at base+0x198/0x208C/0x208
 *   p1 is the same size (720) with the right structure; stopped with 3 runs unspent because the diff is allocator/
 *   retail shares constant 4 (A==4 compare reg reused for s=4) but not 0xA (ours does the reverse), and the loop h
 */
extern int D_0015EEA0 MACRO_ADDR;
extern int D_0015EE20 MACRO_ADDR;
extern unsigned char D_0015EDB4[4] MACRO_ADDR;
extern unsigned char D_0015EDB0[] MACRO_ADDR;
extern unsigned char D_0015EDC0[] MACRO_ADDR;
extern unsigned char D_L00_00179700[][16];
extern void func_L00_00263D40(int arg0, int arg1);



// Records a pressed-input code in a 16-entry ring buffer and toggles any cheat whose sequence now ends the buffer.
void FUN_L00_00206e00(void) {
    char *g;
    int s;
    int i;
    if (D_0015EEA0 == 0 && D_0015EE20 == 0) return;
    g = (char *)D_0013E533 + 0xE1D;
    s = 0;
    if (*(int *)(g + 0x198) == func_001F9688(0xF)) {
        if (*(int *)(g + 0x208C) == 4) {
            int b = *(int *)(g + 0x2084);
            if (b == 0xB) {
                int c = *(int *)(g + 0x450);
                if (c == 0) {
                    s = 1;
                    if (D_0015EDB4[0]) s = 2;
                } else if (c == 1) {
                    s = 2;
                    if (D_0015EDB4[0]) s = 1;
                } else if (c == 3) {
                    s = 3;
                }
            } else if (b == 0x11) {
                s = 9;
            } else if (b == 0xA) {
                s = 0xA;
            } else if (b == 0xE) {
                s = 5;
            }
        } else if (*(int *)(g + 0x208C) == 6) {
            int b = *(int *)(g + 0x2084);
            if (b == 0x14) {
                s = 6;
            } else if (b == 0x13) {
                s = *(int *)(g + 0xA60) + 0xC;
            } else if (b == 0x15) {
                s = 4;
            }
        } else if (*(int *)(g + 0x2084) == 0x22) {
            s = 0xB;
        }
    } else if (*(int *)(g + 0x198) == func_001F9688(0x3C)) {
        if (*(int *)(g + 0x2084) == 4) {
            s = 7;
        } else if (*(int *)(g + 0x2084) == 8) {
            s = 8;
        }
    }
    if (s == 0) return;
    {
    char *q = (char *)D_0013E533 + 0xE1D;
    unsigned char *b0 = (unsigned char *)q + 0x21C4;
    b0[*(int *)(q + 0x21D4)] = s;
    *(int *)(q + 0x21D4) += 1;
    for (i = 0; i < 12; i++) {
        char *g2 = (char *)D_0013E533 + 0xE1D;
        unsigned char *b = (unsigned char *)g2 + 0x21C4;
        int n = 0;
        int j;
        int k;
        while (D_L00_00179700[i][n] != 0) n++;
        if (n == 0) continue;
        k = *(int *)(g2 + 0x21D4) - n;
        if (k < 0) k += 16;
        for (j = 0; j < n; j++) {
            if (*(unsigned char *)(k + (int)b) != D_L00_00179700[i][j]) {
                j = -1;
                break;
            }
            k++;
            if (k >= 16) k -= 16;
        }
        if (j == n) {
            int t = D_0015EDB0[i] == 0;
            D_0015EDC0[i] = 1;
            D_0015EDB0[i] = t;
            func_L00_00263D40(t ? 0x4FBE : 0x4FBF, -1);
            break;
        }
    }
    }
    {
        char *r = (char *)D_0013E533 + 0xE1D;
        if (*(int *)(r + 0x21D4) >= 16) *(int *)(r + 0x21D4) = 0;
    }
}
