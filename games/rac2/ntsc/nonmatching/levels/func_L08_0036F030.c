/* func_L08_0036F030 (116 bytes, 2 places) -- NEAR MISS, not integrated.
   state: 116 bytes, 22 of 29 words identical. Block 1 matches retail exactly (the guard
   `lui $a0,0x15 / lw $v1,0x2C00($a0)`, `lhu/ori/andi/sh`, the reload into $a0, the
   0x98 <- *(*(p+0x24)+0x10) chain). The whole fault is the list walk, and it is the same
   wall as its neighbour func_L08_0036EFC8:
     0036F03C  lui   $a1, 0x22          |  lui   $v0, 0x0     (D_L08_00221AE0)
     0036F05C  lw    $v0, 0x1AE0($a1)   |  lw    $v1, 0x0($v0)   <- test
     0036F064  addiu $a1, $a1, 0x1AE0   |  addiu $a1, $v0, 0x0   <- walk pointer
   Retail keeps the %hi in the register and pays the %lo twice (as the first load's
   displacement and in the addiu); here the address is materialised whole, so the load uses
   offset 0. Everything else - the loop body, the `&= 0xFFFC` form, the bnel and its
   delay-slot load - is retail's.

   Measured (116 bytes unless noted): `for (q = D_00221AE0; *q; q++)` with the `&= 0xFFFC`
   body, the `while` and index spellings, two `__asm__`-free names for the same address
   (`D_L08_00221AE0_a/_b`, also 116 and the same 7 words), and the guard written on the
   symbol `if (D_L08_00221AE0[0] != 0)` (124 bytes - it duplicates the test). Holding the
   object pointer in a local per site drops to 112 bytes (wrong). `-G0` is right: both
   resident symbols are absolute in retail. */

extern u8 *D_L08_00152C00;
extern u8 *D_L08_00221AE0[];

/* clears the 0x34 flags and republishes the owner word at +0x98 for the resident object,
   then for every non-null pointer of the 0x221AE0 list */
void func_L08_0036F030(void)
{
    u8 **q;

    if (D_L08_00152C00 != 0) {
        *(u16 *)(D_L08_00152C00 + 0x34) &= 0xFFFC;
        *(s32 *)(D_L08_00152C00 + 0x98) = *(s32 *)(*(s32 *)(D_L08_00152C00 + 0x24) + 0x10);
    }
    for (q = D_L08_00221AE0; *q != 0; q++) {
        *(u16 *)(*q + 0x34) &= 0xFFFC;
        *(s32 *)(*q + 0x98) = *(s32 *)(*(s32 *)(*q + 0x24) + 0x10);
    }
}
