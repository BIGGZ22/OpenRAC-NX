/* GLOBAL_GetFlagBit__FUiUc_LEVEL18_035BB8 -- LEVEL18.REL 0x035BB8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   lui $v0,0x20 ; addiu $a2,$zero,1 ; lw $a2... ; sllv $a2,$a2,$a1
 *   addu $v1,$v1,$a0 ; lbu $v0,0x4E0($v1) ; jr $ra ; and $v0,$v0,$a2
 *   Le tableau des drapeaux est un octet par drapeau, a l'offset 0x4E0 du
 *   bloc global pointe par le pointeur global (0x1FBF30).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned char *g_globalVars;

int GLOBAL_GetFlagBit__FUiUc(unsigned int index, unsigned char bit)
{
    return *(unsigned char *)(g_globalVars + index + 0x4E0) & (1 << bit);
}
