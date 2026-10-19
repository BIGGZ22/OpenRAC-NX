typedef struct { float x; float y; float z; } VEC3;

void VEC_SetVec3__FP4VEC3fff(VEC3 *v, float x, float y, float z) {
    v->x = x;
    v->y = y;
    v->z = z;
}
