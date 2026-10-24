/* AnimTranslator_GetFrameTrans__FP14AnimTranslator -- LEVEL13.REL 0x04C340, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   (8 octets)
 *   La methode renvoie l'adresse du bloc de transformation situe a 0x18 dans
 *   le traducteur.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AnimTranslator AnimTranslator;

void *AnimTranslator_GetFrameTrans__FP14AnimTranslator(AnimTranslator *self)
{
    return (void *)((unsigned char *)self + 0x18);
}
