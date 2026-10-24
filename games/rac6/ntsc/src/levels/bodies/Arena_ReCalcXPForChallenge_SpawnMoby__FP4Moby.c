/* Arena_ReCalcXPForChallenge_SpawnMoby__FP4Moby -- LEVEL14.REL 0x040BA0, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   64 octets.
 *   La table des signatures est indexee par un octet : la multiplication
 *   par 0x34 (taille de l'entree) est faite par le mult a trois operands
 *   de l'EE.
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
typedef struct CHALLENGE CHALLENGE;
typedef struct ARENA_SIG ARENA_SIG;

struct ARENA_SIG {
    unsigned char reserved_000[0x20];
    unsigned int value;
    unsigned char reserved_024[0x10];
};

struct CHALLENGE {
    unsigned int reserved_000;
    ARENA_SIG *sigs;
    unsigned int reserved_008[10];
    unsigned int level_id;
};

struct Moby {
    unsigned char reserved_000[0x40];
    CHALLENGE *challenge;
    unsigned char index;
};

extern void Arena_ReCalcXpForChallenge_FromSigs__FUiUi(unsigned int a,
                                                        unsigned int b);

void Arena_ReCalcXPForChallenge_SpawnMoby__FP4Moby(Moby *moby)
{
    if (moby && moby->challenge) {
        Arena_ReCalcXpForChallenge_FromSigs__FUiUi(
            moby->challenge->level_id,
            moby->challenge->sigs[moby->index].value);
    }
}
