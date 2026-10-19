typedef struct { float m[16]; } MATRIX4;
typedef struct { float x, y, z, w; } QUAT;

extern void Quat_ConvertToMatrixInternal__H1Z7MATRIX4_PX01PC4QUAT_v(MATRIX4 *m, const QUAT *q);

void Quat_ConvertToMatrix__FP7MATRIX4PC4QUAT(MATRIX4 *m, const QUAT *q) {
    Quat_ConvertToMatrixInternal__H1Z7MATRIX4_PX01PC4QUAT_v(m, q);
    {
        float zero = 0.0f;
        float one = 1.0f;

        m->m[15] = one;
        m->m[14] = zero;
        m->m[13] = zero;
        m->m[12] = zero;
        m->m[11] = zero;
        m->m[7] = zero;
        m->m[3] = zero;
    }
}
