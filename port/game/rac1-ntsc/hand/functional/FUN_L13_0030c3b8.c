/* Carried from the PAL decompilation's src/overlays/l13_gemlik/vendor_0030CAE0.c (matched C there) (games/rac1) through the version
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
extern void func_0020C828(void *);
extern float func_002132A8(float, float);
extern float D_0015ED6C MACRO_ADDR;
extern short D_L13_00161F30;
extern short D_L13_00161F34;
extern short D_L13_00161F38;
extern short D_L13_00161F3C;
extern short D_L13_00161F44;
extern short D_L13_00161F48;
extern short D_L13_00161F4C;
extern short D_L13_00161F50;
extern short D_L13_00161F54;
extern short D_L13_00161F58;
extern short D_L13_00161F5C;
extern short D_L13_00161F60;
extern short D_L13_00161F64;
extern short D_L13_00161F68;
extern short D_L13_00161F08;
extern short D_L13_00161F0C;
extern short D_L13_00161F10;
extern short D_L13_00161F14;
extern short D_L13_00161F18;
extern short D_L13_00161F1C;
extern short D_L13_00161F20;
extern short D_L13_00161F24;
extern short D_L13_00161F28;
extern char D_0013E533[];
extern float D_0015ED70 MACRO_ADDR;
extern float func_L00_001FF8B0(float, float);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F96B0(float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern float func_001FA6C0(int);
extern int func_001F9770(void *);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L10_002E3DE8(char *moby);



/* Gemlik moby update (class 1634): two particle bursts around the moby, then movement and expiry. */
void FUN_L13_0030c3b8(char *moby)
{
    char *data = *(char **)(moby + 0x78);
    char buf[16];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    float f20, deg;
    float r, t;
    int k, ia, ib, ic;

    qcopy(buf, moby + 0x10);
    if (((unsigned char *)moby)[0xBC] == 1) {
        v30[3] = *(float *)&D_L13_00161F5C;
        v40[3] = *(float *)&D_L13_00161F60;
        for (k = 0; k < 2; k++) {
            qcopy(v20, moby + 0x10);
            r = func_002132A8(-0.0649999976f, 0.0649999976f);
            v20[0] = v20[0] + r;
            r = func_002132A8(-0.0649999976f, 0.0649999976f);
            v20[1] = v20[1] + r;
            r = func_002132A8(-0.0649999976f, 0.0649999976f);
            v20[2] = v20[2] + r;
            qzero(v30);
            r = 0.0174532924f;
            v30[2] = func_002132A8(*(float *)&D_L13_00161F30 * D_0015ED6C * 0.899999976f, *(float *)&D_L13_00161F30 * D_0015ED6C * 1.10000002f);
            f20 = func_002132A8(-*(float *)&D_L13_00161F50, *(float *)&D_L13_00161F50) * r;
            t = func_001FA580(func_L00_001FF8B0(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[0] = func_001F9DC8(t) * (*(float *)&D_L13_00161F24 * D_0015ED6C);
            t = func_001FA580(func_L00_001FF8B0(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[1] = func_001F9DE0(t) * (*(float *)&D_L13_00161F24 * D_0015ED6C);
            v40[2] = 0.0f;
            v40[2] = func_002132A8(*(float *)&D_L13_00161F28 * D_0015ED6C * 0.899999976f, *(float *)&D_L13_00161F28 * D_0015ED6C * 1.10000002f);
            t = func_002132A8(*(float *)&D_L13_00161F18, *(float *)&D_L13_00161F18 + *(float *)&D_L13_00161F18);
            ia = func_001FA898_r(func_001F96B0(t));
            t = func_002132A8(*(float *)&D_L13_00161F1C, *(float *)&D_L13_00161F1C * 1.20000005f);
            ib = func_001FA898_r(func_001F96B0(t));
            t = func_002132A8(*(float *)&D_L13_00161F20, *(float *)&D_L13_00161F20 * 1.20000005f);
            ic = func_001FA898_r(func_001F96B0(t));
            func_00218888(v20, v30, v40, *(int *)&D_L13_00161F08, *(int *)&D_L13_00161F0C, ia, ib, ic, -1);
        }
        v30[3] = *(float *)&D_L13_00161F64;
        v40[3] = *(float *)&D_L13_00161F68;
        for (k = 0; k < 2; k++) {
            qcopy(v50, moby + 0x10);
            r = func_002132A8(-0.100000001f, 0.100000001f);
            v50[0] = v50[0] + r;
            r = func_002132A8(-0.100000001f, 0.100000001f);
            v50[1] = v50[1] + r;
            r = func_002132A8(-0.100000001f, 0.100000001f);
            v50[2] = v50[2] + r;
            func_002132A8(-*(float *)&D_L13_00161F58, *(float *)&D_L13_00161F58);
            qzero(v30);
            deg = 0.0174532924f;
            v30[2] = func_002132A8(*(float *)&D_L13_00161F44 * D_0015ED6C * 0.699999988f, *(float *)&D_L13_00161F44 * D_0015ED6C * 1.5f);
            f20 = func_002132A8(-*(float *)&D_L13_00161F54, *(float *)&D_L13_00161F54) * deg;
            r = func_001FA580(func_L00_001FF8B0(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[0] = func_001F9DC8(r) * (*(float *)&D_L13_00161F48 * D_0015ED6C);
            r = func_001FA580(func_L00_001FF8B0(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[1] = func_001F9DE0(r) * (*(float *)&D_L13_00161F48 * D_0015ED6C);
            v40[2] = 0.0f;
            v40[2] = func_002132A8(*(float *)&D_L13_00161F4C * D_0015ED6C * 0.699999988f, *(float *)&D_L13_00161F4C * D_0015ED6C * 1.5f);
            r = func_002132A8(*(float *)&D_L13_00161F34, *(float *)&D_L13_00161F34 + *(float *)&D_L13_00161F34);
            ia = func_001FA898_r(func_001F96B0(r));
            r = func_002132A8(*(float *)&D_L13_00161F38, *(float *)&D_L13_00161F38 * 1.20000005f);
            ib = func_001FA898_r(func_001F96B0(r));
            r = func_002132A8(*(float *)&D_L13_00161F3C, *(float *)&D_L13_00161F3C * 1.20000005f);
            ic = func_001FA898_r(func_001F96B0(r));
            func_00218888(v50, v30, v40, *(int *)&D_L13_00161F10, *(int *)&D_L13_00161F14, ia, ib, ic, -1);
        }
    }

    *(float *)(moby + 0x40) = func_001FA580(*(float *)(moby + 0x40), *(float *)(data + 0x10));
    *(float *)(moby + 0x44) = func_001FA580(*(float *)(moby + 0x44), *(float *)(data + 0x14));
    func_001F9A10(moby + 0x10, moby + 0x10, data);
    *(float *)(data + 0x8) = *(float *)(data + 0x8) - D_0015ED70 * 14.6000004f;
    if (*(float *)(moby + 0x10) < 0.0f || *(float *)(moby + 0x14) < 0.0f || *(float *)(moby + 0x18) < 0.0f) {
        func_0020C828(moby);
        return;
    }
    if (*(short *)(data + 0x18) < *(short *)(data + 0x1A) / 4) {
        f20 = func_001FA6C0(*(short *)(data + 0x18));
        *(float *)(moby + 0x2C) = *(float *)(data + 0x1C) * f20 / func_001FA6C0(*(short *)(data + 0x1A) / 4);
    }
    if (func_001F9770(data + 0x18) != 0) {
        func_0020C828(moby);
    } else if (((unsigned char *)moby)[0xBC] == 1) {
        if (func_L00_001EFC70(buf, moby + 0x10, 2, *(int *)(D_0013E533 + 0x2E9D), 0)) {
            func_L10_002E3DE8(moby);
            func_0020C828(moby);
        }
    }
}
