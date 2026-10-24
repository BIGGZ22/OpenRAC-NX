/* BoltThief_Update__FP4Mobyf -- LEVEL19.REL 0x000BF450, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   BoltThief_Update__FP4Mobyf -- LEVEL19.REL 0x0BF450, 40 bytes.
 *   The thief state machine is the object at offset 0x58 of the moby; the flag
 *   at its offset 0xC4 is raised and the state machine embedded at its offset
 *   0xD0 is updated with the elapsed time.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct StateMachine StateMachine;

typedef struct BoltThiefState {
    unsigned char pad_00[0xC4];
    unsigned int flag;
    unsigned char pad_C8[0x08];
} BoltThiefState;

typedef struct Moby {
    unsigned char pad_00[0x58];
    BoltThiefState *state;
} Moby;

extern void Update__12StateMachinef(StateMachine *machine, float elapsed);

void BoltThief_Update__FP4Mobyf(Moby *moby, float elapsed)
{
    BoltThiefState *state = moby->state;

    state->flag = 1;
    Update__12StateMachinef((StateMachine *)((char *)state + 0xD0), elapsed);
}
