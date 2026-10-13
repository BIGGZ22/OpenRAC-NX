/* Carried from the PAL decompilation's src/game/music.c (matched C there) (games/rac1/pal) through the version
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
extern short D_001516D0[];
extern int D_00137B80[];
extern void func_0012EC08(int, int, int, int, short, int, int, int,
                          int, void (*)(int, long), long);
extern void func_00216A80(int, long);

int FUN_00215e00(int arg0, int arg1, int arg2, int arg3) {
    char *s = (char *)D_001516D0;
    char *base;
    int *tbl;
    long h;

    if (*(int *)(s + 0x6C) == 0) {
        base = (char *)D_00137B80;
        tbl = (int *)(base + 0x2AA8);
        if (tbl[arg1] != 0) {
            *(short *)(s + 0x74) = arg2;
            h = tbl[arg1];
            *(int *)(s + 0x80) = 10;
            *(int *)(s + 0x84) = 0xBB80;
            *(unsigned int *)(s + 0x6C) = 0xFFFFFFFF;
            *(short *)(s + 0x70) = arg0;
            *(short *)(s + 0x76) = 1;
            *(short *)(s + 0x7C) = 1;
            *(short *)(s + 0x72) = arg3;
            func_0012EC08(h, 0, 0, 0, arg3, 0, 1, 0, 0x20,
                          func_00216A80, (long)(unsigned int)(s + 0x6C));
        } else {
            return 0;
        }
    } else {
        return 0;
    }
    return 1;
}
