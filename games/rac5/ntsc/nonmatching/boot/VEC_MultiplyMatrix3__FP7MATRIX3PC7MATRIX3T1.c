typedef struct {
    float m[9];
} MATRIX3;

void VEC_MultiplyMatrix3__FP7MATRIX3PC7MATRIX3T1(MATRIX3 *out, const MATRIX3 *a, const MATRIX3 *b) {
    float m0, m1, m2, m3, m4, m5, m6, m7, m8;

    m0 = a->m[0] * b->m[0] + a->m[1] * b->m[3] + a->m[2] * b->m[6];
    m1 = a->m[0] * b->m[1] + a->m[1] * b->m[4] + a->m[2] * b->m[7];
    m2 = a->m[0] * b->m[2] + a->m[1] * b->m[5] + a->m[2] * b->m[8];
    m3 = a->m[3] * b->m[0] + a->m[4] * b->m[3] + a->m[5] * b->m[6];
    m4 = a->m[3] * b->m[1] + a->m[4] * b->m[4] + a->m[5] * b->m[7];
    m5 = a->m[3] * b->m[2] + a->m[4] * b->m[5] + a->m[5] * b->m[8];
    m6 = a->m[6] * b->m[0] + a->m[7] * b->m[3] + a->m[8] * b->m[6];
    m7 = a->m[6] * b->m[1] + a->m[7] * b->m[4] + a->m[8] * b->m[7];
    m8 = a->m[6] * b->m[2] + a->m[7] * b->m[5] + a->m[8] * b->m[8];

    out->m[0] = m0;
    out->m[1] = m1;
    out->m[2] = m2;
    out->m[3] = m3;
    out->m[4] = m4;
    out->m[5] = m5;
    out->m[6] = m6;
    out->m[7] = m7;
    out->m[8] = m8;
}
