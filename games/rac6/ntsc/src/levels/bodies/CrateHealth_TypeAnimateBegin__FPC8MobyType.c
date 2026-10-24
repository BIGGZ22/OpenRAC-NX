/* CrateHealth_TypeAnimateBegin__FPC8MobyType -- LEVEL14.REL 0x000EB7F0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   CrateHealth_TypeAnimateBegin__FPC8MobyType - LEVEL14.REL 0x000EB7F0, 32 bytes.
 *   The crate starts the animation of its troupe.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct CrateShared_Troupe CrateShared_Troupe;
typedef struct MobyType MobyType;

extern CrateShared_Troupe D_3DCA70;

extern void CrateShared_TroupeAnimate__FP18CrateShared_Troupe(
    CrateShared_Troupe *troupe);

void CrateHealth_TypeAnimateBegin__FPC8MobyType(const char *name,
                                                MobyType *type)
{
    (void)name;
    (void)type;
    CrateShared_TroupeAnimate__FP18CrateShared_Troupe(&D_3DCA70);
}
