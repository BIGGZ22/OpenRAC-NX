typedef struct {
    float x;
    float y;
    float z;
} VEC3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} VEC4;

typedef struct {
    VEC4 x;
    VEC4 y;
    VEC4 z;
    VEC4 w;
} MATRIX4ROWS;

typedef struct {
    int pad0;
    int offset;
    unsigned char dirty;
    unsigned char pad1[7];
    MATRIX4ROWS *matrix;
} GU_CONTEXT;

extern GU_CONTEXT gGumState;

void Translate__Q34scee3Psp3GumPC14ScePspFVector3(const VEC3 *v) {
    MATRIX4ROWS *m = gGumState.matrix;
    float vx = v->x;
    float vy = v->y;
    float vz = v->z;

    m->w.x = m->x.x * vx + m->y.x * vy + m->z.x * vz + m->w.x;
    m->w.y = m->x.y * vx + m->y.y * vy + m->z.y * vz + m->w.y;
    m->w.z = m->x.z * vx + m->y.z * vy + m->z.z * vz + m->w.z;
    m->w.w = m->x.w * vx + m->y.w * vy + m->z.w * vz + m->w.w;
    *(unsigned char *)((char *)&gGumState + gGumState.offset + 8) = 1;
}
