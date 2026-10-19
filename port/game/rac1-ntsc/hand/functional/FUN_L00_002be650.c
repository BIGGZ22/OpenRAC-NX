/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002BF958.c (games/rac1) through the version
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
extern char D_0013E533[];
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9D20(void *, void *, void *);
long func_001F4868_BEF58(int) __asm__("func_001F44B8");
void func_001F9AD8(void *, void *, void *);
void func_L00_001FD228(void *, void *, int);
/* functional: medium. Draws up to three glow quads (slots flagged in D_L00_00161818): per slot the alpha comes from func_001FA6D0 of D_L00_001617E8[i], the 3-row frame is built from D_L00_001DD0D0[i] (cross products with the camera up vector D_0013E533+0x10AD) and the four corners from D_L00_001DBF20 scaled by D_L00_00161808[i]. */
typedef struct { float f[4]; } V_2BF958_c2be650 __attribute__((aligned(16)));
typedef struct {
    V_2BF958_c2be650 corner[4];
    int color[4];
    int unk50, unk54, unk58;
    float unk5C, unk60;
    int unk64;
    float unk68, unk6C;
    long unk70, tex, unk80, unk88;
} Q_2BF958_c2be650;
extern void vmul_2BF958(float, void *, void *) __asm__("func_001F9A68");
extern int D_L00_001618E8_2BF958[] __asm__("D_L00_00161818");
extern float D_L00_001618B8_2BF958[] __asm__("D_L00_001617E8");
extern float D_L00_001618D8_2BF958[] __asm__("D_L00_00161808");
extern V_2BF958_c2be650 D_L00_001DD3A0_2BF958[] __asm__("D_L00_001DD0A0");
extern V_2BF958_c2be650 D_L00_001DD3D0_2BF958[] __asm__("D_L00_001DD0D0");
extern V_2BF958_c2be650 D_L00_001DC220_2BF958[] __asm__("D_L00_001DBF20");
extern int D_L00_00161830_2BF958 __asm__("D_L00_00161760");
extern int D_L00_00161834_2BF958 __asm__("D_L00_00161764");



void FUN_L00_002be650(void *o) {
    Q_2BF958_c2be650 quad;
    /* The matrix FUN_001f9d20 reads, four rows in a row as the US frame keeps them (sp+0x90..0xC0):
     * the slot's direction, the axis, the negated up vector and the slot's position. Separate locals
     * left the position out of the matrix (nothing said they sit together). */
    V_2BF958_c2be650 frame[4], vertex1;
#define scaled frame[0]
#define axis frame[1]
#define negativeUp frame[2]
#define vertex0 frame[3]
    int color;
    int i, j, c;

    quad.tex = func_001F4868_BEF58(8);
    quad.unk88 = 0x8000000048L;
    quad.unk80 = 0xFF9000000260L;
    quad.unk5C = 1.0f;
    quad.unk70 = 0;
    quad.unk50 = 0;
    quad.unk54 = 0;
    quad.unk58 = 0;
    quad.unk60 = 1.0f;
    quad.unk64 = 0;
    quad.unk68 = 1.0f;
    if (((unsigned char *)D_0013E533)[2] != 0) {
        c = D_L00_00161834_2BF958;
    } else {
        c = D_L00_00161830_2BF958;
    }
    color = c & 0xFFFFFF;
    for (i = 0; i < 3; i++) {
        if (D_L00_001618E8_2BF958[i] != 0) {
            unsigned int col = ((unsigned int)func_001FA898_r(D_L00_001618B8_2BF958[i]) << 24) | color;
            quad.color[0] = col;
            quad.color[1] = col;
            quad.color[2] = col;
            quad.color[3] = col;
            vertex0 = D_L00_001DD3A0_2BF958[i];
            scaled = D_L00_001DD3D0_2BF958[i];
            vmul_2BF958(-1.0f, &negativeUp, D_0013E533 + 0x10AD);
            func_001F9AD8(&axis, &scaled, &negativeUp);
            func_L00_001FF500(&axis, &axis, 1.0f);
            func_001F9AD8(&negativeUp, &axis, &scaled);
            for (j = 0; j < 4; j++) {
                vmul_2BF958(D_L00_001618D8_2BF958[i], &vertex1, &D_L00_001DC220_2BF958[j]);
                func_001F9D20(&quad.corner[j], &vertex1, &scaled);
            }
            func_L00_001FD228(&quad, 0, 0);
        }
    }
#undef scaled
#undef axis
#undef negativeUp
#undef vertex0
}
