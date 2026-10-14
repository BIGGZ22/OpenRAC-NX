/* NON_MATCHING func_L04_00296938 -- src/overlays/l04_eudora/vuchain_00293490.c
 * Best so far: SIZE ours 1620 / retail 1644, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 04 walk/steer update (1644 bytes): blends the moby's heading and speed toward targets (func_L00_00259148
 *   Remaining differences: func_0020D830 / func_001FA888 / func_L04_00242868 results are kept live in retail ($f12
 */
extern float func_0020D830(void);
extern float func_001FA888(int);
extern float func_L04_00242868(int a, unsigned int b);
extern int func_L04_00293990(char *, char *);
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern void func_L04_00293F08(char *, char *, int, float);
extern void func_0020DB98(char *, int, void *, void *);
extern void func_001F9BC0(void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF328(void *, void *, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern int func_L04_002939E8(char *, char *);
extern void func_L00_002A2900(char *, char *, int, float, float, float);
extern void func_L01_002649D8(char *, int, void *, void *, void *, void *);
extern int func_L00_001F1D20(float, float, void *, int, void *);
extern float func_001F9CE8(void *);
extern float func_001FA850(float, float);
extern float func_001F9B88(float);
extern char D_L04_00174070[];
extern float D_0015EE60 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE68 MACRO_ADDR;
typedef int u128_96938 __attribute__((mode(TI)));

/* Walk/steer update for a moby on level 04: blends heading and speed toward targets, runs the walk step and returns its flags. */
int func_L04_00296938(char *m, char *s, float fa, float fb, float fc) {
    float f0, f1, f2, f12, f13, f14, f15, f16, f20, f22, f21, f23, f24;
    int ia[6];
    float v10[4], v20[24], v80[4];
    float a90, a94, a98;
    int r9c, r16, r21, r30, i, n, bit, k, m52;
    unsigned char old;
    char *q, *ps, *pa;
    char **pp;

    f21 = fa;
    f24 = fb;
    f22 = fc;
    if (*(unsigned char *)(m + 0x52) != 0xFF) {
        func_0020D830();
    } else {
        f20 = func_001FA888(*(unsigned char *)(m + 0x51));
        f0 = func_L04_00242868(*(unsigned char *)(m + 0x22), *(unsigned char *)(m + 0x53));
        f0 = f20 + f0;
    }

    r9c = func_L04_00293990(m, s);
    if (*(unsigned short *)(s + 0xEE) & 4) {
        f12 = *(float *)(m + 0x48);
        f13 = *(float *)(s + 0x6C);
        f14 = *(float *)(s + 0xCC);
        f15 = *(float *)(s + 0xD0);
    } else if (r9c & 0xF) {
        f12 = *(float *)(m + 0x48);
        f13 = f24;
        f14 = *(float *)(s + 0xCC);
        f15 = *(float *)(s + 0xD0);
    } else {
        f12 = *(float *)(m + 0x48);
        f13 = f24;
        f14 = 0.0f;
        f15 = *(float *)(s + 0xD0);
        f15 = f15 + f15;
    }
    f16 = *(float *)(s + 0xD4);
    *(float *)(m + 0x48) = func_L00_00259148((float *)(s + 0xC8), f12, f13, f14, f15, f16);
    func_L04_00293F08(m, s, r9c, f24);

    f0 = D_0015EE60;
    f16 = 0.017453292f;
    f16 = f0 * f16;
    f12 = *(float *)(m + 0x44);
    f13 = *(float *)(s + 0xB8);
    f14 = 0.005f;
    f15 = 0.01f;
    *(float *)(m + 0x44) = func_L00_00259148((float *)(s + 0xBC), f12, f13, f14, f15, f16);

    ia[0] = *(unsigned char *)(s + 0xF0);
    ia[1] = *(unsigned char *)(s + 0xF1);
    ia[2] = *(unsigned char *)(s + 0x1A0);
    ia[3] = *(unsigned char *)(s + 0x1A1);
    ia[4] = *(unsigned char *)(s + 0xB4);
    ia[5] = *(unsigned char *)(s + 0xB5);
    func_0020DB98(m, 6, ia, v20);

    if (!(*(unsigned short *)(s + 0xEE) & 4)) {
        r30 = r9c | (*(unsigned char *)(s + 0xB6) & 0x30);
        func_001F9BC0(v80);
        f20 = 0.0f;
        f23 = 1.0f;
        bit = 1;
        ps = s;
        pa = (char *)v20;
        for (n = 5; n >= 0; n--) {
            if (r30 & bit) {
                f20 = f20 + f23;
                func_001F9BD8(v80, v80, ps);
                func_001F9BF0(v80, v80, pa);
            }
            bit <<= 1;
            ps += 0x10;
            pa += 0x10;
        }
        func_L00_001FF328(v80, v80, f20);
    } else {
        f0 = func_001F9F90(*(float *)(s + 0x6C));
        f1 = *(float *)(s + 0xC0);
        f0 = f0 * f1;
        v80[0] = f0;
        f0 = func_001F9FA8(*(float *)(s + 0x6C));
        f1 = *(float *)(s + 0xC0);
        f0 = f0 * f1;
        v80[1] = f0;
        v80[2] = 0.0f;
    }
    ps = s;
    pa = (char *)v20;
    for (n = 5; n >= 0; n--) {
        func_001F9BD8(ps, pa, v80);
        ps += 0x10;
        pa += 0x10;
    }
    func_001F9BD8(m + 0x10, m + 0x10, v80);

    f0 = *(float *)(s + 0xC4);
    old = *(unsigned char *)(s + 0xB6);
    *(unsigned char *)(s + 0xB6) = r9c;
    r16 = (r9c ^ old) & r9c;
    if (f0 < f22) f22 = f0;

    f0 = *(float *)(s + 0xC0);
    if (f0 < f21) {
        f0 = f0 + f22;
        *(float *)(s + 0xC0) = f0;
        if (f21 < f0) *(float *)(s + 0xC0) = f21;
    } else if (f21 < f0) {
        f0 = f0 - f22;
        *(float *)(s + 0xC0) = f0;
        if (f0 < f21) *(float *)(s + 0xC0) = f21;
    }

    r21 = func_L04_002939E8(m, s);
    func_L00_002A2900(m, s, r16, f21, f24, f22);

    if (!(*(unsigned short *)(s + 0xEE) & 4)) {
        func_L01_002649D8(m, 0, &a90, ia, &a94, &a98);
        if (*(unsigned char *)(m + 0x53) == **(int **)(s + 0x7C) || *(unsigned char *)(m + 0x53) == **(int **)(s + 0x74) || *(unsigned char *)(m + 0x53) == **(int **)(s + 0x78)) {
            f0 = *(float *)(s + 0xE0);
            f0 = f0 + f0;
            *(float *)(s + 0xE8) = f0;
        } else if (*(unsigned char *)(m + 0x53) == **(int **)(s + 0x8C) && (*(unsigned char *)(s + 0xB6) & 0xF) != 0) {
            f0 = *(float *)(s + 0xE0);
            f0 = f0 + f0;
            *(float *)(s + 0xE8) = f0;
        } else if (*(unsigned char *)(m + 0x53) == *(unsigned char *)(m + 0x52)) {
            f0 = *(float *)(s + 0xE8);
            f0 = f0 * 0.9f;
            *(float *)(s + 0xE8) = f0;
        }
        f2 = *(float *)(s + 0xE8);
        ((float *)ia)[2] = ((float *)ia)[2] + f2;
        a98 = a98 - f2;
        func_001F9BF0(v10, m + 0x10, ia);
        for (k = 0; k < 8;) {
            if (!func_L00_001F1D20(a94, a98, ia, 4, m)) break;
            *(u128_96938 *)ia = *(u128_96938 *)D_L04_00174070;
            r21 = r21 | 1;
            k++;
        }
        func_001F9BF0(v20, ia, m + 0x10);
        func_001F9BD8(m + 0x10, ia, v10);
        f0 = func_001F9CE8(v20);
        f1 = *(float *)(s + 0xC0) * 0.5f;
        if (f0 < f1) r21 = r21 | 2;
    }

    ia[0] = *(unsigned char *)(s + 0xF0);
    ia[1] = *(unsigned char *)(s + 0xF1);
    ia[2] = *(unsigned char *)(s + 0x1A0);
    ia[3] = *(unsigned char *)(s + 0x1A1);
    ia[4] = *(unsigned char *)(s + 0xB4);
    ia[5] = *(unsigned char *)(s + 0xB5);
    func_0020DB98(m, 6, ia, s);
    if (*(unsigned char *)(s + 0xB7) == 0x12 && *(unsigned char *)(m + 0x52) == **(int **)(s + 0xA0)) {
        return r21;
    }
    *(float *)(m + 0x58) = 1.0f;
    if (*(unsigned char *)(s + 0xB7) == 0) return r21;

    m52 = *(unsigned char *)(m + 0x52);
    f22 = 0.0923f;
    pp = (char **)(s + 0x70);
    for (i = 0; ; ) {
        q = pp[0];
        if (*(int *)q == m52) {
            f20 = 0.19f;
            f0 = D_0015EE6C * f20;
            if (*(float *)(s + 0xC0) < f0) {
                f0 = func_001FA850(*(float *)(m + 0x48), f24);
                if (f22 < f0) {
                    f0 = D_0015EE6C * f20;
                    if (!(f21 < f0)) f0 = *(float *)(s + 0xC0);
                } else {
                    f0 = *(float *)(s + 0xC0);
                }
            } else {
                f0 = *(float *)(s + 0xC0);
            }
            f12 = *(float *)(q + 8);
            f0 = func_001F9B88(f0 / f12);
            f1 = D_0015EE68 * f0;
            *(float *)(m + 0x58) = f1;
            return r21;
        }
        i++;
        pp++;
        if (!(i < 13)) break;
    }
    return r21;
}
