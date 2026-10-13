/* Carried from the PAL decompilation's nonmatching/game/func_L00_002688A8.c (games/rac1/pal) through the version
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
/* FUN_L00_00267a08(rec): frees particle record rec (2048 records of 0x40 bytes at the pool pointer).
 * Live count--; clears the record's bit in the allocation bitmap (and in the second bitmap when
 * byte 1 has 0x40); when it was the highest live index, moves that index down to the next set
 * bit; sets byte 1 to 0x80; lowers the lowest-free hint to the record's index if smaller. */
extern unsigned char *D_0016012C SDATA(D_0016012C);
extern int D_00160130 SDATA(D_00160130);
extern int D_00160134 SDATA(D_00160134);
extern int D_00160138 SDATA(D_00160138);
extern unsigned char D_L00_001B1880[];

void FUN_L00_00267a08(unsigned char *rec) {
    unsigned int idx;
    int top;

    idx = (unsigned int)((char *)rec - (char *)D_0016012C) >> 6;
    D_00160138--;
    if (rec[1] & 0x40) {
        D_L00_001B1880[0x100 + (idx >> 3)] &= (unsigned char)~(1 << (idx & 7));
    }
    D_L00_001B1880[idx >> 3] &= (unsigned char)~(1 << (idx & 7));
    if ((int)idx == D_00160134) {
        top = D_00160134 - 1;
        while (top >= 0 && !(D_L00_001B1880[top >> 3] & (1 << (top & 7)))) {
            top--;
        }
        D_00160134 = top;
    }
    rec[1] = 0x80;
    if ((int)idx < D_00160130) {
        D_00160130 = (int)idx;
    }
}
