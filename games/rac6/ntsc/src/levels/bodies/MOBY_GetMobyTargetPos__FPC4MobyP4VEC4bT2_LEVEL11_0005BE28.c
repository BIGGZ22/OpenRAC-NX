/* MOBY_GetMobyTargetPos__FPC4MobyP4VEC4bT2_LEVEL11_0005BE28 -- LEVEL11.REL 0x0005BE28, 48 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La position cible du Moby est calculee dans un VEC4 : les trois
 *   coordonnees sont produites par MOBY_GetMobyTargetPos (variante VEC3) et
 *   la quatrieme composante vaut 1.0f (offset 0xC du VEC4).
 *   Le nom retail porte quatre arguments (Moby*, VEC4*, bool, T2) ; le retail
 *   n'initialise que $a0 et $a1, les deux derniers ne sont donc pas utilises.
 *   Preuve : chaine gnu-ee-20261015:-O2 -G0 (egalement exacte sur
 *   gnu-ee-trial-20261015:-O2 -G0), comparaison octet pour octet au retail
 *   au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
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

/* Le retail n'initialise que $a0 et $a1 : declaration sans prototype. */
extern void MOBY_GetMobyTargetPos__FPC4MobyP4VEC3bT2();

void MOBY_GetMobyTargetPos__FPC4MobyP4VEC4bT2(Moby *moby,
                                             VEC4 *pos,
                                             int use_target,
                                             unsigned long long flag)
{
    (void)use_target;
    (void)flag;

    MOBY_GetMobyTargetPos__FPC4MobyP4VEC3bT2(moby, (VEC3 *)pos);

    pos->w = 1.0f;
}
