/* 0 of 27 places match - last line: 00_aranos_tutorial: 208 bytes, expected 204
   Everything but one instruction is in place: the request flag test, the cleared store,
   the hero-entry match, the counter/watch/flag block with its `bnel` forms and the two
   zeroed counters. The remaining difference is the first value, at 0x001A7398:
   retail reads it `lw $v1,-0x7C58($gp)` and merges it with a following `movz`, while this
   cc1 emits that load absolutely whenever the value is a `movz`/`movn` operand - measured
   over six source shapes (load hoisted, load inside the branch, ?:, two uses, pointer-typed
   symbol, -G8 unit) and every one gives `lui`+`lw`. Outside a merge the same declaration
   does give `$gp` (`lw $v0,0($gp)` with a GPREL16 relocation), so the symbol and the
   `__attribute__((sda))` form are right; only the merge context is unreached.
   The second absolute view of 0x001A7390 in the same block IS reached (retail has both a
   `$gp` and a plain absolute access to that address - two names are needed). */

#include "common.h"

extern char D_00189E20_36[];
extern char D_0018C750[];
extern int D_001A7A04;
extern int D_001A7390;
extern int D_001A79F0;
extern int ref_L00_001A7390_00 __attribute__((sda));
extern int ref_L00_001A7398_00 __attribute__((sda));
extern int func_2D3FB0(int);

void func_L00_002CB2D8(void)
{
    char *s0 = D_00189E20_36;
    char *a3;
    int ac, r, t, g;
    u16 c;

    if (*(int *)(s0 + 0x2538) == 0)
        return;
    g = ref_L00_001A7398_00;
    ac = *(int *)(s0 + 0x1248);
    r = D_001A7A04;
    if (ac == 10)
        r = g;
    *(int *)(s0 + 0x2538) = 0;
    if (r == 0)
        return;
    if (ac == D_001A7A04)
        return;
    a3 = D_0018C750;
    c = *(u16 *)(a3 + 0x174);
    if (c > 0xFFFE)
        t = ref_L00_001A7390_00;
    else {
        *(u16 *)(a3 + 0x174) = c + 1;
        t = D_001A7390;
    }
    if (*(u32 *)(a3 + 0x178) < (u32)t)
        *(int *)(a3 + 0x178) = t;
    *(int *)(a3 + 0x17C) |= (1 << D_001A79F0) | 0x80000000;
    *(int *)(s0 + 0x22C8) = func_2D3FB0(r);
    *(short *)(s0 + 0x24FA) = 0;
    *(short *)(s0 + 0x24F8) = 0;
}
