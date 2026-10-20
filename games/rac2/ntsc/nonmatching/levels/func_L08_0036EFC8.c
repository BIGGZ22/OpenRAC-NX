/* func_L08_0036EFC8 (100 bytes, 2 places) -- NEAR MISS, not integrated.
   state: 100 bytes and every retail instruction present; 20 of 25 words identical.
   check.py (08_tabora), with the source below:
     0036EFC8  lui   $a1, 0x15             |  lui   $a0, 0x0      (D_L08_00152C00, %hi)
     0036EFCC  lw    $v1, 0x2C00($a1)      |  lw    $v1, 0x0($a0)
     0036EFEC  lw    $v0, 0x1AE0($a0) <-   |  the array base is materialised whole
     0036EFF4  addiu $a1, $a0, 0x1AE0      |  into $v0, so the test loads 0($v0) and
     0036F010  addiu $a1, $a1, 0x4         |  the pointer is built `addiu $a1,$v0,0`
   What retail does: the two resident addresses are split hi/lo (`lui $a0,0x22` lands in
   the guard's delay slot, the %lo stays in the first load's displacement and comes back
   as `addiu $a1,$a0,0x1AE0` for the walk), and the short-lived 0x00152C00 base is the one
   that gets $a1. Here the array base gets its own register ($v0), which also moves the
   branch targets and the reload's register ($a0 inside the loop is retail's too, so the
   loop itself is right).

   Measured, same 100 bytes unless noted: pointer-walk `for (q = D_00221AE0; *q; q++)`,
   `while (*q) { ...; q++; }`, guard-then-`do {} while (*++q)`, the index form
   (`for (i = 0; D_00221AE0[i]; i++)`), the pointer taken before the first block, and the
   four literal-address spellings (`*(u8 **)0x00152C00` / `(u8 **)0x00221AE0`, 108 bytes -
   a constant address costs one more instruction). Naming the array in the guard instead
   (`if (D_00221AE0[0] != 0)`) matches retail's preheader load but duplicates the test:
   108 bytes. `-G8` is not it either: both symbols are absolute in retail, so the file's
   default `-O2 -G0 -ffunction-sections` profile is the right one.
   Next idea: a lever that keeps the array address symbolic for the preheader test while
   the walk's pointer is still built from the %hi register (a `__asm__` alias on the same
   address, one name per site, is the shape that has not been tried here). */

extern u8 *D_L08_00152C00;
extern u8 *D_L08_00221AE0[];

/* sets the +0x34 flags and clears the +0x98 word of the object behind the resident
   pointer, then does the same for every non-null pointer of the 0x221AE0 list */
void func_L08_0036EFC8(void)
{
    u8 **q;

    if (D_L08_00152C00 != 0) {
        *(u16 *)(D_L08_00152C00 + 0x34) |= 3;
        *(s32 *)(D_L08_00152C00 + 0x98) = 0;
    }
    for (q = D_L08_00221AE0; *q != 0; q++) {
        *(u16 *)(*q + 0x34) |= 3;
        *(s32 *)(*q + 0x98) = 0;
    }
}
