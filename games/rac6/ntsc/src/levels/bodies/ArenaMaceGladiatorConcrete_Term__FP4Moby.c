/* ArenaMaceGladiatorConcrete_Term__FP4Moby -- LEVEL14.REL 0x000CFD10, 48 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ArenaMaceGladiatorConcrete_Term__FP4Moby - LEVEL14.REL 0x000CFD10, 48 bytes.
 *   Two PAR groups of the moby are destroyed: the ones at 0x68 and 0x84 of
 *   the object the moby points to at 0x58.
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
typedef struct PAR_Group PAR_Group;
typedef struct MobyExtra MobyExtra;

struct MobyExtra {
    unsigned char reserved_000[0x68];
    PAR_Group *par_group_068;
    unsigned char reserved_06C[0x18];
    PAR_Group *par_group_084;
};

struct Moby {
    unsigned char reserved_000[0x58];
    MobyExtra *extra;
};

extern void PAR_DestroyGroup__FP9PAR_Group(PAR_Group *group);

void ArenaMaceGladiatorConcrete_Term__FP4Moby(Moby *moby)
{
    MobyExtra *extra = moby->extra;

    PAR_DestroyGroup__FP9PAR_Group((PAR_Group *)((char *)extra + 0x68));
    PAR_DestroyGroup__FP9PAR_Group((PAR_Group *)((char *)extra + 0x84));
}
