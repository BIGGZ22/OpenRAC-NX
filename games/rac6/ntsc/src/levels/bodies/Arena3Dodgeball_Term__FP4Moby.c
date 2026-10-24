/* Arena3Dodgeball_Term__FP4Moby -- LEVEL14.REL 0x000BA600, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Arena3Dodgeball_Term__FP4Moby - LEVEL14.REL 0x000BA600, 32 bytes.
 *   The PAR group of the moby lives 0x14 bytes into the object the moby
 *   points to at 0x58; the arithmetic is a byte offset, so the load is a
 *   single word and the displacement an add of the constant.
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
    unsigned char reserved_000[0x14];
    PAR_Group *par_group;
};

struct Moby {
    unsigned char reserved_000[0x58];
    MobyExtra *extra;
};

extern void PAR_DestroyGroup__FP9PAR_Group(PAR_Group *group);

void Arena3Dodgeball_Term__FP4Moby(Moby *moby)
{
    PAR_DestroyGroup__FP9PAR_Group((PAR_Group *)((char *)moby->extra + 0x14));
}
