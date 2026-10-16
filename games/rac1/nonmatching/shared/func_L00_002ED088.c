/* NON_MATCHING func_L00_002ED088 -- src/overlays/shared/vendor_002EB0D8.c
 * Best so far: BYTES 87/664 (86.9% of the bytes match), checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: the hero as a local struct (moby 0x2080, state 0x2084, unk964, unk994, unk9AC; the file cannot
 *   include hero.h because it defines its own Vec4) brings the size to retail's 664, BYTES 87/664. Retail
 *   keeps the hero's %hi in $a1 from the prologue and rebuilds the address twice. Left: retail loads
 *   D_0015EE84 and the hero's %hi before the m+0x70 load and the zero store (a float zero store does not
 *   change that), and the constants of the q + 0xA0 block load in another order. The if/else with a store in
 *   each arm is right: `f = 1.5f; if (...) f = 2.0f;` loses retail's duplicated store (660 bytes).
 */
#include "common.h"
extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;
typedef struct {
    char pad0[0x964];
    char *unk964;
    char pad968[0x994 - 0x968];
    char *unk994;
    char pad998[0x9AC - 0x998];
    float unk9AC;
    char pad9B0[0x2080 - 0x9B0];
    char *moby;
    int state;
} HeroCam_2ED088;
extern HeroCam_2ED088 D_0013F450_2ED088 __asm__("D_0013F450");
extern char D_L00_00166D80_c[] __asm__("D_L00_00166D80");
extern int func_001F9850(int);
extern void func_001F9BC0(void *);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_L00_001FF860(float, float);

/* Sets up the camera activation moby: tuning floats, then the pose copied from the current camera. */
void func_L00_002ED088(void *arg) {
    char *m = arg;
    char *p = *(char **)(m + 0x70) + 0x40;
    float f;
    char *t, *q, *r, *w, *s19, *src;
    char *gg, *gg0;
    *(int *)(p + 0x40) = 0;
    f = 1.5f;
    if (D_0015EE84_m == 0xE && (unsigned)(D_0013F450_2ED088.state - 0x2C) < 2)
        *(float *)(p + 0x4C) = 2.0f;
    else
        *(float *)(p + 0x4C) = 1.5f;
    *(float *)(p + 0x48) = 0.20943951606750488f;
    *(float *)(p + 0x58) = 0.001745329238474369f;
    *(float *)(p + 0x5C) = -0.004363323096185923f;
    *(int *)(p + 0x44) = (int)D_0013F450_2ED088.moby;
    q = *(char **)(m + 0x70);
    *(float *)(q + 0xA0) = 4.639999866485596f;
    q += 0xA0;
    *(float *)(q + 0x14) = 0.05f;
    *(float *)(q + 4) = 0.5f;
    *(float *)(q + 0x10) = 2.0f;
    r = *(char **)(m + 0x70);
    *(int *)(r + 0x18) = 0;
    *(float *)(r + 0x10) = 0.01f;
    *(float *)(r + 0x14) = 0.3f;
    *(short *)(r + 0x1E) = func_001F9850(0x78);
    *(short *)(r + 0x1C) = 0;
    func_001F9BC0(r);
    q = *(char **)(m + 0x70);
    t = q + 0x30;
    *(float *)(t + 4) = 0.02f;
    *(float *)(t + 8) = 0.2f;
    *(int *)(t + 0xC) = 0;
    *(int *)(q + 0x30) = 0;
    s19 = *(char **)(m + 0x70);
    w = s19 + 0xB8;
    *(int *)(w + 4) = 0;
    gg0 = D_L00_00166D80_c;
    src = *(char **)(gg0 + 0x184);
    qcopy(m + 0x30, src + 0x30);
    qcopy(m, src);
    qcopy(m + 0x10, src + 0x10);
    qcopy(m + 0x20, src + 0x20);
    qcopy(m + 0x40, m);
    *(short *)(m + 0x7E) = 0;
    if ((unsigned)(D_0013F450_2ED088.state - 0x2C) < 2) {
        *(float *)(p + 0x30) = func_001F9F90(D_0013F450_2ED088.unk9AC);
        *(float *)(p + 0x34) = func_001F9FA8(D_0013F450_2ED088.unk9AC);
        *(int *)(p + 0x38) = 0;
        *(float *)(s19 + 0xB8) = D_0013F450_2ED088.unk9AC;
        *(int *)(w + 8) = (int)D_0013F450_2ED088.unk994;
    } else {
        func_001F9BF0(p + 0x30, D_0013F450_2ED088.unk964 + 0x10, m + 0x30);
        *(int *)(p + 0x38) = 0;
        *(int *)(p + 0x3C) = 0;
        func_L00_001FF4B0(p + 0x30, p + 0x30, 1.0f);
        *(float *)(s19 + 0xB8) = func_L00_001FF860(*(float *)(p + 0x30), *(float *)(p + 0x34));
        *(int *)(w + 8) = (int)D_0013F450_2ED088.unk964;
    }
    gg = D_L00_00166D80_c;
    *(char *)(gg + 0x273) = 2;
    *(short *)(gg + 0x270) = 1;
    *(int *)(gg + 0x2F4) = func_001F9850(0x3C);
}
