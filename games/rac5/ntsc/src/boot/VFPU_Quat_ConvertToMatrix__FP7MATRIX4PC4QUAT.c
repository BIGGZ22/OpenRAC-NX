typedef struct { float m[16]; } MATRIX4;
typedef struct { float x, y, z, w; } QUAT;

extern void Quat_ConvertToMatrix__FP7MATRIX4PC4QUAT(MATRIX4 *m, const QUAT *q);

MATRIX4 *VFPU_Quat_ConvertToMatrix__FP7MATRIX4PC4QUAT(MATRIX4 *m, const QUAT *q) {
    Quat_ConvertToMatrix__FP7MATRIX4PC4QUAT(m, q);
    return m;
}
