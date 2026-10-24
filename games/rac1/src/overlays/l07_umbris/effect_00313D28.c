/* Level 07 code: func_L07_00313D28 in a file of its own, built with
 * -mno-split-addresses (config/file_cflags.txt). The functions of
 * vendor_00313D28.c do not build with it (11 change), so it was another
 * object in retail (docs/BUILD_FIDELITY.md, rule 2). */
#include "common.h"
#include "callees.h"
#include "vector.h"
#include "moby.h"

extern void func_00234C98(int, u64);
extern u64 func_001F4868(int);
extern float func_001FA888(int);
extern int func_001FA898(float);
extern void func_L00_00250800(void *, int, void *);
extern float func_L00_002001D8(void *, float);
extern float func_L00_0025F368(float);
extern void func_00215C00(void *, float, float, float);
extern float func_001FA748(float, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FD1D8(void *, int, int);
extern s32 D_0015EF88;
extern s32 D_L07_0015F6B0;
typedef struct {
    float pos[4][4];
    u32 colors[4];
    float uv[4][2];
    u64 reserved;
    u64 texture;
    u64 texture_state;
    u64 primitive;
} FlareQuad_h6 __attribute__((aligned(16)));

static inline int fx_tint(int col) {
    int k = col | 0x404040;
    return k;
}

// Umbris effect: builds GS packets for a flare strip from the moby's data and draws its segments
void draw_snagglebeast_sphere_quad_strips(char *moby, int arg2, float fa, float fb, float fc, float fd, float fe) {
    char *data = *(char **)(moby + 0x78);
    int c = *(unsigned char *)(data + 0x1C6);
    int col = (c << 24) | (c << 16) | (c << 8) | c;
    u64 ts = 0xFF9000000260L;
    u64 prim = 0xA000000029L;
    FlareQuad_h6 quad;
    float v90[4] __attribute__((aligned(16)));
    float vA0[4] __attribute__((aligned(16)));
    float vB0;
    u64 texture;
    int color;
    float da, alpha, spin, step, ustep, angle, next, wrapped, theta, ph;

    VU1_addGSregister(0x4E, (u64)(D_0015EF88 >> 13) | 0x1000000L | (0x8000L << 17));
    texture = GetEffectTex(0x3C);
    ph = fd;
    color = truncate_float_to_s32(IntToFloat(0) * ph) << 24;
    color |= fx_tint(col);
    quad.colors[3] = color;
    quad.colors[2] = color;
    quad.colors[1] = color;
    quad.colors[0] = color;
    quad.texture = texture;
    quad.texture_state = ts;
    quad.primitive = prim;
    quad.reserved = 0;
    spin = IntToFloat(D_L07_0015F6B0 + arg2) * fa;
    Moby_GetJointWorldPos(moby, 9, v90);
    qcopy(vA0, v90);
    vA0[2] = vA0[2] - (fb - 0.8f);
    quad.uv[0][0] = split_float_fraction(&vB0, spin * -0.01f);
    quad.uv[2][0] = quad.uv[0][0];
    quad.uv[1][0] = quad.uv[0][0] + 1.0f;
    quad.uv[3][0] = quad.uv[1][0];
    quad.uv[0][1] = split_float_fraction(&vB0, spin * -0.02f);
    quad.uv[1][1] = quad.uv[0][1];
    quad.uv[2][1] = quad.uv[0][1] + 1.0f;
    quad.uv[3][1] = quad.uv[2][1];
    step = (fe - ph) * 0.0625f;
    ustep = fc * 0.0625f;
    da = 0.3926991f;
    for (angle = -2.7488937f; angle < 3.1415927f; angle = next) {
        qcopy(&quad.pos[2], vA0);
        qcopy(&quad.pos[3], vA0);
        next = angle + da;
        alpha = IntToFloat(0);
        ph = fd;
        color = truncate_float_to_s32(alpha * ph) << 24;
        color |= fx_tint(col);
        quad.colors[3] = color;
        quad.colors[2] = color;
        for (theta = ustep + -1.5707964f; theta <= fc - 1.5707964f; theta += ustep) {
            qcopy(&quad.pos[0], &quad.pos[2]);
            qcopy(&quad.pos[1], &quad.pos[3]);
            ph += step;
            quad.colors[0] = quad.colors[2];
            quad.colors[1] = quad.colors[3];
            color = truncate_float_to_s32(IntToFloat(0) * ph) << 24;
            color |= fx_tint(col);
            quad.colors[3] = color;
            quad.colors[2] = color;
            wrapped = normalize_angle(angle + spin * 0.02f);
            BuildSphericalOffset(&quad.pos[2], fb, wrapped, theta);
            BuildSphericalOffset(&quad.pos[3], fb, FastAddRots(wrapped, da), theta);
            if (0.0f < theta) {
                quad.pos[2][2] = quad.pos[2][2] + 0.6f;
                quad.pos[3][2] = quad.pos[3][2] + 0.6f;
            } else {
                quad.pos[2][2] = quad.pos[2][2] - 0.375f;
                quad.pos[3][2] = quad.pos[3][2] - 0.375f;
            }
            FastVecAdd(&quad.pos[2], &quad.pos[2], v90);
            FastVecAdd(&quad.pos[3], &quad.pos[3], v90);
            func_L00_001FD1D8(&quad, 0, 1);
        }
    }
    VU1_addGSregister(0x4E, (u64)(D_0015EF88 >> 13) | 0x1000000L);
}
