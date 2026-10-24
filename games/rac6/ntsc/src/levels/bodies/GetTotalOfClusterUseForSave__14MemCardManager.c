/* GetTotalOfClusterUseForSave__14MemCardManager -- FE.REL 0x026F58, 8 bytes.
 *
 * Returns the fixed number of clusters the save uses.
 *
 * Evidence:
 *   La methode rend la constante 0x2CA.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct MemCardManager MemCardManager;

unsigned int GetTotalOfClusterUseForSave(MemCardManager *self)
{
    (void)self;
    return 0x2CA;
}
