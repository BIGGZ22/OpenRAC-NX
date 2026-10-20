/* func_L08_0037E6D8 (116 bytes, 2 places) -- NEAR MISS, not integrated.
   state: 116 bytes, 26 of 29 words identical; one word of the body and its two knock-ons.
   check.py (08_tabora), with the source below:
     0037E714  daddu $a3, $s1, $zero    |  addiu $a2, $s1, 0x190   <- retail copies the base
     0037E718  addiu $a2, $s1, 0x190    |  nop                        into $a3 first
     0037E738  lh    $v0, 0x44($a3)     |  lh    $v0, 0x44($s1)
   Retail reads the loop bound through a second copy of the block base ($a3) while the
   preheader test reads it through $s1; here the bound is read through $s1 in both places,
   which frees a word that the assembler fills with the nop. The prologue, the call, the
   8-byte copy and the counter are retail's.

   Measured: the winning shape is the *base held in a local* (`u8 *base = D_L08_001BD800;`
   with the fields at +0x18C / +0x44 / +0x190), which is what makes cc1 materialise the
   block base once instead of folding each field into its own symbol+offset (that folding
   form is 8 diffs, `base` taken as a cast 3 diffs, pointer walks 16, a hoisted count local
   12). Two locals at the same address, the count taken through a `s16 *` local, and the
   symbol named again inside the loop condition were also tried: 3, 12 and 4 diffs. The
   missing copy is a register-allocation artefact, not a shape difference. */

extern u8 D_L08_001BD800[];
extern void func_L00_00311DC0(s32);

/* re-runs the draw routine on the block's 0x18C object, then copies the eight bytes at
   +0x38 of that object into every entry of the block's list at +0x190 (entry 0 upward,
   while the counter is under the signed halfword at +0x44) */
void func_L08_0037E6D8(void)
{
    u8 *base = D_L08_001BD800;
    s32 s0;
    s32 i;

    s0 = *(s32 *)(base + 0x18C);
    func_L00_00311DC0(s0);
    for (i = 1; i < *(s16 *)(base + 0x44); i++) {
        s32 o = *(s32 *)(base + 0x190 + (i - 1) * 4);
        *(u64 *)(o + 0x38) = *(u64 *)(s0 + 0x38);
    }
}
