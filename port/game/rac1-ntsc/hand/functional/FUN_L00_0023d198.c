/* Carried from the PAL decompilation's nonmatching/game/func_L00_0023DB30.c (games/rac1/pal) through the version
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
extern int D_0015ED84 MACRO_ADDR;
extern char D_0013D407[];
extern int D_0014171B_i[] __asm__("D_0014161B");

/* Veldin level code: vendor stock-level table setup (retail FUN_L00_0023d198, 0x23C bytes).
 * With the level number below 19 it fills the id list at D_0015EDD0 from the table D_L00_001C3E98
 * (ids in the low six bits, 0xFF ends the list), dropping ids the table lacks; otherwise it fills two
 * flag arrays and the 8-word record at D_0014161B + 0x885. Ends with func_L00_0023D3D8 on the level. */
extern unsigned char D_0013D4C2[];
extern char D_0013D4E3[];
extern int D_L00_001C3E98[];
extern unsigned char D_0015EDD0[] MACRO_ADDR;
void func_L00_00260860(int, int);
void func_L00_0023D3D8(int);

void FUN_L00_0023d198(void) {
    int lvl = D_0015ED84;
    int i;

    if (lvl >= 0x13) {
        char *b7 = D_0013D407 + 0xB9;
        char *b6 = D_0013D4E3 + 5;
        int *s = (int *)((char *)D_0014171B_i + 0x885);

        for (i = 0; i < 0x25; i++) {
            b7[i] = 1;
            b6[i] = 1;
        }
        s[0] = 0xF;
        s[1] = 0xC;
        s[2] = 0xD;
        s[3] = 0xB;
        s[4] = 0x11;
        s[5] = 0xA;
        s[6] = 0x10;
        s[7] = 0x13;
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
    } else {
        int *t = D_L00_001C3E98;

        if (D_0013D4C2[8] == 0) {
            func_L00_00260860(0xA, 1);
            D_0015EDD0[0] = 0x4A;
            *(int *)((char *)D_0014171B_i + 0x45) = 0xA;
        }
        i = 0;
        while (i < 12) {
            int v = D_0015EDD0[i];
            int v4;
            int k;

            if (v == 0xFF) {
                i++;
                continue;
            }
            v4 = v & 0x3F;
            if (v4 == 0xA) {
                i++;
                continue;
            }
            if (t[0] == v4) {
                k = 0;
            } else {
                for (k = 1; k < 0x13; k++) {
                    if (t[k] == v4)
                        break;
                }
            }
            if (k == 0x13 || v4 == 0)
                D_0015EDD0[i] = 0xFF;
            i++;
        }
        func_L00_0023D3D8(D_0015ED84);
    }
}
