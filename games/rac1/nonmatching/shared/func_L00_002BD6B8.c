/* NON_MATCHING func_L00_002BD6B8 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 1060 / retail 1076, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Scans the moby list at D_L00_001ABD80 for the nearest target to a1 (float math through matched helpers), write
 *   Left: the early code differs (retail stores a0 to sp+0x40 and keeps the result at sp+0x44 in memory; ours keep
 *   Unblock: a form of the entry copy and the result that gives retail's frame slots, then the placement of the r 
 */
extern unsigned char *func_L00_0025D390(int);
extern float func_L00_001FF860(float, float);
extern float func_001F9D48(void *, void *);
extern float func_001FA850(float, float);
extern float func_001F9D10(void *, void *);
extern float func_001F9B88(float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9FC0(float x);
extern float func_001FA888(int);
extern s32 func_L00_001EFFF0_C0358(void *, void *, s32, void *, s32) __asm__("func_L00_001EFFF0");
extern char *D_L00_001ABD80[];
extern int D_L00_00173F40[];
extern char D_L00_00173F60_v[] __asm__("D_L00_00173F60") MACRO_ADDR;
extern float D_L00_001617AC SDATA(D_L00_001617AC);
extern float D_L00_001617A8 SDATA(D_L00_001617A8);
extern char D_0013E633[];
typedef int u128_BD6B8 __attribute__((mode(TI)));

/* Scans the moby list for the target nearest a1; sets a2/a3 for the hit and returns that moby, else 0. */
int func_L00_002BD6B8(char *a0, float *a1, float *a2, float *a3) {
    float vec[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float f20, f21, f22, f23, f24, f25, f26, t, r;
    char *obj;
    char *g;
    char *obj2;
    char *p;
    char **k;
    int ok;
    int a0s[1];
    int res[1];

    f26 = D_L00_001617AC;
    a0s[0] = (int)a0;
    g = D_0013E633 + 0xE1D;
    *(u128_BD6B8 *)vec = *(u128_BD6B8 *)a1;
    obj2 = *(char **)(g + 0x2080);
    qcopy(v10, obj2 + 0x10);
    v10[2] = vec[2];
    res[0] = 0;
    if (func_L00_001EFFF0_C0358(v10, vec, 6, obj2, 0)) {
        *(u128_BD6B8 *)vec = *(u128_BD6B8 *)D_L00_00173F60_v;
        return 0;
    }
    for (k = D_L00_001ABD80; (obj = *k) != 0; k++) {
        if ((signed char)obj[0x20] < 0) continue;
        if (*(short *)(obj + 0xA6) == 0x58E) continue;
        if (*(short *)(obj + 0xA6) == 0x452) continue;
        qcopy(v20, obj + 0x10);
        p = (char *)func_L00_0025D390((int)obj);
        if (!p) continue;
        v20[2] = v20[2] + *(float *)(p + 0x10);
        if (!*(char **)(obj + 0x24) || *(short *)(*(char **)(obj + 0x24) + 0x46) != 5) continue;
        f23 = func_L00_001FF860(v20[0] - vec[0], v20[1] - vec[1]);
        t = func_001F9D48(vec, v20);
        f24 = func_L00_001FF860(t, v20[2] - vec[2]);
        f25 = func_001FA850(*a2, f23);
        f22 = func_001F9D10(vec, v20);
        if (f22 < 2.5f) {
            float x = *(float *)(obj + 0x10);
            t = func_L00_001FF860(x - *(float *)(g + 0x80), *(float *)(obj + 0x14) - *(float *)(g + 0x84));
            t = func_001FA850(*(float *)(*(char **)(g + 0x2080) + 0x48), t);
            if (t < 1.04719758f) {
                t = func_001F9B88(vec[2] - v20[2]);
                if (t < 2.0f) {
                    if (func_L00_001EFFF0_C0358(vec, v20, 6, *(void **)(g + 0x2080), 0) == 0) {
                        a3[0] = -f24;
                        a2[0] = f23;
                        res[0] = (int)obj;
                        return res[0];
                    }
                }
            }
        }
        if (f26 < f22) continue;
        func_00215C00(v30, f22, a2[0], a3[0] + *(float *)(g + 0x2E4) * 0.5f);
        func_001F9BD8(v30, v30, vec);
        r = func_001F9D10(v30, v20);
        f21 = func_001F9FC0(r / (f22 + f22));
        f21 = f21 + f21;
        if (0.0174532924f * D_L00_001617A8 < f21) {
            p = (char *)func_L00_0025D390((int)obj);
            if (p) {
                f20 = func_001FA888(*(unsigned char *)(p + 0xA)) * 0.125f;
                r = func_001F9FC0(f20 / f22);
                if (f20 < f22) {
                    f21 = f21 - r;
                    if (f21 < 0.0f) f21 = 0.0f;
                }
            }
        }
        if (f21 < D_L00_001617A8 * 0.0174532924f) {
            ok = 1;
            if (func_L00_001EFFF0_C0358(vec, v20, 6, (void *)a0s[0], 0)) {
                int x2 = D_L00_00173F40[6];
                if (x2 == 0 || x2 != (int)obj) ok = 0;
            }
            if (ok) {
                f26 = f22;
                a3[0] = -f24;
                if (f25 < 0.17453292f) a2[0] = f23;
                res[0] = (int)obj;
            }
        }
    }
    return res[0];
}
