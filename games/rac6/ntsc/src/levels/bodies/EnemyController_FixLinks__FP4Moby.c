/* EnemyController_FixLinks__FP4Moby -- LEVEL14.REL 0x000F74C0, 36 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   EnemyController_FixLinks__FP4Moby - LEVEL14.REL 0x000F74C0, 36 bytes.
 *   The enemy state block of the moby (at 0x58) is cleared: 0x19C bytes.
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
typedef struct MobyExtra MobyExtra;

struct Moby {
    unsigned char reserved_000[0x58];
    MobyExtra *extra;
};

extern void *memset(void *dst, int value, unsigned int count);

void EnemyController_FixLinks__FP4Moby(Moby *moby)
{
    memset(moby->extra, 0, 0x19C);
}
