/* Carried from the PAL decompilation's nonmatching/shared/func_L11_0030A8C8.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L11_003094f8 -- src/overlays/shared/vendor_002C99E0.c
 * Best so far: SIZE ours 1456 / retail 1480, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Rocket moby update (1480 bytes): eases the moby toward its target with three 002140F8 samples, a flight loop (
 *   Remaining differences: about 40 regions, mostly constant loads and float register choices (retail reloads 1021
 *   Unblock: a full diff listing for the size gap, then the constant and register order of the loop.
 */
extern char D_L11_001B2A80[];
extern char D_L11_00174860[];
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015EE6C_e __asm__("D_0015ED6C") MACRO_ADDR;
extern float func_002132A8(float, float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_001F9688(int);
extern char *func_L00_002712B8(void *, void *, int, int, int, int, float, float, float, float, float);
extern void func_001F9CF8(void *, void *, void *);
extern float func_00213308(void);
extern void func_00214890(void *, void *, void *, float);
extern void func_L00_0026CBB0(void *, void *, int, int, int, int, float);
extern float func_001F9B48(void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern void func_00214DB0(void *, float, float, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_0020C828(void *);
extern int func_001F9740(int *);

// Rocket moby update: eases toward its target, runs the flight loop and spawns its effect.
void FUN_L11_003094f8(char *moby)
{
    char *data;
    char *p5;
    char *p22;
    char *p30;
    char *p24;
    float loc20[4];
    float loc30[4];
    float v40[4];
    float v50[4];
    float loc60[4];
    float f0;
    float f13;
    float f23;
    float t;
    int s;
    char *r5;
    int st;

    data = *(char **)(moby + 0x78);
    qcopy(loc20, moby + 0x10);
    p24 = *(char **)(moby + 0x24);
    p30 = moby + 0x10;
    p22 = moby + 0xC0;
    f0 = *(float *)(p24 + 0x24) - *(float *)(moby + 0x2C);
    *(float *)(moby + 0x2C) = *(float *)(moby + 0x2C) + f0 * 0.1f;
    qzero(loc60);
    loc60[0] = func_002132A8(-1.0f, 1.0f);
    loc60[1] = func_002132A8(-1.0f, 1.0f);
    loc60[2] = func_002132A8(-1.0f, 1.0f);
    qcopy(v50, loc60);
    f0 = func_002132A8(0.1f, 0.2f);
    func_L00_001FF500(v50, v50, f0 * D_0015EE6C_e);
    f0 = func_002132A8(D_0015EE6C_e * 0.1f, D_0015EE6C_e);
    func_L00_001FF500(v40, data, -f0);
    func_001F9A10(v50, v50, v40);
    s = func_001F9688(6);
    f23 = 0.0f;
    func_L00_002712B8(p30, v50, s, 0x7F, 0xB0B0B0, 3, 40000.0f, 1000.0f, 1.0f, -0.00039999999f, f23);

    do {
        t = f23;
        f23 = t + 0.33333334f;
        f0 = func_002132A8(t, f23);
        func_L00_001FF500(v40, data, f0 * *(float *)(data + 0x2C));
        func_001F9A10(v40, v40, p30);
        s = func_001F9688(0x3C);
        r5 = func_L00_002712B8(v40, v50, s, 0x7F, 0x606060, 3, 40000.0f, 1000.0f, 1.0f, -0.000199999995f, 0.0f);
        if (r5 != 0) {
            r5[2] = *(unsigned char *)*(char **)(D_L11_001B2A80 + 0x5C);
            r5[3] = 0x44;
        }
        qzero(loc60);
        loc60[2] = 0.0199999996f;
        func_001F9CF8(loc60, loc60, p22);
        f0 = func_00213308();
        func_00214890(loc60, loc60, p22, f0);
        s = func_001F9688(5);
        func_L00_0026CBB0(v40, loc60, 0x4F007FFF, 0x1FFFFFFF, s, 1, 20000.0f);
    } while (f23 < 1.0f);

    p5 = *(char **)(data + 0x24);
    st = 0;
    if (p5 != 0 && (unsigned char)p5[0x20] != 0xFE && (unsigned char)p5[0x20] != 0xFD) {
        f0 = func_001F9B48(data + 0x10, p5 + 0x10);
        if (f0 < 3.0f) {
            *(float *)(moby + 0x48) = func_L00_001FF8B0(*(float *)(p5 + 0x10) - *(float *)(moby + 0x10), *(float *)(p5 + 0x14) - *(float *)(moby + 0x14));
            f0 = func_001F9B80(p30, p5 + 0x10);
            *(float *)(moby + 0x44) = -func_L00_001FF8B0(f0, *(float *)(p5 + 0x18) - *(float *)(moby + 0x18));
            qcopy(data + 0x10, p5 + 0x10);
            goto ACE4_;
        }
    }
    f13 = D_0015ED70;
    f13 = f13 * 251.327408f;
    func_L00_0025BE00((float *)(moby + 0x48), (float *)(data + 0x30), *(float *)(moby + 0x48), f13, f13, D_0015EE6C_e * 2513.27417f);
    f13 = D_0015ED70 * 251.327408f;
    func_L00_0025BE00((float *)(moby + 0x44), (float *)(data + 0x30), *(float *)(moby + 0x44), f13, f13, D_0015EE6C_e * 2513.27417f);
    *(int *)(data + 0x24) = 0;

ACE4_:
    func_00214DB0(data, *(float *)(data + 0x2C), *(float *)(moby + 0x48), -*(float *)(moby + 0x44));
    func_001F9A10(p30, p30, data);
    if (*(float *)(moby + 0x10) < 2.0f) {
        goto AE24;
    }
    if (1021.0f < *(float *)(moby + 0x10)) {
        goto AE24;
    }
    if (*(float *)(moby + 0x14) < 2.0f) {
        goto AE24;
    }
    if (1021.0f < *(float *)(moby + 0x14)) {
        goto AE24;
    }
    if (*(float *)(moby + 0x18) < 2.0f) {
        goto AE24;
    }
    if (1021.0f < *(float *)(moby + 0x18)) {
        goto AE24;
    }
    func_L00_0024F7C8(moby, 1, loc30);
    s = func_L00_001EFC70(loc20, loc30, 0, (void *)*(int *)(data + 0x20), 0);
    if (s == 0) {
        if (func_001F9740((int *)(data + 0x28)) == 0) {
            goto EPI;
        }
        func_0020C828(moby);
        goto EPI;
    }
    func_L00_0025E450(moby, data, D_L11_00174860, 2.0f, 10.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, -1, 20.0f, 0, 1, -1, 0);

AE24:
    func_0020C828(moby);

EPI:
    ;
}
