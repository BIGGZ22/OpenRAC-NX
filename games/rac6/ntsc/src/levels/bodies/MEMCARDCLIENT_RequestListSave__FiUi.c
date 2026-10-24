/* MEMCARDCLIENT_RequestListSave__FiUi -- FE.REL 0x066D60, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   MEMCARDCLIENT_RequestListSave__FiUi  (FE.REL 0x066D60, 32 bytes)
 *   Retail:  addiu $sp,-0x10 / sd $ra / jal func_0096E0 / nop
 *   ld $ra / daddu $v0,$zero,$zero / jr $ra / addiu $sp,0x10
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_0096E0(void);

int MEMCARDCLIENT_RequestListSave__FiUi(int param1, unsigned int param2)
{
    func_0096E0();
    return 0;
}
