/* MOBY_GetMobyTargetPos__FPC4MobyP4VEC4bT2 -- LEVEL10.REL 0x0005BE60, 48 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La position visee du Moby est ecrite dans les trois premiers flottants du
 *   VEC4 ; le quatrieme composante vaut 1.0.
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
typedef struct VEC4 {
    float x;
    float y;
    float z;
    float w;
} VEC4;

extern void MOBY_GetMobyTargetPos__FPC4MobyPC4VEC3bT2(Moby *moby, VEC3 *pos,
                                                     int flag);

void MOBY_GetMobyTargetPos__FPC4MobyP4VEC4bT2(Moby *moby, VEC4 *out, int flag)
{
    MOBY_GetMobyTargetPos__FPC4MobyPC4VEC3bT2(moby, (VEC3 *)out, flag);
    out->w = 1.0f;
}
