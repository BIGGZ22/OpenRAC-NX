/* HUDGENHEALTHBAR_Hide__FUi_LEVEL10_0008EAB0 -- LEVEL10.REL 0x0008EAB0, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La barre de vie de tete est terminee ; l'identifiant de barre est
 *   transmis tel quel.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void HUDGENHEALTHBAR_Terminate__FUi(unsigned int barId);

void HUDGENHEALTHBAR_Hide__FUi(unsigned int barId)
{
    HUDGENHEALTHBAR_Terminate__FUi(barId);
}
