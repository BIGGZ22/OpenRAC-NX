/* Shutdown__11ApeSkeleton -- FE.REL 0x000BD3A8, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'arret du squelette est celui de sa classe de definition ; le pointeur
 *   passe en $a0 n'est pas modifie.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeSkeleton ApeSkeleton;

extern void Shutdown__14ApeSkeletonDef(void *self);

void Shutdown__11ApeSkeleton(ApeSkeleton *self)
{
    Shutdown__14ApeSkeletonDef(self);
}
