/* Carried from the PAL decompilation's nonmatching/l12_hoven/func_L12_0030A880.c (games/rac1) through the version
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
extern int D_L12_0015F5C4 MACRO_ADDR;
extern void func_L00_002637F8(int);
typedef struct {
    char pad0[0x30];
    int current;
    char pad34[0x10];
    short count;
    char pad46[0x132];
    char *mobys[1];
} Level12MobyList_c3094a0;
extern Level12MobyList_c3094a0 D_L12_0016CCE0;
static inline unsigned char func_L12_pick(int cur) {
    if (cur == 1) return 2;
    return 0;
}



void FUN_L12_003094a0(unsigned char *moby) {
    switch (moby[0x20]) {
    case 0:
        moby[0x30] = 0xFF;
        moby[0x20] = 1;
        break;
    case 1:
        if (D_L12_0015F5C4 == 2) {
            int cur = D_L12_0016CCE0.current;
            if (cur == 1 || cur == 7) {
                func_L00_002637F8((int)D_L12_0016CCE0.mobys[func_L12_pick(cur)]);
            }
        }
        break;
    }
}
