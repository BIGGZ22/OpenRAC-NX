/* CrateHealth_TypeAnimateBegin__FPC8MobyType_LEVEL19_000D2478 -- LEVEL19.REL 0x000D2478, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   CrateHealth_TypeAnimateBegin__FPC8MobyType -- LEVEL19.REL 0x0D2478, 32 bytes.
 *   The moby type argument is not used: the routine always animates the single
 *   troupe shared by the crates, an anonymous object at 0x3C0310.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct MobyType MobyType;
typedef struct CrateShared_Troupe CrateShared_Troupe;

extern CrateShared_Troupe D_3C0310;

extern void CrateShared_TroupeAnimate__FP18CrateShared_Troupe(
    CrateShared_Troupe *troupe);

void CrateHealth_TypeAnimateBegin__FPC8MobyType(MobyType *type)
{
    (void)type;

    CrateShared_TroupeAnimate__FP18CrateShared_Troupe(&D_3C0310);
}
