/* Carried from the PAL decompilation's nonmatching/l12_hoven/func_L12_0030A880.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L12_003094a0 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: SIZE ours 136 / retail 152, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   variant tried (p6 to p10) moved it.
 *   3. Address at the call. Retail keeps the `lui` high part in `$4` (`daddu $4,$2,
 *   $0`) and rebuilds `addiu $2,$4,%lo` at the call; ours keeps the full address
 *   in `$6`. Two extra instructions in retail, from the same allocator tie.
 *   Verdict for the next worker: stop here unless a source form that keeps the
 *   state constant away from the index compare shows up. Three variants (p7, p8,
 *   p9) compiled to the same bytes, so this is the allocator tie the protocol
 *   describes. Not a known wall in LEVERS.md.
 */
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

/* Runs a selected callback while the level is in mode two. */
void FUN_L12_003094a0(unsigned char *moby) {
    int state = moby[0x20];
    switch (state) {
    case 0:
        moby[0x30] = 0xFF;
        moby[0x20] = 1;
        break;
    case 1:
        if (D_L12_0015F5C4 == 2) {
            Level12MobyList_c3094a0 *g = &D_L12_0016CCE0;
            int cur = g->current;
            if (cur == state || cur == 7) {
                int index = 0;
                if (cur == state) index = 2;
                func_L00_002637F8((int)g->mobys[index]);
            }
        }
        break;
    }
}
