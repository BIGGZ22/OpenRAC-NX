/* Carried from the PAL decompilation's src/overlays/shared/hud_00235960.c (matched C there) (games/rac1) through the version
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
extern int D_0015ED84 MACRO_ADDR;
extern char D_0013D407[];
/* NON_MATCHING FUN_L00_0023d198 -- src/overlays/shared/hud_00235960.c
 * Best so far: BYTES 36/572 (93.7% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_0023d198: vendor stock init. If D_0015ED84 >= 19 it fills two 0x25-byte flag arrays with 1, sets seve
 *   Best candidate p7.c: schedule identical to retail (source order bytes, then ints 4..0x18, then the 0x1C store 
 *   Would unblock: making the int-constant pseudos rank before the byte-constant pseudos without changing the sche
 */
extern unsigned char D_0015EDD0[] MACRO_ADDR;
extern int D_L00_001C3E98[];
extern void func_L00_0023D3D8(int idx);
extern unsigned char D_0013D4E3[] NOT_SDA;
extern unsigned char D_0013D4C2[] NOT_SDA;
extern unsigned char D_0014161B[] NOT_SDA;
extern void func_L00_00260860(int, int);



/* Initialise the vendor stock list: fixed defaults once the level is at least 19, else prune unknown entries. */
void FUN_L00_0023d198(void) {
    int i, j, v, k;
    unsigned char c;
    if (D_0015ED84 >= 0x13) {
        for (k = 0; k < 0x25; k++) {
            (D_0013D407 + 0xB9)[k] = 1;
            (D_0013D4E3 + 5)[k] = 1;
        }
        D_0015EDD0[0] = 0x4A;
        D_0015EDD0[1] = 0x50;
        D_0015EDD0[2] = 0x4F;
        D_0015EDD0[3] = 0x54;
        D_0015EDD0[4] = 0x51;
        D_0015EDD0[5] = 0x4E;
        D_0015EDD0[6] = 0x4B;
        D_0015EDD0[7] = 0x52;
        D_0015EDD0[8] = 0x4D;
        D_0015EDD0[9] = 0x59;
        D_0015EDD0[10] = 0x58;
        D_0015EDD0[11] = 0x53;
        *(int *)(D_0014161B + 0x885) = 0xF;
        *(int *)(D_0014161B + 0x889) = 0xC;
        *(int *)(D_0014161B + 0x88D) = 0xD;
        *(int *)(D_0014161B + 0x891) = 0xB;
        *(int *)(D_0014161B + 0x895) = 0x11;
        *(int *)(D_0014161B + 0x899) = 0xA;
        *(int *)(D_0014161B + 0x89D) = 0x10;
        *(int *)(D_0014161B + 0x8A1) = 0x13;
    } else {
        if (D_0013D4C2[8] == 0) {
            func_L00_00260860(0xA, 1);
            D_0015EDD0[0] = 0x4A;
            *(int *)(D_0014161B + 0x45) = 0xA;
        }
        for (i = 0; i < 12; i++) {
            c = D_0015EDD0[i];
            if (c == 0xFF) continue;
            v = c & 0x3F;
            if (v == 10) continue;
            for (j = 0; j < 19; j++) {
                if (D_L00_001C3E98[j] == v) break;
            }
            if (j == 19 || v == 0) {
                D_0015EDD0[i] = 0xFF;
            }
        }
        func_L00_0023D3D8(D_0015ED84);
    }
}
