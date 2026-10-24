/* MEMCARDCLIENT_ClearError__Fv -- FE.REL 0x066550, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   MEMCARDCLIENT_ClearError__Fv  (FE.REL 0x066550, 32 bytes)
 *   Retail:  addiu $sp,-0x10 / li $a0,0x10 / sd $ra / jal func_066548
 *   daddu $a1,$zero,$zero / ld $ra / jr $ra / addiu $sp,0x10
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_066548(int mask, void *param);

void MEMCARDCLIENT_ClearError__Fv(void)
{
    func_066548(0x10, 0);
}
