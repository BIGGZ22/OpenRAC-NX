/*
 func_L06_003E3ED0 (88 o) — reste 8 octets : if-conversion du couple return 0/1.

   Le retail garde une BRANCHE explicite pour le corps :
     003E3EF4 lw  $v0,0x588($a1)
     003E3EF8 bne $v0,$a0, -> épilogue partagé
     003E3EFC daddu $v0,$zero,$zero      (slot de délai)
     003E3F00 jr  $ra
     003E3F04 addiu $v0,$zero,0x1        (slot de délai)
   Notre chaîne replie ce `if (x != a0) return 0; return 1;` en
     `lw v0,0x588 ; xor v0,v0,a0 ; jr $ra ; sltiu v0,v0,1`
   => 80 o au lieu de 88. Essayé sans succès : les deux polarités
   (`!= → return 0; return 1;` et `== → return 1; return 0;`), `||` simple,
   et les blocs dupliqués (cross-jump) : la fusion revient chaque fois.
   Les autres blocs (tests 0x229C/0x2294/0x34E/0x33C, épilogue partagé) sont
   reproduits correctement ; seul ce couple 0/1 est replié.
   Mur « if-conversion » du BRIEF.

   Dernière ligne de check.py :
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
