/*
 func_L06_003E3ED0 (88 bytes) — 8 bytes left: if-conversion of the return 0/1 pair.

   The retail keeps an explicit BRANCH for the body:
     003E3EF4 lw  $v0,0x588($a1)
     003E3EF8 bne $v0,$a0, -> shared epilogue
     003E3EFC daddu $v0,$zero,$zero      (delay slot)
     003E3F00 jr  $ra
     003E3F04 addiu $v0,$zero,0x1        (delay slot)
   Our chain folds this `if (x != a0) return 0; return 1;` into
     `lw v0,0x588 ; xor v0,v0,a0 ; jr $ra ; sltiu v0,v0,1`
   => 80 bytes instead of 88. Tried without success: both polarities
   (`!= → return 0; return 1;` and `== → return 1; return 0;`), plain `||`,
   and the duplicated blocks (cross-jump): the merge comes back every time.
   The other blocks (tests 0x229C/0x2294/0x34E/0x33C, shared epilogue) are
   reproduced correctly; only this 0/1 pair is folded.
   "if-conversion" wall of the BRIEF.

   last check.py line:
     func_L06_003E3ED0: 0 of 4 places match (src/levels/shared/L06_0035EA68.c)
     06_notak: 80 bytes, expected 88
*/

typedef struct {
    u8 pad00[0x33C]; s32 field33C;
    u8 pad340[0x34E - 0x340]; s16 field34E;
    u8 pad350[0x588 - 0x350]; s32 field588;
    u8 pad58C[0x2294 - 0x58C]; s32 field2294; s32 field2298; s32 field229C;
} D00189E20;
extern D00189E20 D_00189E20;

s32 func_L06_003E3ED0(s32 a0) {
    if (D_00189E20.field229C == 3 || D_00189E20.field2294 == 0x1B) {
        if (D_00189E20.field588 == a0)
            return 1;
        return 0;
    }
    if (D_00189E20.field34E != 0)
        return 0;
    if (D_00189E20.field33C == a0)
        return 1;
    return 0;
}
