/* Carried from the PAL decompilation's nonmatching/game/func_L00_0020B060.c (games/rac1) through the version
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
extern unsigned char D_0013E533[] NOT_SDA;
extern char D_L00_0017A680[];
extern int func_001F9740(int *);
extern float func_L00_00257C48(float lo, float hi);
extern int func_001F9688(int);
extern s32 func_L00_00257B90(s32, s32);
extern float func_002132A8(float, float);

/* Four slots of random-angle pairs in the state at D_0013E533 + 0xE1D: each slot i checks
   func_001F9740 on its word at +0x1060 + 4i and, if that is nonzero, draws a float (and a
   sign-folded float for slot 3) and words from the random helpers, then copies the slot's two
   floats (+0x1024, +0x1028, stride 16) into the 176-byte records of D_L00_0017A680 at 13 + i.
   Returns at once when the word at +0x2084 is set or the byte at +0x2080 -> +0x53 is set. */
void FUN_L00_0020a9e8(void) {
    unsigned char *g = (unsigned char *)D_0013E533 + 0xE1D;
    unsigned char *B = g + 0x1060;
    unsigned char *A = (unsigned char *)D_L00_0017A680;
    int i;

    if (*(int *)(g + 0x2084) != 0) {
        return;
    }
    if (*(unsigned char *)(*(int *)(g + 0x2080) + 0x53) != 0) {
        return;
    }
    for (i = 0; i < 4; i++) {
        unsigned char *p20 = B + 4 * i;
        unsigned char *p17 = g + 16 * i;
        int rec_index = i + 13;
        float f0;
        float f1;
        int r;

        if (func_001F9740((int *)p20) != 0) {
            if (i == 0 || i == 1) {
                float x = func_L00_00257C48(0.17453292f, 0.52359879f);
                int a, b;
                if (i == 0) {
                    *(float *)(g + 0x1028) = x;
                    a = func_001F9688(70);
                    b = func_001F9688(150);
                    r = func_L00_00257B90(a, b);
                    *(int *)(D_0013E533 + 0x1E7D) = r;
                } else {
                    *(float *)(B - 0x28) = x;
                    a = func_001F9688(40);
                    b = func_001F9688(90);
                    r = func_L00_00257B90(a, b);
                    *(int *)(B + 4) = r;
                }
            } else if (i == 2) {
                float x = func_L00_00257C48(0.261799395f, 0.87266463f);
                int a, b;
                *(float *)(B - 0x18) = x;
                a = func_001F9688(40);
                b = func_001F9688(90);
                r = func_L00_00257B90(a, b);
                *(int *)(B + 8) = r;
            } else {
                float w = func_002132A8(-0.0872664601f, 0.52359879f);
                float old;
                float v;
                int a, b;
                *(float *)(p17 + 0x1024) = w;
                old = *(float *)(p17 + 0x1028);
                v = func_L00_00257C48(0.34906584f, 0.959931076f);
                if ((old <= 0.0f && v <= 0.0f) || (old >= 0.0f && v >= 0.0f)) {
                    v = -v;
                }
                *(float *)(p17 + 0x1028) = v;
                a = func_001F9688(35);
                b = func_001F9688(70);
                r = func_L00_00257B90(a, b);
                *(int *)(p20) = r;
            }
        }
        f1 = *(float *)(p17 + 0x1028);
        f0 = *(float *)(p17 + 0x1024);
        *(float *)(A + rec_index * 0xB0 + 0x64) = f0;
        *(float *)(A + rec_index * 0xB0 + 0x68) = f1;
    }
}
