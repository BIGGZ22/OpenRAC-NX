/* Best attempt for func_L13_0040A2C8 (232 bytes, 2 places) — NOT MARKED.
   last check.py line: "13_boldan: 228 bytes, expected 232" (1 instruction
   off, function 4 bytes too short).

   Variant tried next, WORSE (220 bytes, "13_boldan: 220 bytes, expected 232"):
   locals declared (t, v, q) instead of (q, v, t) so that the null vector lands
   in sp+0x20, plus a local TI128 `z = 0;` before the store. The register
   allocation comes closer (s3 = a0+0x10, a1 and v remain swapped: retail
   s1 = a1, s2 = v) but the length drops 8 bytes more — the variant
   below remains the best.
   What is left to solve:
   - the null 128-bit store: the retail keeps the register
     form `por $v0,$zero,$zero ; sq $v0,0x20($sp)` whereas `*(TI128*)q = 0;`
     is folded by cc1 into `sq $zero,0x20($sp)` (1 instruction less).
     The rac2_fold_zero_ti_store pass only folds if the register is dead after
     the store AND the materialisation is a (set (reg:TI) 0) less than 12
     instructions back: so a source is needed where the zero stays live,
     or is materialised in another mode. Not found in the time of the lot.
   - the declaration order of the locals: the retail places the three 16-byte
     vectors at sp+0x00 (used by 3 calls), sp+0x10 (copy of the previous, held
     in $s2) and sp+0x20 (the null vector). The declaration order tried here
     (q, v, t) gives the inverse order (q at sp+0), to be inverted into (t, v, q).

   The rest of the shape (prologue, call sequence, addresses D_L13_001BFD00
   +0x18/+0x20/+0x30, store order) conforms to the retail. */

#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_00283098(char *, char *, char *);
extern void func_00282CC0(char *, char *, char *);
extern int func_00277258(void *, int, void *, float);
extern void func_00282CF0(void *, void *, void *);
extern char D_L13_001BFD00[];

int func_L13_0040A2C8(char *a0, char *a1)
{
    float q[4];
    float v[4];
    float t[4];
    TI128 w;

    *(TI128 *)q = 0;
    q[2] = *(float *)(a1 + 0) + *(float *)(a1 + 4);
    qcopy(v, q);
    func_00283098((char *)v, (char *)v, a0 + 0xC0);
    func_00282CC0((char *)t, a0 + 0x10, (char *)v);
    if (func_00277258((char *)t, *(int *)(a1 + 0x38) & 2, a0, *(float *)(a1 + 0)) == 0) {
        return 0;
    }
    w = *(TI128 *)(D_L13_001BFD00 + 0x20);
    *(int *)(a1 + 0x84) = *(int *)(D_L13_001BFD00 + 0x18);
    *(TI128 *)(a1 + 0x60) = w;
    *(TI128 *)(D_L13_001BFD00 + 0x30) = w;
    *(TI128 *)t = w;
    func_00282CF0(a0 + 0x10, (char *)t, (char *)v);
    return 1;
}
