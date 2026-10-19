typedef struct { float x, y, z, w; } QUAT;

extern void Quat_Slerp__FP4QUATPC4QUATT1f(QUAT *out, const QUAT *a, const QUAT *b, float t);

QUAT *VFPU_Slerp__FP4QUATPC4QUATT1f(QUAT *out, const QUAT *a, const QUAT *b, float t) {
    Quat_Slerp__FP4QUATPC4QUATT1f(out, a, b, t);
    return out;
}
