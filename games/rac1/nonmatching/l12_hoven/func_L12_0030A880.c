/* NON_MATCHING func_L12_0030A880 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: SIZE ours 144 / retail 152, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: the index select. Every plain form (ternary, default then override, if/else, short or int
 *   index, function-scope index) compiles to xori/movn. Retail branches. A `static inline unsigned char`
 *   helper returning 2 or 0 keeps the branch (the subreg lever in DECOMP_PROGRESS.md). With it inline in the
 *   subscript (variant p) only two instructions are missing: retail keeps the %hi of D_L12_0016CD60 in $4 and
 *   rebuilds the base at the call as `addiu %lo` then `addiu 0x178`. Ours folds both into one addiu. Putting
 *   the index in its own local threads the jump (BYTES 10/152, same size). A list pointer at function scope
 *   gets hoisted. Not tried: GCSE-sensitive forms that reference the global in two separate blocks.
 *   fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18).
 */
#include "common.h"
extern int D_L12_0015F6A8 MACRO_ADDR;
extern void func_L00_00264870(int);
typedef struct {
    char pad0[0x30];
    int current;
    char pad34[0x10];
    short count;
    char pad46[0x132];
    char *mobys[1];
} Level12MobyList;
extern Level12MobyList D_L12_0016CD60;

static inline unsigned char func_L12_pick(int cur) {
    if (cur == 1) return 2;
    return 0;
}

void func_L12_0030A880(unsigned char *moby) {
    switch (moby[0x20]) {
    case 0:
        moby[0x30] = 0xFF;
        moby[0x20] = 1;
        break;
    case 1:
        if (D_L12_0015F6A8 == 2) {
            int cur = D_L12_0016CD60.current;
            if (cur == 1 || cur == 7) {
                func_L00_00264870((int)D_L12_0016CD60.mobys[func_L12_pick(cur)]);
            }
        }
        break;
    }
}
