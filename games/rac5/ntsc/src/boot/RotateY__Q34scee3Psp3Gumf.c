/* w12/q30 — RotateY__Q34scee3Psp3Gumf
 * Reconstructed from the retail disassembly (Sony scee::Psp::Gum matrix
 * stack, state block at 0x01F77900).
 * w12/q30: same semantics as the w11 form; the second block is written with
 * the sine product first so the FP temporaries are created in the retail
 * order (f12 first for m0*s).
 */
typedef struct {
    float m[16];
} MATRIX4;

typedef struct {
    char pad0[4];
    int farOffset;
    unsigned char dirty;
    char pad1[7];
    MATRIX4 *current;
} GUM_STATE;

extern GUM_STATE gum;

void RotateY__Q34scee3Psp3Gumf(float angle) {
    MATRIX4 *m = gum.current;

    float c = cosf(angle);
    float s = sinf(angle);
    float m0 = m->m[0], m1 = m->m[1], m2 = m->m[2], m3 = m->m[3];
    float m8 = m->m[8], m9 = m->m[9], m10 = m->m[10], m11 = m->m[11];

    m->m[0] = m0 * c - m8 * s;
    m->m[1] = m1 * c - m9 * s;
    m->m[2] = m2 * c - m10 * s;
    m->m[3] = m3 * c - m11 * s;
    m->m[8] = m0 * s + m8 * c;
    m->m[9] = m1 * s + m9 * c;
    m->m[10] = m2 * s + m10 * c;
    m->m[11] = m3 * s + m11 * c;

    ((unsigned char *)&gum)[gum.farOffset + 8] = 1;
}
