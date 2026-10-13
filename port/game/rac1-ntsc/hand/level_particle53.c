/* Carried from port/game/rac1-pal/hand/level_particle53.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL particle type 53 update, recovered from the complete 0x240
 * retail body and nonmatching/shared/FUN_L00_002721f0.c. Not a PS2 match.
 * Byte accumulation wraps; RGB survives alpha updates. Out-of-bounds
 * removal precedes lifetime decrement, as in retail. */
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_00267A08(void *);
extern int func_001F9770(void *);

/* particle update: tween size/alpha over its life, drift, kill when out of range */
void FUN_L00_002721f0(char *m) {
    char *v = m + 0x20;
    int a;
    float t;
    float f;
    unsigned c;
    unsigned char u;
    float *p;
    float vec[4];
    ((unsigned char *)m)[8] += ((unsigned char *)v)[0xA];
    a = *(short *)(v + 8);
    if (a - *(short *)(m + 0xA) < 6) {
        float s;
        t = (float)(a - *(short *)(m + 0xA)) / 5.0f;
        s = *(float *)(v + 4);
        *(float *)(m + 0xC) = s + ((*(float *)(m + 0x20) + s) * 0.5f - s) * t;
        u = v[0xB];
        f = (float)u + (float)((u >> 1) - u) * t;
    } else {
        float s;
        float q = func_001FA6C0(a - 6);
        s = *(float *)(m + 0x20);
        t = (float)*(short *)(m + 0xA) / q;
        *(float *)(m + 0xC) = s + ((s + *(float *)(v + 4)) * 0.5f - s) * t;
        u = v[0xB];
        f = (float)(u >> 1) * t + 0.0f;
    }
    c = func_001FA6D0(f);
    *(unsigned *)(m + 4) = (c << 24) | (*(unsigned *)(m + 4) & 0xFFFFFFu);
    p = (float *)(v + 0xC);
    vec[0] = p[0];
    vec[1] = p[1];

    vec[3] = 0;
    vec[2] = p[2];
    func_001F9A10(m + 0x10, m + 0x10, vec);
    *(float *)(v + 0x14) -= *(float *)(v + 0x18);
    if (*(float *)(m + 0x10) < 2.0f || *(float *)(m + 0x10) > 1021.0f ||
        *(float *)(m + 0x14) < 2.0f || *(float *)(m + 0x14) > 1021.0f ||
        *(float *)(m + 0x18) < 2.0f || *(float *)(m + 0x18) > 1021.0f) {
        func_L00_00267A08(m);
    } else if (func_001F9770(m + 0xA)) {
        func_L00_00267A08(m);
    }
}
