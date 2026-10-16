/* NON_MATCHING func_L00_0026C3A0 -- src/overlays/shared/partupd_0026A130.c
 * Best so far: SIZE ours 660 / retail 656, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_0026C3A0 (PartType12Update): per-frame update of a particle; fades/scales it from a blend ratio = FA8
 *   p3.c is one instruction over (660 vs 656): retail keeps the blend ratio in $f12 (div.s $f12,$f20,$f0) with lo 
 *   Tried: ratio inline vs via locals a/b, lo reassigned vs a separate x, declaration orders. Would need whatever 
 */
extern unsigned char D_0013E15A[];
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern short D_L00_001602A8;
extern short D_L00_001602AC;
extern short D_L00_0016029C;
extern short D_L00_00160298;
extern short D_L00_0016027C;
extern short D_L00_00160278;
extern float func_001FA888(int);
extern float func_001F9CB8(void *);
extern float func_00214D88(float *, float *, float, float, float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_001FA8A8(int, int, float);
extern int func_001F9850(int);
extern int func_001F9938(void *);
extern void func_L00_002688A8(void *);
extern void func_001F9BD8(void *, void *, void *);

/* per-frame update of a spinning particle: fades, scales and kills it when out of range */
void func_L00_0026C3A0(unsigned char *m) {
    unsigned char *p = m + 0x20;
    float r;
    float w = 1.0f;
    float hi, lo, ratio;
    m[8] = m[8] + p[0x1E];
    if (p[0x1F] & 4) {
        w = func_001FA888(D_0013E15A[0x4D6]) * 0.5f + w;
    }
    func_001F9BD8(m + 0x10, m + 0x10, p);
    r = -func_001F9CB8(p);
    func_00214D88((float *)(m + 0x30), &r, 0.0f, 0.0f, *(float *)&D_L00_001602A8 * D_0015EE70, r);
    func_L00_001FF4B0(p, p, -r);
    *(float *)(m + 0x18) = *(float *)(m + 0x18) + *(float *)&D_L00_001602AC * D_0015EE6C;
    ratio = func_001FA888(*(short *)(m + 0xA)) / func_001FA888(p[0x1D]);
    if (p[0x1F] & 1) {
        lo = *(float *)&D_L00_0016029C;
        hi = *(float *)&D_L00_00160298;
    } else {
        lo = *(float *)&D_L00_0016027C;
        hi = *(float *)&D_L00_00160278;
    }
    {
        float x = lo * w;
        *(float *)(m + 0xC) = ((hi - x) * ratio + x) * 210000.0f;
    }
    *(int *)(m + 4) = func_001FA8A8(*(int *)(p + 0x18), *(int *)(p + 0x14), ratio);
    if (*(short *)(m + 0xA) < func_001F9850(15)) {
        float q = func_001FA888(*(short *)(m + 0xA));
        float t = q / func_001FA888(func_001F9850(10));
        *(int *)(m + 4) = func_001FA8A8(*(int *)(m + 4) & 0xFFFFFF, *(int *)(m + 4), t);
    }
    if (*(float *)(m + 0x10) < 2.0f || *(float *)(m + 0x10) > 1021.0f ||
        *(float *)(m + 0x14) < 2.0f || *(float *)(m + 0x14) > 1021.0f ||
        *(float *)(m + 0x18) < 2.0f || *(float *)(m + 0x18) > 1021.0f ||
        func_001F9938(m + 0xA) != 0) {
        func_L00_002688A8(m);
    } else if (*(float *)(p + 0x10) < 0.01f) {
        *(short *)(m + 0xA) = *(short *)(m + 0xA) - 1;
    }
}
