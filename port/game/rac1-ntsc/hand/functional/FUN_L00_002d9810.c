/* Carried from port/game/rac1-pal/hand/level_nanotech_glow.c through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* The level's tables (level 0's addresses; the port relocates them to the level loaded). */
extern unsigned char D_L00_001E14C0[];
/* the hemisphere: 290 vertices, x y z w */
extern unsigned char D_L00_001E3DA0[];
/* the vertices turned to the camera: x y z, 12 bytes */
extern unsigned char D_L00_001E2FF0[];
/* their colours, RGBA */
extern unsigned char D_L00_001E26E0[];
/* their texture coordinates, S T floats */
extern unsigned char D_L00_001E3480[];
/* the halo's corner directions, x y z w */
extern unsigned char D_L00_001E3A80[];
/* the halo's 32 quads: 4 corner indices each (shorts) */
extern unsigned char D_L00_001E3B80[];
/* the crate glass's corners, x y z w */
extern unsigned char D_L00_001E3C80[];
/* the glass's 4 quads: 4 corner indices each (shorts) */
extern unsigned char D_L00_00161AF0[];
/* added to the centre */
extern unsigned char D_L00_00166DC0[];
/* the camera's position */
/* The level's constants, read through $gp in retail. */
extern float D_L00_00161A10;
/* the bob's amplitude (the alpha pulse is bob / this) */
extern int D_L00_00161AD4;
/* the sphere's effect texture */
extern float D_L00_00161ADC;
/* the halo's inner radius */
extern int D_L00_00161AE0;
/* the halo's colour (RGB) */
extern int D_L00_00161AE4;
/* the glass sheen's effect texture */
extern float D_L00_00161AE8;
/* the sheen's view vector length */
extern float D_L00_00161AEC;
/* the halo's outer radius over the inner */
extern int D_L00_00161B10;
extern long long func_001F44B8(int id);
/* GetEffectTex: the effect texture's TEX0 */
extern void func_001F9A10(void *out, void *a, void *b);
/* out = a + b */
extern void func_001F9A28(void *out, void *a, void *b);
/* out = a - b */
extern void func_001F9A68(void *out, void *a, float s);
/* out.xyz = a.xyz * s, w kept */
extern void func_001F9AD8(void *out, void *a, void *b);
/* out = a x b */
extern void func_001F9D20(void *out, void *v, void *m);
/* out = m * v (rows) */
extern void func_L00_001FD228(void *quad, void *matrix, int unused);
/* the callbacks' quad */
extern void func_L00_001FF500(void *out, void *v, float length);
extern void func_L00_001FF550(void *out, void *v, float length);
extern float func_L00_00200260(float a, float b);
/* a less the whole multiples of b */
extern int func_L00_002D8128(void *moby);
/* the moby's state is 1 (on its crate) */
extern int func_001FA6D0(float f);
typedef struct GlowQuad_c2d9810 {
    float corner[4][4];
    unsigned int rgba[4];
    float st[4][2];
    unsigned long long clamp, tex0, tex1, alpha;
} GlowQuad_c2d9810;
/* The sphere's strip from vertex `first`, `n` vertices, as quads of four strip vertices. */
static void sphere_strip_c2d9810(GlowQuad_c2d9810 *q, int first, int n) {
    int j, k;
    for (j = 0; j + 2 < n; j += 2) {
        for (k = 0; k < 4; k++) {
            /* An odd strip's last triangle: its fourth vertex repeats the third (no area). */
            const int v = first + (j + k < n ? j + k : n - 1);
            const float *xyz = (const float *)(D_L00_001E3DA0 + v * 12);
            q->corner[k][0] = xyz[0];
            q->corner[k][1] = xyz[1];
            q->corner[k][2] = xyz[2];
            q->corner[k][3] = 1.0f;
            q->rgba[k] = *(unsigned int *)(D_L00_001E2FF0 + v * 4);
            q->st[k][0] = *(float *)(D_L00_001E26E0 + v * 8);
            q->st[k][1] = *(float *)(D_L00_001E26E0 + v * 8 + 4);
        }
        func_L00_001FD228(q, 0, 0);
    }
}



void FUN_L00_002d9810(unsigned char *m) {
    GlowQuad_c2d9810 q;
    float matrix[4][4] __attribute__((aligned(16)));  /* rows F, R, U, p */
    float v[4] __attribute__((aligned(16)));
    float out[4] __attribute__((aligned(16)));
    unsigned char *pv = *(unsigned char **)(m + 0x78);
    unsigned char *box = *(unsigned char **)(pv + 0xC);  /* its crate */
    const float pulse = *(float *)pv / D_L00_00161A10;
    int i, k;

    /* The camera-facing frame at the centre. */
    matrix[3][0] = *(float *)(m + 0x10);
    matrix[3][1] = *(float *)(m + 0x14);
    matrix[3][2] = *(float *)(m + 0x18) + 0.5f + *(float *)pv;
    matrix[3][3] = 1.0f;
    func_001F9A10(matrix[3], matrix[3], D_L00_00161AF0);
    func_001F9A28(v, D_L00_00166DC0, matrix[3]);
    func_L00_001FF500(matrix[0], v, 1.0f);
    func_001F9AD8(matrix[1], matrix[0], box + 0xE0);
    func_L00_001FF500(matrix[1], matrix[1], 1.0f);
    func_001F9AD8(matrix[2], matrix[1], matrix[0]);

    /* 1. The sphere. */
    for (i = 0; i < 290; i++) {
        func_001F9D20(out, D_L00_001E14C0 + i * 16, matrix);
        *(float *)(D_L00_001E3DA0 + i * 12) = out[0];
        *(float *)(D_L00_001E3DA0 + i * 12 + 4) = out[1];
        *(float *)(D_L00_001E3DA0 + i * 12 + 8) = out[2];
    }
    q.clamp = 0;
    q.tex0 = (unsigned long long)func_001F44B8(D_L00_00161AD4);
    q.tex1 = 0xFF9000000260ull;
    q.alpha = 0x8000000044ull;
    sphere_strip_c2d9810(&q, 0, 0x94);
    sphere_strip_c2d9810(&q, 146, 0x6D);
    sphere_strip_c2d9810(&q, 254, 0x24);

    /* 2. The halo. */
    q.tex0 = (unsigned long long)func_001F44B8(0xB);
    q.alpha = 0x8000000048ull;
    {
        const unsigned int a = (unsigned int)(func_001FA6D0(pulse * 64.0f + 128.0f) + D_L00_00161B10);
        q.rgba[0] = q.rgba[1] = (a << 24) | (unsigned int)D_L00_00161AE0;
        q.rgba[2] = q.rgba[3] = 0;
    }
    q.st[0][0] = q.st[0][1] = q.st[1][0] = q.st[1][1] = 0.5f;
    q.st[2][0] = q.st[3][0] = 0.5f;
    q.st[2][1] = q.st[3][1] = 1.0f;
    for (i = 0; i < 32; i++) {
        for (k = 0; k < 4; k++) {
            const short idx = *(short *)(D_L00_001E3A80 + i * 8 + k * 2);
            const float r = k < 2 ? D_L00_00161ADC : D_L00_00161ADC * D_L00_00161AEC;
            func_001F9A68(q.corner[k], D_L00_001E3480 + idx * 16, r);
        }
        func_L00_001FD228(&q, matrix, 0);
    }

    /* 3. The crate's glass sheen, while the cluster sits on its crate. */
    if (!func_L00_002D8128(m)) {
        return;
    }
    q.tex0 = (unsigned long long)func_001F44B8(D_L00_00161AE4);
    q.rgba[0] = q.rgba[1] = q.rgba[2] = q.rgba[3] = 0x107F7F7Fu;
    q.alpha = 0x8000000044ull;
    for (k = 0; k < 4; k++) {
        matrix[0][k] = *(float *)(box + 0xC0 + k * 4);
        matrix[1][k] = *(float *)(box + 0xD0 + k * 4);
        matrix[2][k] = *(float *)(box + 0xE0 + k * 4);
    }
    matrix[3][2] = *(float *)(m + 0x18);
    for (i = 0; i < 4; i++) {
        for (k = 0; k < 4; k++) {
            const short idx = *(short *)(D_L00_001E3C80 + i * 8 + k * 2);
            func_001F9D20(q.corner[k], D_L00_001E3B80 + idx * 16, matrix);
            func_001F9A28(v, q.corner[k], D_L00_00166DC0);
            v[1] = v[2];
            func_L00_001FF550(v, v, D_L00_00161AE8);
            q.st[k][0] = func_L00_00200260(v[0] * 0.5f + 0.5f, 8.0f);
            q.st[k][1] = func_L00_00200260(v[1] * 0.5f + 0.5f, 8.0f);
        }
        func_L00_001FD228(&q, 0, 0);
    }
}
