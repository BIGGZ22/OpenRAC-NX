/* KillEffect__19CuffLinkBombContentP4Moby_LEVEL19_000D4710 -- LEVEL19.REL 0x000D4710, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   KillEffect__19CuffLinkBombContentP4Moby -- LEVEL19.REL 0x0D4710, 32 bytes.
 *   The bomb explodes at the position stored at offset 0x30 of the moby, and the
 *   moby to explode is the one held at offset 0x0C of the content.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;

typedef struct Moby {
    unsigned char pad_00[0x30];
    VEC3 position;
} Moby;

typedef struct CuffLinkBombContent {
    unsigned char pad_00[0x0C];
    Moby *moby;
} CuffLinkBombContent;

extern void CuffLinkBomb_Explode__FP4MobyPC4VEC3(Moby *moby, VEC3 *position);

void KillEffect__19CuffLinkBombContentP4Moby(CuffLinkBombContent *self,
                                             Moby *moby)
{
    CuffLinkBomb_Explode__FP4MobyPC4VEC3(self->moby, &moby->position);
}
