/* GLOBALVARS_GetPlayerTitaniumBoltCount__Fv_LEVEL1_0373A0 -- LEVEL1.REL 0x0373A0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail: calls GLOBAL_GetFlag__FUiUc(0x37, 0xFF) in a frame that saves $ra.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int GLOBAL_GetFlag__FUiUc(unsigned int flag, unsigned char value);

int GLOBALVARS_GetPlayerTitaniumBoltCount__Fv(void)
{
    return GLOBAL_GetFlag__FUiUc(0x37, 0xFF);
}
