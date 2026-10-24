/* GLOBAL_GetFlagBitU32__FUiUi_LEVEL18_035C98 -- LEVEL18.REL 0x035C98, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   lui $v0,0x20 ; addiu $v1,$zero,1 ; lw $a2,-0x40D0($v0)
 *   sllv $v1,$v1,$a1 ; addu $a0,$a0,$a2 ; lw $v0,0x4E0($a0)
 *   jr $ra ; and $v0,$v0,$v1
 *   Meme tableau de drapeaux que GLOBAL_GetFlagBit, mais lu sur 32 bits.
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

int GLOBAL_GetFlagBitU32__FUiUi(unsigned int index, unsigned int bit)
{
    return *(unsigned int *)(g_globalVars + (index + 0x4E0)) & (1 << bit);
}
