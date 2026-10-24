/* CONTROLLER_GetWorldButtonLeft__FUi -- LEVEL13.REL 0x04D8D8, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'etat des boutons du monde tient dans un mot : le bouton gauche est le
 *   bit de poids faible. La valeur est posee dans le slot de delai du jr.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

unsigned int CONTROLLER_GetWorldButtonLeft__FUi(unsigned int buttons)
{
    return buttons & 1;
}
