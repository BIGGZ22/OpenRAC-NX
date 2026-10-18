typedef struct {
    float x;
    float y;
    float z;
} VEC3;

typedef struct {
    float m[9];
} MATRIX3;

void VEC_MulMat3Vec3__FP4VEC3PC7MATRIX3PC4VEC3(VEC3 *out, const MATRIX3 *mat, const VEC3 *v) {
    VEC3 t;

    t.x = v->x * mat->m[0] + v->y * mat->m[3] + v->z * mat->m[6];
    t.y = v->x * mat->m[1] + v->y * mat->m[4] + v->z * mat->m[7];
    t.z = v->x * mat->m[2] + v->y * mat->m[5] + v->z * mat->m[8];
    *out = t;
}
