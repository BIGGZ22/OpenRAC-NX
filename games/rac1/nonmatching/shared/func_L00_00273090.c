/* NON_MATCHING func_L00_00273090 -- src/overlays/shared/partupd_00272158.c
 * Best so far: SIZE ours 568 / retail 576, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   PartType53Update: tweens a particle's size (m+0xC) and alpha over its life (first 5 ticks ramp, then 1/func_00
 *   Stopped at budget: ours 568 bytes vs retail 576 (2 instrs short). Retail has two extra `daddu $3,$2,$0` regist
 *   Best candidate p7.c/p8.c (all else matches structurally). Unblock: find a typing of v[0xB] and of the alpha re
 *   Hint for a retry (no budget left): m/v were char*; the later function func_L00_0025E590 only matched once its 
 */
extern float func_001FA888(int);
extern int func_001FA898(float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_002688A8(void *);
extern int func_001F9938(void *);

/* particle update: tween size/alpha over its life, drift, kill when out of range */
void func_L00_00273090(char *m) {
    char *v = m + 0x20;
    int a;
    float t;
    float f;
    unsigned c;
    unsigned char u;
    float *p;
    float vec[4];
    m[8] += v[0xA];
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
        float q = func_001FA888(a - 6);
        s = *(float *)(m + 0x20);
        t = (float)*(short *)(m + 0xA) / q;
        *(float *)(m + 0xC) = s + ((s + *(float *)(v + 4)) * 0.5f - s) * t;
        u = v[0xB];
        f = (float)(u >> 1) * t + 0.0f;
    }
    c = func_001FA898(f);
    *(int *)(m + 4) = (c << 24) | (*(int *)(m + 4) & 0xFFFFFF);
    p = (float *)(v + 0xC);
    vec[0] = p[0];
    vec[1] = p[1];

    vec[3] = 0;
    vec[2] = p[2];
    func_001F9BD8(m + 0x10, m + 0x10, vec);
    *(float *)(v + 0x14) -= *(float *)(v + 0x18);
    if (*(float *)(m + 0x10) < 2.0f || *(float *)(m + 0x10) > 1021.0f ||
        *(float *)(m + 0x14) < 2.0f || *(float *)(m + 0x14) > 1021.0f ||
        *(float *)(m + 0x18) < 2.0f || *(float *)(m + 0x18) > 1021.0f) {
        func_L00_002688A8(m);
    } else if (func_001F9938(m + 0xA)) {
        func_L00_002688A8(m);
    }
}
