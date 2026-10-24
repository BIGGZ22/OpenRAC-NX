/* FireSnake_Term__FP4Moby -- LEVEL14.REL 0x000FAC08, 48 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   FireSnake_Term__FP4Moby - LEVEL14.REL 0x000FAC08, 48 bytes.
 *   The line group of the moby is stopped (0x10C of the object at 0x58) and
 *   its sound is stopped (0x2C4 of the same object).
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
typedef struct Line_Group Line_Group;
typedef struct MobyExtra MobyExtra;

struct MobyExtra {
    unsigned char reserved_000[0x10C];
    Line_Group *line_group;
    unsigned char reserved_110[0x1B4];
    unsigned int sound_id;
};

struct Moby {
    unsigned char reserved_000[0x58];
    MobyExtra *extra;
};

extern void Line_Stop__FP10Line_Group(Line_Group *group);
extern void MOBY_StopSound__FUi(unsigned int sound);

void FireSnake_Term__FP4Moby(Moby *moby)
{
    MobyExtra *extra = moby->extra;

    Line_Stop__FP10Line_Group((Line_Group *)((char *)extra + 0x10C));
    MOBY_StopSound__FUi(extra->sound_id);
}
