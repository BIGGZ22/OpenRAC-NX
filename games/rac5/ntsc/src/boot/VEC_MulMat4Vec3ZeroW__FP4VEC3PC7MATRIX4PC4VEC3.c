typedef struct {
    float x;
    float y;
    float z;
} VEC3;

typedef struct {
    float m[16];
} MATRIX4;

void VEC_MulMat4Vec3ZeroW__FP4VEC3PC7MATRIX4PC4VEC3(VEC3 *out, const MATRIX4 *mat,
                                                   const VEC3 *v) {
    VEC3 t;

    t.x = v->x * mat->m[0] + v->y * mat->m[4] + v->z * mat->m[8];
    t.y = v->x * mat->m[1] + v->y * mat->m[5] + v->z * mat->m[9];
    t.z = v->x * mat->m[2] + v->y * mat->m[6] + v->z * mat->m[10];
    *out = t;
}
