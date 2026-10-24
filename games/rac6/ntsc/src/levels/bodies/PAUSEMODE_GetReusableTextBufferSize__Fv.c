/* PAUSEMODE_GetReusableTextBufferSize__Fv -- LEVEL14.REL 0x0996E0, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   La valeur de retour vit dans l'emplacement de delai du `jr $ra` : d'ou la
 *   variable locale `result` plutot qu'un litteral dans le `return`.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int PAUSEMODE_GetReusableTextBufferSize__Fv(void)
{
    int result = 0x200;
    return result;
}
