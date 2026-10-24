/* FastMemZero__FPvUi -- LEVEL13.REL 0x035888, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Mise a zero de `count` mots de 32 bits : memset(ptr, 0, count * 4).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void *memset(void *dest, int value, unsigned int size);

void FastMemZero__FPvUi(void *ptr, unsigned int count)
{
    memset(ptr, 0, count * 4);
}
