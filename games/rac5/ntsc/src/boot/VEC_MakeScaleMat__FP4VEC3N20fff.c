typedef struct {
    float x;
    float y;
    float z;
} VEC3;

void VEC_MakeScaleMat__FP4VEC3N20fff(VEC3 *a, VEC3 *b, VEC3 *c, float x, float y, float z) {
    a->x = x;
    a->y = 0.0f;
    a->z = 0.0f;
    b->x = 0.0f;
    b->y = y;
    b->z = 0.0f;
    c->x = 0.0f;
    c->y = 0.0f;
    c->z = z;
}
