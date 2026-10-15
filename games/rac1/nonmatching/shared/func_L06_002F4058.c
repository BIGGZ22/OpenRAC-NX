/* NON_MATCHING func_L06_002F4058 -- src/overlays/shared/vendor_002D9548.c
 * Best so far: SIZE ours 1700 / retail 1732, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Emitter moby update (1732 bytes): picks the spark colours and velocities from the class and the emitter state 
 *   Left: retail keeps f22 (0.0), f23 (0.0 copy) and f24 (0.25) live across the calls and saves them, plus $s2 (sq
 *   Unblock: a source form that makes the 0.0 and 0.25 constants live variables, which probably needs the loop-one
 */
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9D48(void *, void *);
extern int func_L00_0028EF68(int i, int a1, int v, int k);
extern float func_002140F8(float, float);
extern int func_L00_00258BC8(int, int);
extern float func_00214158(void);
extern float func_001F9F90(float);
extern float func_001FA7D8_26410(f32) __asm__("func_001FA7D8");
extern float func_001F9FA8_D9970b(float) __asm__("func_001F9FA8");
extern void func_L00_001FF240(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_001F9850(int);
extern int func_002140B0(int);
extern void func_L00_00272F00(void *, int, float, float, int, int, int, float, void *);
extern float func_001FA888(int);
extern float func_001FA748(float, float);
extern int func_L01_00277A38(float, int *, int, int, void *);
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern GbMoby *D_L06_00160058 MACRO_ADDR;
typedef int u128_F058 __attribute__((mode(TI)));

/* Particle emitter moby update: picks the spark colours and velocities from the class and the emitter state, then runs two spray loops. */
int func_L06_002F4058(char *ctx, char *ow, char *moby, int a3, int *out) {
    float v0[4];
    float vB[4];
    float vA[4];
    float vC[4];
    float vD[4];
    short cls;
    int v58;
    int i;
    int k;
    int r16, r17, r18, r2;
    int st8;
    char *st;
    char *g;
    float f0, f1, f20, f21, f22, f23, f24, f14;

    cls = *(short *)(moby + 0xA6);
    if (cls != 0x515 && cls != 0x15F) {
        func_001F9BD8(v0, moby + 0x10, moby + 0xC0);
    } else {
        *(u128_F058 *)v0 = *(u128_F058 *)(moby + 0x10);
    }
    cls = *(short *)(moby + 0xA6);

    if (cls == 0x400) goto L4678;
    if (cls < 0x401) {
        if (cls == 0x15F || cls == 0x3B1) goto L412C;
        return 0;
    }
    if (cls == 0x515) goto L412C;
    if (cls == 0x516) goto L4678;
    return 0;

L412C:
    f0 = func_001F9D48(ctx + 0x10, v0);
    if (!(f0 < 1.0f)) return 0;
    st = *(char **)(moby + 0x78);
    if (*(int *)st < 0) return 0;
    v58 = 0;
    if (cls == 0x3B1 || cls == 0x515 || cls == 0x15F) {
        v58 = *(int *)(st + 4);
        *(int *)(st + 4) = v58 + 1;
    }
    if (cls == 0x515) {
        func_L00_0028EF68(0, 0, (int)moby, 0x515);
    }

    f22 = 0.0f;
    f24 = 0.25f;
    k = 9;
    do {
        k--;
        f21 = func_002140F8(0.1f, 1.0f);
        r18 = func_L00_00258BC8(0x40, 0xFF);
        r17 = func_L00_00258BC8(0x40, 0xFF);
        r16 = func_L00_00258BC8(0x40, 0xFF);
        f20 = func_001F9F90(func_00214158()) * func_002140F8(0.0f, 0.25f);
        vA[0] = f20;
        f20 = func_001F9FA8_D9970b(func_00214158());
        f20 = f20 * func_002140F8(0.0f, 0.25f);
        vA[2] = 0.0f;
        vA[1] = f20;
        f1 = vA[2] + func_002140F8(0.0f, 0.5f);
        vA[2] = f1;
        func_L00_001FF240(vC, vA, ctx + 0x10);
        f0 = func_002140F8(0.0f, *(float *)&D_0015EE6C);
        func_L00_001FF4B0(vB, vB, f0);
        f0 = func_002140F8(*(float *)&D_0015EE6C * 2.5f, *(float *)&D_0015EE6C * 5.0f);
        vB[2] = f0;
        r2 = func_001F9850(func_L00_00258BC8(0xF, 0x1E));
        r17 = (r17 << 8) | 0x7F000000;
        r16 = (r16 << 16) | r17;
        f20 = f21 * 0.06f;
        f21 = f21 * 0.7f;
        r16 = r16 | r18;
        r17 = r2;
        r2 = func_002140B0(2);
        f14 = *(float *)&D_0015EE70 * 15.0f;
        st8 = -1;
        if (r2 != 0) st8 = 1;
        func_L00_00272F00(vA, r17, f20, f21, r16, 0, st8, f14, vB);
        f23 = f22;
    } while (k >= 0);

    g = (char *)D_L06_00160058 + (*(int *)st << 8);
    f0 = func_001FA888(v58) * 60.0f;
    f20 = func_001FA7D8_26410(f0 * 0.017453292f);
    f0 = func_001FA748(*(float *)(g + 0x48), f20);
    vB[0] = func_001F9F90(f0);
    f0 = func_001FA748(*(float *)(g + 0x48), f20);
    vB[1] = func_001F9FA8_D9970b(f0);
    vB[2] = f22;
    func_001F9BD8(ctx + 0x10, vB, g + 0x10);
    *(u128_F058 *)(ow + 0x2F0) = *(u128_F058 *)(ctx + 0x10);
    *(u128_F058 *)(ow + 0x2E0) = *(u128_F058 *)(ctx + 0x10);

    k = 9;
    do {
        k--;
        f21 = func_002140F8(0.1f, 1.0f);
        r18 = func_L00_00258BC8(0x40, 0xFF);
        r17 = func_L00_00258BC8(0x40, 0xFF);
        r16 = func_L00_00258BC8(0x40, 0xFF);
        f20 = func_001F9F90(func_00214158()) * func_002140F8(f23, 0.25f);
        vC[0] = f20;
        f20 = func_001F9FA8_D9970b(func_00214158());
        f20 = f20 * func_002140F8(f23, 0.25f);
        vC[2] = f23;
        vC[1] = f20;
        f1 = vC[2] + func_002140F8(f23, 0.5f);
        vC[2] = f1;
        func_L00_001FF240(vD, vC, ctx + 0x10);
        f0 = func_002140F8(f23, *(float *)&D_0015EE6C);
        func_L00_001FF4B0(vA, vA, f0);
        f0 = func_002140F8(*(float *)&D_0015EE6C * 2.5f, *(float *)&D_0015EE6C * 5.0f);
        vA[2] = f0;
        r2 = func_001F9850(func_L00_00258BC8(0xF, 0x1E));
        r17 = (r17 << 8) | 0x7F000000;
        r16 = (r16 << 16) | r17;
        f20 = f21 * 0.06f;
        f21 = f21 * 0.7f;
        r16 = r16 | r18;
        r17 = r2;
        r2 = func_002140B0(2);
        f14 = *(float *)&D_0015EE70 * 15.0f;
        st8 = (r2 != 0) ? 1 : -1;
        func_L00_00272F00(vC, r17, f20, f21, r16, 0, st8, f14, vA);
        f23 = f22;
    } while (k >= 0);

    *(char *)(ctx + 0x20) = 4;
    *out = 1;
    if (*(int *)(ow + 0x310) < 2) return 0;
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            if (*(int *)(ow + 0x1E0 + idx * 4) >= 0) {
                if (func_L01_00277A38(0.0f, (int *)(ow + 0x1E0 + idx * 4), 1, *(int *)(ow + 0x220 + idx * 4), ctx + 0x10) != 0) {
                    *(int *)(ow + 0x27C) = idx;
                }
            }
        }
    }
    return 0;

L4678:
    if (a3 != 0) return 0;
    if (*(unsigned char *)(moby + 0x20) == 2) return 0;
    f0 = func_001F9D48(ctx + 0x10, v0);
    if (!(f0 < 1.0f)) return 0;
    *(char *)(ctx + 0x20) = 8;
    *(int *)(ow + 0x304) = (int)moby;
    *out = 2;
    return 0;
}
