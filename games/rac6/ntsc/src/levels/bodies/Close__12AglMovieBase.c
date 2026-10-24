/* Close__12AglMovieBase -- FE.REL 0x003CB8, 64 bytes.
 *
 * Closes the Bink stream of a movie object and puts the object in state 2.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglMovieBase AglMovieBase;

struct AglMovieBase {
    unsigned char pad[0x100];
    void *m_bink;
    int m_state;
};

extern void BinkClose(void *buffer);

void Close(AglMovieBase *self)
{
    if (self->m_bink != 0) {
        BinkClose(self->m_bink);
        self->m_bink = 0;
    }
    self->m_state = 2;
}
