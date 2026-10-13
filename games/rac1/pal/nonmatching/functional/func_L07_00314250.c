/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L07_00312e70), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: its PAL file declares a symbol it uses with another type, so it was not built there. */
typedef u32 u128 __attribute__((mode(TI), aligned(16)));
typedef union {
    u128 q;
    f32 f[4];
    s32 i[4];
} Vec4_14250;
struct GeometryQuad {
    Vec4_14250 positions[4]; /* 0x00 */
    u32 colors[4]; /* 0x40 */
    f32 texture_coordinates[4][2]; /* 0x50 */
    union {
        u64 value;
        volatile u64 ordered;
    } reserved; /* 0x70 */
    u64 texture; /* 0x78 */
    u64 texture_state; /* 0x80 */
    u64 primitive; /* 0x88 */
};
extern s32 D_L07_0015F6B0 MACRO_ADDR;
extern float func_001FA888(int);
extern int func_001FA898(float);
extern int func_001F9850(int);
extern u64 func_001F4868(int);
extern void func_L00_001FD1D8(void *, int, int);
extern float func_L00_0025F368(float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l07/gameplay/entities/00312948.c, FUN_L07_00312e70. */
void func_L07_00314250(char *moby, float radius, int phase, float speed, int color) {
    char *vars = *(char **)(moby + 0x78);
    struct GeometryQuad quad;
    struct GeometryQuad inner;
    float offset[4];
    float spin;
    float step;
    float angle;
    float wrapped;
    u64 texture;

    spin = func_001FA888(D_L07_0015F6B0 + phase) * speed;
    if (*(int *)(vars + 0x164) == 4) {
        float alpha = func_001FA888(color >> 24);
        float fade = func_001FA888(*(int *)(vars + 0x160));
        fade = fade / func_001F9850(20);
        alpha = alpha * fade;
        if (alpha < 0.0f)
            alpha = 0.0f;
        color &= 0xFFFFFF;
        color |= func_001FA898(alpha) << 24;
    } else if (*(int *)(vars + 0x164) > 4) {
        return;
    }
    step = 0.5235988f;
    texture = func_001F4868(0x3D);
    {
        float zero = 0.0f;
        float one = 1.0f;
        float half = 0.5f;

        quad.colors[3] = color;
        quad.colors[2] = color;
        quad.colors[1] = color;
        quad.colors[0] = color;
        quad.texture = texture;
        quad.texture_state = 0xFF9000000260LL;
        quad.reserved.value = 4;
        quad.primitive = 0x8000000048LL;
        quad.texture_coordinates[0][0] = zero;
        quad.texture_coordinates[2][0] = zero;
        quad.texture_coordinates[1][0] = one;
        quad.texture_coordinates[3][0] = one;
        quad.texture_coordinates[0][1] = zero;
        quad.texture_coordinates[1][1] = zero;
        quad.texture_coordinates[2][1] = half;
        quad.texture_coordinates[3][1] = half;
    }
    qcopy(&quad.positions[1], vars + 0x110);
    qcopy(&quad.positions[3], vars + 0x140);
    inner = quad;
    qcopy(&inner.positions[0], vars + 0x120);
    qcopy(&inner.positions[1], vars + 0x120);
    for (angle = -3.1415927f; angle < step + 3.1415927f; angle += step) {
        qcopy(&quad.positions[0], &quad.positions[1]);
        qcopy(&quad.positions[2], &quad.positions[3]);
        wrapped = func_L00_0025F368(angle + spin * 0.02f);
        offset[1] = radius * func_001F9F90(wrapped);
        offset[2] = radius * func_001F9FA8(wrapped);
        offset[0] = 0.0f;
        func_001F9EC0(offset, offset, moby + 0xC0);
        func_001F9BD8(&quad.positions[3], vars + 0x140, offset);
        if (angle != -3.1415927f) {
            func_L00_001FD1D8(&quad, 0, 1);
            qcopy(&inner.positions[2], &quad.positions[2]);
            qcopy(&inner.positions[3], &quad.positions[3]);
            func_L00_001FD1D8(&inner, 0, 0);
        }
    }
}
