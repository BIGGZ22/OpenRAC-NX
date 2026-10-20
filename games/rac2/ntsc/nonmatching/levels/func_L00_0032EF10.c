/* func_L00_0032EF10 (160 bytes, 27 places, shared, default profile -O2 -G0)
   -- measured 0 of 27: candidate is 152 bytes, two instructions short.

   Scans a 30-slot ring for a free slot: from i = 1 up to min(*(s32 *)(D_00138180 +
   0x190), arg1) exclusive, index (*(s16 *)(D_00138180 + 0x18E) - i + 30) % 30 into
   the table at +0x1E0, and returns the first entry whose masked word is non-zero
   (writing i - 1 through the third argument when it is not null); 0 when none.

   Form-dependent details now measured, all of them needed:
     - the divisor 30 must be a VARIABLE holding 30 (`s32 count = 30;`): with a
       literal the whole `%` folds into a multiply and the `beql $t0,$t3 ; break 0,7`
       divide guard disappears;
     - `if (n >= arg1) n = arg1;` gives retail's `slt $v1,$v0,$a1 ; movz $v0,$a1,$v1`.
       The max form (`if (n < arg1) n = arg1;`) gives `movn` with the operands the
       other way round - measured, not assumed;
     - one `char *base = D_00138180;` local (three uses of the global give three
       address materialisations, 152 with an extra `lui`), plus a `s32 *tbl =
       (s32 *)(base + 0x1E0);` local so that `addiu $t4,$base,0x1E0` is hoisted into
       the preheader as retail does instead of folding 0x1E0 into the index;
     - `while (i < n)` on the single bound variable, with n incremented in place.

   The residue is register allocation, not shape. Retail's preheader holds two
   copies this cc1 does not create:
     daddu $t1,$t0,$zero     (base, because $t0 is then reused for the count 30)
     daddu $t2,$v0,$zero     (bound, because $v0 is clobbered by the loop's mfhi)
   and the register assignment differs with it: retail i = $a3, base = $t0, bound =
   $v0 -> $t2; here i = $t0, base = $t2, bound = $a3 (and $t1 = count). The whole
   instruction multiset otherwise aligns, including the `lh`/`nop` and both `beql`
   arms. Five runs (~30 forms: declaration orders, explicit base/limit copies - both
   fold away - `for`/do-while/guard variants, struct fields, ternary min) all land on
   148 or 152. `-mno-split-addresses` (wave 7's lever for `lui %hi; sw %lo($r)`
   global stores) leaves all five of those sizes unchanged - measured, so the
   residue here is the two copies, not the address form. */

typedef short s16;
typedef int s32;
typedef unsigned int u32;

extern char D_00138180[];

s32 func_L00_0032EF10(u32 mask, s32 arg1, s32 *out) {
    char *base = D_00138180;
    s32 count = 30;
    s32 n = *(s32 *)(base + 0x190);
    s32 i = 1;
    s32 *tbl;

    if (n >= arg1)
        n = arg1;
    n = n + 1;
    tbl = (s32 *)(base + 0x1E0);
    while (i < n) {
        s32 *p = tbl + (*(s16 *)(base + 0x18E) - i + count) % count;
        if ((*p & mask) != 0) {
            if (out != 0)
                *out = i - 1;
            return *p & mask;
        }
        i++;
    }
    return 0;
}
