/* Carried from the PAL decompilation's src/overlays/shared/tieproc_00299108.c (matched C there) (games/rac1) through the version
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
extern int D_L00_0015F5C4 MACRO_ADDR;
extern int D_L00_0015F5D8 MACRO_ADDR;
extern int D_L00_0015F3FC MACRO_ADDR;
extern void func_001F4A58(int);
extern s32 func_002168A8(s32);
extern int D_0015ED80 MACRO_ADDR;
extern void func_001F97E8(void *, int, int);
extern int func_00216788(int, int, int);
extern int D_L00_0015F620[4] MACRO_ADDR;
extern int D_L00_00173E00[];
extern int D_L00_001611CC MACRO_ADDR;
typedef struct { int a; int b; } LL9_c299a68;
typedef struct { char pad[0x16A8]; LL9_c299a68 normal[20]; LL9_c299a68 alt[20]; } G9_c299a68;
extern G9_c299a68 D_00137B80;



/* starts the slideshow: resets state, rebases the level data pointers, and starts the first two streams */
void FUN_L00_00299a68(void) {
    int t;
    func_002168A8(0);
    func_001F4A58(8);
    D_L00_0015F5D8 = 1;
    *(float *)&D_L00_0015F3FC = 1.0f;
    D_L00_0015F5C4 = 7;
    func_001F97E8(D_L00_0015F620, 0, 16);
    t = D_L00_001611CC - 0xE0000;
    D_L00_0015F620[2] = D_L00_00173E00[1] + t;
    D_L00_0015F620[3] = D_L00_00173E00[2] + t;
    D_L00_001611CC = t;
    if (D_0015ED80 != 0) {
        func_00216788(D_L00_0015F620[2], D_00137B80.alt[((short *)D_L00_0015F620)[1]].a, D_00137B80.alt[((short *)D_L00_0015F620)[1]].b);
    } else {
        func_00216788(D_L00_0015F620[2], D_00137B80.normal[((short *)D_L00_0015F620)[1]].a, D_00137B80.normal[((short *)D_L00_0015F620)[1]].b);
    }
    func_002168A8(1);
    ((short *)D_L00_0015F620)[1]++;
    if (D_0015ED80 != 0) {
        func_00216788(D_L00_0015F620[3], D_00137B80.alt[((short *)D_L00_0015F620)[1]].a, D_00137B80.alt[((short *)D_L00_0015F620)[1]].b);
    } else {
        func_00216788(D_L00_0015F620[3], D_00137B80.normal[((short *)D_L00_0015F620)[1]].a, D_00137B80.normal[((short *)D_L00_0015F620)[1]].b);
    }
}
