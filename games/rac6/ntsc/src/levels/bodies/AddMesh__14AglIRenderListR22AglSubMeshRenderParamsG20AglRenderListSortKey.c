/* AddMesh__14AglIRenderListR22AglSubMeshRenderParamsG20AglRenderListSortKey -- FE.REL 0x0799F8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   AddMesh__14AglIRenderListR22AglSubMeshRenderParamsG20AglRenderListSortKey
 *   (FE.REL 0x0799F8, 32 bytes)
 *   Retail:  lw $v0,4($a0) / sw $a2,0($v0) / lw $v1,4($a0) / sw $a1,4($v1)
 *   lw $v0,4($a0) / addiu $v0,$v0,8 / jr $ra / sw $v0,4($a0)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglRenderListSortKey AglRenderListSortKey;
typedef struct AglSubMeshRenderParams AglSubMeshRenderParams;
typedef struct AglRenderListEntry AglRenderListEntry;
typedef struct AglIRenderList AglIRenderList;

struct AglRenderListEntry
{
    void *sortKey;
    void *params;
};

struct AglIRenderList
{
    void *unknown;
    AglRenderListEntry *entries;
};

void AddMesh__14AglIRenderListR22AglSubMeshRenderParamsG20AglRenderListSortKey(
    AglIRenderList *self,
    AglSubMeshRenderParams *params,
    AglRenderListSortKey *sortKey)
{
    self->entries->sortKey = sortKey;
    self->entries->params = params;
    self->entries = self->entries + 1;
}
