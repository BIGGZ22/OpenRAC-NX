/* GLOBALVARS_GetPlayerTitaniumBoltCount__Fv_LEVEL18_0372B0 -- LEVEL18.REL 0x0372B0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   addiu $sp,$sp,-0x10 ; addiu $a0,$zero,0x37 ; sd $ra,0x0($sp)
 *   jal func_035B18 ; addiu $a1,$zero,0xFF ; ld $ra,0x0($sp)
 *   jr $ra ; addiu $sp,$sp,0x10
 *   Le compteur de boulons de titane est lu dans le drapeau 0x37 avec le
 *   masque 0xFF.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_035B18(int flag, int mask);

int GLOBALVARS_GetPlayerTitaniumBoltCount__Fv(void)
{
    return func_035B18(0x37, 0xFF);
}
