/* NON_MATCHING func_L01_00234970 -- src/overlays/shared/help_002274A8.c
 * Best so far: SIZE ours 668 / retail 660, checked 2026-10-03.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   AirAccel: switch on g[0x20B3] (0: scale stick vector, project, clamp to [1,3 or 1.7], update g[0x194]; 1,2: mo
 *   Best p0.c (628 vs 660 bytes): control flow and calls right, diff is addressing: retail never keeps g=D_0013E63
 *   Folding g into each access (p1) folds the offset into the symbol (wrong). Would need a way to make the allocat
 *   Update: best is p5.c (648 vs 660). A fresh `char *gN = D_0013F450;` (NOT_SDA) per if-block, as in func_L00_002
 *   Left: prologue hi in $s1 with g in $a0 ($s1 vs $s0 for the later g), v[1]/v[2] load/store order after func_001
 */
typedef int u128 __attribute__((mode(TI)));
extern unsigned char D_0013E633[];
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001F9CE8(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_L00_00213A08(float *);
extern float func_L00_00234250(float *v);
extern void func_L00_00234420(float *dst, float *src, float z);
extern void func_L00_00233F88(float *dst, float *src, float r);
extern float D_0015EE6C MACRO_ADDR;

// Accelerates the hero's air velocity toward the stick direction, clamped to the step.
void func_L01_00234970(float step) {
    char *h;
    float v[4];
    float w[4];
    float u[4];
    float f20, f21, f0, f1;
    switch (*(unsigned char *)(((char *)D_0013E633 + 0xE1D) + 0x20B3)) {
    case 0:
        f21 = *(float *)(((char *)D_0013E633 + 0xE1D) + 0x180);
        f20 = *(float *)(((char *)D_0013E633 + 0xE1D) + 0x190);
        if (*(int *)(((char *)D_0013E633 + 0xE1D) + 0x208C) == 4) {
            if (*(short *)(((char *)D_0013E633 + 0xE1D) + 0x30A) == 0 && *(short *)(((char *)D_0013E633 + 0xE1D) + 0x4A4) != 0 && *(unsigned char *)(((char *)D_0013E633 + 0xE1D) + 0x257) != 0)
                f20 = D_0015EE6C * 2.1f;
        }
        if (*(short *)(((char *)D_0013E633 + 0xE1D) + 0x30A) != 0) {
            if (*(int *)(((char *)D_0013E633 + 0xE1D) + 0x2084) == 0xE) {
                if (f20 > D_0015EE6C) f20 = D_0015EE6C;
            }
        }
        v[0] = func_001F9F90(f21) * f20;
        v[1] = func_001F9FA8(f21) * f20;
        v[2] = *(float *)(((char *)D_0013E633 + 0xE1D) + 0xE8);
        f20 = -(*(float *)(((char *)D_0013E633 + 0xE1D) + 0xE0) * v[0] + *(float *)(((char *)D_0013E633 + 0xE1D) + 0xE4) * v[1]);
        f21 = func_001F9CE8(v);
        f0 = func_001F9CE8(((char *)D_0013E633 + 0xE1D) + 0xE0);
        if (f21 == 0.0f || f0 == 0.0f) f20 = 0.0f;
        else f20 = f20 / f21 / f0;
        f20 = (f20 + 1.0f) * 1.5f;
        f1 = 3.0f;
        if (*(int *)(((char *)D_0013E633 + 0xE1D) + 0x2084) == 0x81) f1 = 1.7f;
        if (f20 > f1) f20 = f1;
        if (f20 < 1.0f) f20 = 1.0f;
        h = (char *)D_0013E633 + 0xEFD;
        step = step * f20;
        func_001F9BF0(v, v, h);
        if (step < func_001F9CE8(v)) func_L00_001FF500(v, v, step);
        func_001F9BD8(h, h, v);
        *(u128 *)w = *(u128 *)(h + 0x840);
        f0 = func_L00_00213A08(w);
        *(float *)(((char *)D_0013E633 + 0xE1D) + 0x194) = *(float *)(((char *)D_0013E633 + 0xE1D) + 0x168) - f0;
        break;
    case 1:
    case 2:
        h = (char *)D_0013E633 + 0xEFD;
        u[0] = func_L00_00234250((float *)h);
        func_00214D28(u, *(float *)(h + 0xB0), step);
        func_L00_00234420((float *)h, (float *)h, 0.0f);
        func_L00_00233F88((float *)h, (float *)h, u[0]);
        break;
    }
}
