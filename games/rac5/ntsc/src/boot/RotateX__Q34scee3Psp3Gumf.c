/* w11/b01 — RotateX__Q34scee3Psp3Gumf
 * Reconstructed from the retail disassembly (Sony scee::Psp::Gum matrix
 * stack).  The state block is at 0x01F77900: +0x04 far offset, +0x08 dirty
 * flag (through the far offset), +0x10 current matrix.
 */
typedef struct {
    float m[16];
} MATRIX4;

typedef struct {
    char pad0[4];
    int farOffset;          /* 0x04 */
    unsigned char dirty;    /* 0x08 (reached through farOffset) */
    char pad1[7];
    MATRIX4 *current;       /* 0x10 */
} GUM_STATE;

extern GUM_STATE gum;       /* 0x01F77900 */

void RotateX__Q34scee3Psp3Gumf(float angle) {
    MATRIX4 *m = gum.current;

    float c = cosf(angle);
    float s = sinf(angle);
    float m4 = m->m[4], m5 = m->m[5], m6 = m->m[6], m7 = m->m[7];
    float m8 = m->m[8], m9 = m->m[9], m10 = m->m[10], m11 = m->m[11];

    m->m[4] = m4 * c + m8 * s;
    m->m[5] = m5 * c + m9 * s;
    m->m[6] = m6 * c + m10 * s;
    m->m[7] = m7 * c + m11 * s;
    m->m[8] = m8 * c - m4 * s;
    m->m[9] = m9 * c - m5 * s;
    m->m[10] = m10 * c - m6 * s;
    m->m[11] = m11 * c - m7 * s;

    ((unsigned char *)&gum)[gum.farOffset + 8] = 1;
}
