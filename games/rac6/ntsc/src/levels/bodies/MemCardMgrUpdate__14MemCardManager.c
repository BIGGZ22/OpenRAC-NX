/* MemCardMgrUpdate__14MemCardManager -- LEVEL14.REL 0x00076A40, 44 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   MemCardMgrUpdate__14MemCardManager - LEVEL14.REL 0x00076A40
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

extern void UpdateLogicsState__14MemCardManager(MemCardManager *self);
extern void UpdateSync__14MemCardManager(MemCardManager *self);

void MemCardMgrUpdate__14MemCardManager(MemCardManager *self)
{
    UpdateLogicsState__14MemCardManager(self);
    UpdateSync__14MemCardManager(self);
}
