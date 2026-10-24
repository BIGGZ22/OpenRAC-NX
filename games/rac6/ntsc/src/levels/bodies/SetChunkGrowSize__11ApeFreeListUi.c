/* SetChunkGrowSize__11ApeFreeListUi -- FE.REL 0x09A290, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   SetChunkGrowSize__11ApeFreeListUi  (FE.REL 0x09A290, 8 bytes)
 *   Retail:  jr $ra / sh $a1, 0x30($a0)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeFreeList ApeFreeList;

struct ApeFreeList
{
    char pad_00[0x30];
    unsigned short chunkGrowSize;
};

void SetChunkGrowSize__11ApeFreeListUi(ApeFreeList *self, unsigned int size)
{
    self->chunkGrowSize = size;
}
