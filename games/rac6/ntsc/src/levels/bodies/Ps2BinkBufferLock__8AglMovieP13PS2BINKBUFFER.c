/* Ps2BinkBufferLock__8AglMovieP13PS2BINKBUFFER -- FE.REL 0x003FD8, 8 bytes.
 *
 * Answers 1 without touching the buffer.
 *
 * Evidence:
 *   La methode rend 1 sans toucher a ses arguments.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglMovie AglMovie;
typedef struct PS2BINKBUFFER PS2BINKBUFFER;

int Ps2BinkBufferLock(AglMovie *self, PS2BINKBUFFER *buffer)
{
    (void)self;
    (void)buffer;
    return 1;
}
