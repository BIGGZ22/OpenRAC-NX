/* CHEAT_IsActive__FUi_LEVEL10_038C80 -- LEVEL10.REL 0x038C80, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   lui   $v0, 0x25          ; &pGV (0x255030)
 *   addiu $v1, $zero, 0x1
 *   lw    $a1, 0x5030($v0)   ; pGV  (le bas de l'adresse de pGV)
 *   sllv  $v1, $v1, $a0      ; masque 1 << id
 *   lw    $v0, 0x8E0($a1)    ; pGV->drapeaux de triche
 *   and   $v0, $v0, $v1
 *   jr    $ra
 *   sltu  $v0, $zero, $v0    ; (v0 != 0)
 *   Le masque est ecrit dans une variable locale : sans cela le compilateur
 *   transforme (x & (1 << n)) != 0 en (x >> n) & 1 (autre forme de code).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GLOBALVARS GLOBALVARS;

struct GLOBALVARS {
    unsigned char reserved_000[0x8E0];
    unsigned int cheatFlags;      /* 0x8E0 */
};

extern GLOBALVARS *pGV;

int CHEAT_IsActive__FUi(unsigned int cheatId)
{
    unsigned int mask = 1u << cheatId;
    return (pGV->cheatFlags & mask) != 0;
}
