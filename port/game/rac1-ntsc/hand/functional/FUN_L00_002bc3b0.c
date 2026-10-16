/* Carried from the PAL decompilation's src/overlays/shared/vendor_002BA7C8.c (matched C there) (games/rac1) through the version
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
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c2bc3b0;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2bc3b0 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2bc3b0 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2bc3b0;
extern char D_0013E533[];
extern float func_001FA6C0(int);
extern void func_001F9A10(void *, void *, void *);
extern char *D_L00_001ABA00[];
extern short D_L00_001616DC;
extern short D_L00_001616D8;
extern Vec4_c2bc3b0 D_L00_00173E60;
extern int D_L00_00173E40[];
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern float func_001FA688(float, float);
extern float func_001F9B48(void *, void *);
extern float func_001F99C0(float);
extern float func_001F9DF8(float);
extern int func_L00_0025C338(char *);
extern void func_00214DB0(void *, float, float, float);
extern int func_L00_001EFC70(Vec4f_c2bc3b0 *, Vec4f_c2bc3b0 *, int, void *, int);



char *FUN_L00_002bc3b0(char *self, float *position, float *heading, float *pitch) {
    float start[4] __attribute__((aligned(16)));
    float end[4] __attribute__((aligned(16)));
    float target[4] __attribute__((aligned(16)));
    float offset[4] __attribute__((aligned(16)));
    char *candidate;
    char *data;
    char *moby;
    int collision;
    int a0s[1];
    int res[1];
    float distance, angle, elevation, headingError, limit, apparentAngle, radius, reduction;
    int i;
    char *g;
    char *m0;

    limit = *(float *)&D_L00_001616DC;
    a0s[0] = (int)self;
    *(VecQuad_c2bc3b0 *)start = *(VecQuad_c2bc3b0 *)position;
    g = D_0013E533 + 0xE1D;
    m0 = *(char **)(g + 0x2080);
    *(VecQuad_c2bc3b0 *)end = *(VecQuad_c2bc3b0 *)(m0 + 0x10);
    position = start;
    end[2] = position[2];
    collision = func_L00_001EFC70((Vec4f_c2bc3b0 *)end, (Vec4f_c2bc3b0 *)position, 6, m0, 0);
    res[0] = 0;
    if (collision) {
        qcopy(position, &D_L00_00173E60);
        return 0;
    }
    i = 0;
    if ((candidate = D_L00_001ABA00[i]) != 0) do {
        if (*(signed char *)(candidate + 0x20) < 0 || *(short *)(candidate + 0xA6) == 0x58E ||
            *(short *)(candidate + 0xA6) == 0x452)
            continue;
        qcopy(target, candidate + 0x10);
        data = (char *)func_L00_0025C338(candidate);
        if (!data)
            continue;
        target[2] += *(float *)(data + 0x10);
        if (!candidate || !*(char **)(candidate + 0x24) ||
            *(short *)(*(char **)(candidate + 0x24) + 0x46) != 5)
            continue;
        angle = func_L00_001FF8B0(target[0] - position[0], target[1] - position[1]);
        distance = func_001F9B80(position, target);
        elevation = func_L00_001FF8B0(distance, target[2] - position[2]);
        headingError = func_001FA688(*heading, angle);
        distance = func_001F9B48(position, target);
        if (distance < 2.5f) {
            float toward;
            char *g1 = D_0013E533 + 0xE1D;
            moby = *(char **)(g1 + 0x2080);
            toward = func_L00_001FF8B0(*(float *)(candidate + 0x10) - *(float *)(g1 + 0x80),
                                   *(float *)(candidate + 0x14) - *(float *)(g1 + 0x84));
            if (func_001FA688(*(float *)(moby + 0x48), toward) < 1.0471976f &&
                func_001F99C0(position[2] - target[2]) < 2.0f &&
                !func_L00_001EFC70((Vec4f_c2bc3b0 *)position, (Vec4f_c2bc3b0 *)target, 6, *(char **)(g1 + 0x2080), 0)) {
                res[0] = (int)candidate;
                *pitch = -elevation;
                *heading = angle;
                goto done;
            }
        }
        if (distance > limit)
            continue;
        {
            char *g2 = D_0013E533 + 0xE1D;
            func_00214DB0(offset, distance, *heading, *pitch + *(float *)(g2 + 0x2E4) * 0.5f);
        }
        func_001F9A10(offset, offset, position);
        apparentAngle = func_001F9DF8(func_001F9B48(offset, target) / (distance + distance));
        apparentAngle += apparentAngle;
        if (*(float *)&D_L00_001616D8 * 0.017453292f < apparentAngle) {
            data = (char *)func_L00_0025C338(candidate);
            if (data) {
                radius = func_001FA6C0(*(unsigned char *)(data + 10)) * 0.125f;
                reduction = func_001F9DF8(radius / distance);
                if (radius < distance) {
                    apparentAngle -= reduction;
                    if (apparentAngle < 0.0f)
                        apparentAngle = 0.0f;
                }
            }
        }
        if (apparentAngle < *(float *)&D_L00_001616D8 * 0.017453292f) {
            int valid = 1;
            if (func_L00_001EFC70((Vec4f_c2bc3b0 *)position, (Vec4f_c2bc3b0 *)target, 6, self, 0)) {
                if (!D_L00_00173E40[6] || (char *)D_L00_00173E40[6] != candidate)
                    valid = 0;
            }
            if (valid) {
                *pitch = -elevation;
                limit = distance;
                if (headingError < 0.17453292f)
                    *heading = angle;
                res[0] = (int)candidate;
            }
        }
    } while ((candidate = D_L00_001ABA00[++i]) != 0);
done:
    return (char *)res[0];
}
