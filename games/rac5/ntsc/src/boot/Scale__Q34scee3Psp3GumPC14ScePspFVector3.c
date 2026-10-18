typedef struct { float m[16]; } MATRIX4;

typedef struct {
    char pad00[4];
    int offset;
    char dirty;
    char pad09[7];
    MATRIX4 *matrix;
} tGumState;

typedef struct { float x, y, z; } VEC3;

extern tGumState gGumState; /* 0x01F77900 */

void Scale__Q34scee3Psp3GumPC14ScePspFVector3(const VEC3 *v) {
    float x = v->x;
    float y = v->y;
    float z = v->z;
    MATRIX4 *m = gGumState.matrix;

    m->m[0] *= x;
    m->m[1] *= x;
    m->m[2] *= x;
    m->m[3] *= x;
    m->m[4] *= y;
    m->m[5] *= y;
    m->m[6] *= y;
    m->m[7] *= y;
    m->m[8] *= z;
    m->m[9] *= z;
    m->m[10] *= z;
    m->m[11] *= z;
    *(char *)((char *)&gGumState + gGumState.offset + 8) = 1;
}
