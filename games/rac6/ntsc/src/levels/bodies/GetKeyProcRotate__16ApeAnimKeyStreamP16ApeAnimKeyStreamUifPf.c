/* GetKeyProcRotate__16ApeAnimKeyStreamP16ApeAnimKeyStreamUifPf -- LEVEL13.REL 0x001DFEA0, 132 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   The key stream owns a table of floats at +0x14.  For three steps (the
 *   counter starts at 2) the destination receives
 *   cos(angle) * table[3] + table[0]
 *   and both the table cursor and the destination move on by one float.  The
 *   cosine is recomputed by the three library calls at each step (they are
 *   external: the retail keeps the double in the integer registers).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeAnimKeyStream ApeAnimKeyStream;

extern double fptodp(float);
extern double cos(double);
extern float  dptofp(double);

struct ApeAnimKeyStream {
    char   m_pad00[0x14];
    float *m_table;      /* 0x14 */
};

int GetKeyProcRotate__16ApeAnimKeyStreamP16ApeAnimKeyStreamUifPf(
    ApeAnimKeyStream *self, ApeAnimKeyStream *other, float *dest,
    int x, float angle, void *p, float y)
{
    float *t = self->m_table;
    int    i;

    for (i = 2; i >= 0; i--) {
        float c = (float)cos((double)angle);
        *dest = c * t[3] + t[0];
        t++;
        dest++;
    }
    return 0;
}
