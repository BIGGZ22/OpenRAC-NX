/* Carried from the PAL decompilation's src/game/hud.c (matched C there) (games/rac1/pal) through the version
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
extern char D_0019A4E8_raw[] __asm__("D_0019A3E8");
extern int *D_00160F00 MACRO_ADDR;
extern int D_0013E500[];

void FUN_00200468(long arg0, int arg1, int arg2, int arg3, int arg4,
                    int arg5, int arg6, int arg7, int arg8, int arg9) {
    int *base;
    long *p;
    char *arena = D_0019A4E8_raw;

    D_00160F00[0] = 0x10000005;
    D_00160F00[1] = 0;
    D_00160F00[2] = 0;
    D_00160F00[3] = 0x50000005;

    base = D_00160F00;
    D_00160F00 = base + 4;
    p = (long *)D_00160F00;
    p[0] = 0x7400000000008001L;
    p[1] = 0x5353106;
    p[2] = arg0;
    p[3] = 0x156;
    p[4] = ((long)arg9 << 24) | 0x7F7F7F;
    p[5] = arg7 | ((long)arg8 << 16);
    p[6] = (arg1 + D_0013E500[4] - 8)
         | ((long)(arg2 + D_0013E500[5] - 8) << 16)
         | ((long)*(int *)(arena + 0xC) << 32);
    p[7] = (arg7 + (1 << (arg3 + 4)))
         | ((long)(arg8 + (1 << (arg4 + 4))) << 16);
    p[8] = (arg1 + arg5 + D_0013E500[4] - 8)
         | ((long)(arg2 + arg6 + D_0013E500[5] - 8) << 16)
         | ((long)*(int *)(arena + 0xC) << 32);
    p[9] = 0;
    D_00160F00 = (int *)((char *)D_00160F00 + 0x50);
}
