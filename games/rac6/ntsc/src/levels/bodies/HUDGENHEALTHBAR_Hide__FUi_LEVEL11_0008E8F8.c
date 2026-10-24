/* HUDGENHEALTHBAR_Hide__FUi_LEVEL11_0008E8F8 -- LEVEL11.REL 0x0008E8F8, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'identifiant de barre de vie reste dans $a0 : il est passe tel quel
 *   a la fonction de terminaison.
 *   Preuve : chaine gnu-ee-20261015:-O2 -G0 (egalement exacte sur
 *   gnu-ee-trial-20261015:-O2 -G0), comparaison octet pour octet au retail
 *   au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void HUDGENHEALTHBAR_Terminate__FUi(unsigned int id);

void HUDGENHEALTHBAR_Hide__FUi(unsigned int id)
{
    HUDGENHEALTHBAR_Terminate__FUi(id);
}
