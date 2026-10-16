/* Carried from the PAL decompilation's nonmatching/functional/func_L06_002D9B08.c (games/rac1) through the version
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
/* functional: medium. Draw callback of a moby (Umbris-style backdrop cube): transforms 57 vertices by the moby's 3x4 matrix; when the hero is within 16 units in x and y it also computes reflection-style UVs; then draws up to 42 front-facing quads. Not run against captured calls. */
typedef struct Quad_D9B08_c2d8738 {
    float corner[4][4];
    int color[4];
    float uv[4][2];
    long unk70;
    long tex;
    long unk80;
    long unk88;
} Quad_D9B08_c2d8738;
extern char D_L06_00167500_D9B08[] __asm__("D_L06_00167400");
extern char D_L06_00167640_D9B08[] __asm__("D_L06_00167540");
extern float D_L06_001D35E0_D9B08[][4] __asm__("D_L06_001D3290");
extern float D_L06_001D4080_D9B08[][4] __asm__("D_L06_001D3D30");
extern float D_L06_001D4880_D9B08[][4] __asm__("D_L06_001D4530");
extern float D_L06_001D46B0_D9B08[][2] __asm__("D_L06_001D4360");
extern float D_L06_001D4410_D9B08[][4] __asm__("D_L06_001D40C0");
extern float D_L06_001D3DE0_D9B08[][4] __asm__("D_L06_001D3A90");
extern short D_L06_001D3970_D9B08[][8] __asm__("D_L06_001D3620");
extern float D_L06_001D3C10_D9B08[][2] __asm__("D_L06_001D38C0");
extern short G00_D9B08 __asm__("D_L06_00161870");
extern short G04_D9B08 __asm__("D_L06_00161874");
extern short G08_D9B08 __asm__("D_L06_00161878");
extern short G0C_D9B08 __asm__("D_L06_0016187C");
extern short G10_D9B08 __asm__("D_L06_00161880");
extern short G14_D9B08 __asm__("D_L06_00161884");
extern short G18_D9B08 __asm__("D_L06_00161888");
extern short G1C_D9B08 __asm__("D_L06_0016188C");
extern float func_001F9B88_D9B08(float) __asm__("func_001F99C0");
extern float func_001F9B50_D9B08(float) __asm__("func_001F9988");
extern void func_001F9EE8_D9B08(void *, void *, void *) __asm__("func_001F9D20");
extern void func_001F9EC0_D9B08(void *, void *, void *) __asm__("func_001F9CF8");
extern void func_001F9BF0_D9B08(void *, void *, void *) __asm__("func_001F9A28");
extern void func_L00_001FF4B0_D9B08(void *, void *, float) __asm__("func_L00_001FF500");
extern float func_001F9C78_D9B08(void *, void *) __asm__("func_001F9AB0");
extern void func_001F9C30_D9B08(void *, void *, float) __asm__("func_001F9A68");
extern long func_001F4868_D9B08(int) __asm__("func_001F44B8");
extern void func_L00_001FD1D8_D9B08(void *, int, int) __asm__("func_L00_001FD228");

void func_L06_002D9B08_impl(char *moby) __asm__("FUN_L06_002d8738");


void func_L06_002D9B08_impl(char *moby) {
    Quad_D9B08_c2d8738 quad;
    float matrix[3][4] __attribute__((aligned(16)));
    float pos[4] __attribute__((aligned(16)));
    float direction[4] __attribute__((aligned(16)));
    float facePos[4] __attribute__((aligned(16)));
    float normal[4] __attribute__((aligned(16)));
    float proj[4] __attribute__((aligned(16)));
    int near;
    int i, j;
    float s, x, y;
    float *ref = (float *)D_L06_00167640_D9B08;
    float *g = (float *)D_L06_00167500_D9B08;

    qcopy(matrix[0], moby + 0xC0);
    qcopy(matrix[1], moby + 0xD0);
    qcopy(matrix[2], moby + 0xE0);
    pos[0] = *(float *)(moby + 0x10);
    pos[1] = *(float *)(moby + 0x14);
    pos[2] = *(float *)(moby + 0x18);
    pos[3] = 1.0f;
    near = 0;
    if (func_001F9B88_D9B08(g[0x140 / 4] - pos[0]) < 16.0f) {
        if (func_001F9B88_D9B08(g[0x144 / 4] - *(float *)(moby + 0x14)) < 16.0f) {
            near = 1;
        }
    }
    if (near) {
        float (*dst)[4] = D_L06_001D4880_D9B08;
        float (*uvo)[2] = D_L06_001D46B0_D9B08;
        for (i = 0; i < 57; i++) {
            func_001F9EE8_D9B08(dst, D_L06_001D35E0_D9B08[i], matrix);
            func_001F9BF0_D9B08(direction, dst, ref);
            dst++;
            func_L00_001FF4B0_D9B08(direction, direction, 1.0f);
            func_001F9EE8_D9B08(normal, D_L06_001D4080_D9B08[i], matrix);
            func_L00_001FF4B0_D9B08(normal, normal, *(float *)&G1C_D9B08);
            s = func_001F9C78_D9B08(normal, direction);
            func_001F9C30_D9B08(proj, normal, s + s);
            func_001F9BF0_D9B08(proj, direction, proj);
            func_L00_001FF4B0_D9B08(proj, proj, 1.0f);
            proj[2] = proj[2] + 1.0f;
            s = func_001F9B50_D9B08(proj[2] + proj[2]);
            s = s + s;
            x = proj[0];
            y = proj[1];
            y = y / s + 0.5f;
            x = x / s + 0.5f;
            (*uvo)[1] = y;
            (*uvo)[0] = x;
            uvo++;
        }
    } else {
        float (*dst)[4] = D_L06_001D4880_D9B08;
        float (*src)[4] = D_L06_001D35E0_D9B08;
        for (i = 56; i >= 0; i--) {
            func_001F9EE8_D9B08(dst, src, matrix);
            dst++;
            src++;
        }
    }
    quad.tex = func_001F4868_D9B08(*(int *)&G14_D9B08);
    quad.unk80 = 0xFF9000000260L;
    quad.unk88 = (long)*(int *)&G00_D9B08 | ((long)*(int *)&G04_D9B08 << 2) |
                 ((long)*(int *)&G08_D9B08 << 4) | ((long)*(int *)&G0C_D9B08 << 6) |
                 ((long)*(int *)&G10_D9B08 << 32);
    quad.color[0] = *(int *)&G18_D9B08;
    quad.unk70 = 0;
    quad.color[3] = *(int *)&G18_D9B08;
    quad.color[2] = *(int *)&G18_D9B08;
    quad.color[1] = *(int *)&G18_D9B08;
    i = 0;
    do {
        func_001F9EE8_D9B08(facePos, D_L06_001D4410_D9B08[i], matrix);
        func_001F9BF0_D9B08(direction, facePos, ref);
        func_L00_001FF4B0_D9B08(direction, direction, 1.0f);
        func_001F9EC0_D9B08(facePos, D_L06_001D3DE0_D9B08[i], matrix);
        if (!(0.0f < func_001F9C78_D9B08(direction, facePos))) {
            short *idx = D_L06_001D3970_D9B08[i];
            for (j = 0; j < 4; j++, idx += 2) {
                short v = idx[0];
                qcopy(quad.corner[j], D_L06_001D4880_D9B08[v]);
                if (near) {
                    quad.uv[j][0] = D_L06_001D46B0_D9B08[v][0];
                    quad.uv[j][1] = D_L06_001D46B0_D9B08[v][1];
                } else {
                    quad.uv[j][0] = D_L06_001D3C10_D9B08[idx[1]][0];
                    quad.uv[j][1] = D_L06_001D3C10_D9B08[idx[1]][1];
                }
            }
            func_L00_001FD1D8_D9B08(&quad, 0, 0);
        }
        i++;
    } while (i < 42);
}
