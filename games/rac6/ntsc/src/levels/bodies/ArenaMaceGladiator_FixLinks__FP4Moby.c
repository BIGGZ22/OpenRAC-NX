/* ArenaMaceGladiator_FixLinks__FP4Moby -- LEVEL14.REL 0x000CD540, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ArenaMaceGladiator_FixLinks__FP4Moby - LEVEL14.REL 0x000CD540, 28 bytes.
 *   The navigate object of the moby is at 0x54.
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
typedef struct Navigate Navigate;

struct Moby {
    unsigned char reserved_000[0x54];
    Navigate *navigate;
};

extern void Navigate_FixLinks__FP8Navigate(Navigate *navigate);

void ArenaMaceGladiator_FixLinks__FP4Moby(Moby *moby)
{
    Navigate_FixLinks__FP8Navigate(moby->navigate);
}
