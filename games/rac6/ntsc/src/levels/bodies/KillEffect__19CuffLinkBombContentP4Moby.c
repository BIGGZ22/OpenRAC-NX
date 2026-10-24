/* KillEffect__19CuffLinkBombContentP4Moby -- LEVEL10.REL 0x000CE4D8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le contenu de la bombe fait exploser le Moby stocke en 0x0C, a la position
 *   (offset 0x30) du Moby passe en second argument.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Moby Moby;
typedef struct VEC3 {
    float x;
    float y;
    float z;
} VEC3;
typedef struct CuffLinkBombContent CuffLinkBombContent;

struct Moby {
    unsigned char unknown_000[0x30];
    VEC3 pos;
};

struct CuffLinkBombContent {
    VEC3 unknown_000;
    Moby *bomb;
};

extern void CuffLinkBomb_Explode__FP4MobyPC4VEC3(Moby *moby, VEC3 *pos);

void KillEffect__19CuffLinkBombContentP4Moby(CuffLinkBombContent *self, Moby *moby)
{
    CuffLinkBomb_Explode__FP4MobyPC4VEC3(self->bomb, &moby->pos);
}
