/* LEVEL14_017B00 -- LEVEL14.REL 0x017B00, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Trois constantes flottantes sont passees en $f12, $f13, $f14 et
 *   l'adresse d'un objet global (0x345168) en $a0.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int D_00345168;

extern void func_1CE770(void *object, float a, float b, float c);

void func_017B00(void)
{
    func_1CE770((void *)&D_00345168, 62.831852f, 20.0f, 0.033333335f);
}
