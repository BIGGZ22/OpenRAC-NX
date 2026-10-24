/* GetNumFree__C11ApeFreeList -- FE.REL 0x09A530, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GetNumFree__C11ApeFreeList  (FE.REL 0x09A530, 8 bytes)
 *   Retail:  jr $ra / lw $v0, 0x14($a0)
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
    char pad_00[0x14];
    int numFree;
    char pad_18[0x10];
};

int GetNumFree__C11ApeFreeList(const ApeFreeList *self)
{
    return self->numFree;
}
