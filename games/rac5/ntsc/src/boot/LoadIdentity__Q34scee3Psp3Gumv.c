typedef struct { float m[16]; } MATRIX4;

typedef struct {
    char pad00[4];
    int offset;
    char dirty;
    char pad09[7];
    MATRIX4 *matrix;
} tGumState;

extern tGumState gGumState;

void LoadIdentity__Q34scee3Psp3Gumv(void) {
    MATRIX4 *m = gGumState.matrix;
    m->m[0] = 1.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    m->m[4] = 0.0f;
    m->m[5] = 1.0f;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = 1.0f;
    m->m[11] = 0.0f;
    m->m[12] = 0.0f;
    m->m[13] = 0.0f;
    m->m[14] = 0.0f;
    m->m[15] = 1.0f;
    *(char *)((char *)&gGumState + gGumState.offset + 8) = 1;
}
