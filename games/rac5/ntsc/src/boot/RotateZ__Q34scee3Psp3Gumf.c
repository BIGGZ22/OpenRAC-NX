/* w11/b01 — RotateZ__Q34scee3Psp3Gumf
 * Reconstructed from the retail disassembly (Sony scee::Psp::Gum matrix
 * stack, state block at 0x01F77900).
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

void RotateZ__Q34scee3Psp3Gumf(float angle) {
    MATRIX4 *m = gum.current;

    float c = cosf(angle);
    float s = sinf(angle);
    float m0 = m->m[0], m1 = m->m[1], m2 = m->m[2], m3 = m->m[3];
    float m4 = m->m[4], m5 = m->m[5], m6 = m->m[6], m7 = m->m[7];

    m->m[0] = m0 * c + m4 * s;
    m->m[1] = m1 * c + m5 * s;
    m->m[2] = m2 * c + m6 * s;
    m->m[3] = m3 * c + m7 * s;
    m->m[4] = m4 * c - m0 * s;
    m->m[5] = m5 * c - m1 * s;
    m->m[6] = m6 * c - m2 * s;
    m->m[7] = m7 * c - m3 * s;

    ((unsigned char *)&gum)[gum.farOffset + 8] = 1;
}
