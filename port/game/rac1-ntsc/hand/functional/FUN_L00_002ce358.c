/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_L00_002CF808.c (games/rac1): func_L00_002CF808, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: high. Corrected from nonmatching/shared/func_L00_002CF808.c: the pitch target (f22 in state 2, m+0x44 in state 3) is the float result of func_L00_001FF860 negated; the near miss truncated it to int first, which drops the fraction. */
/* carry, not from a `same` function or an `exact` data address (read them):
 *   func_L00_0025F4A8 -> 0025E450: near
 */

static __inline__ void qcopy_c00_2ce358(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}
typedef union { float v[4]; unsigned int q __attribute__((mode(TI))); } Q4_F808_c00_2ce358 __attribute__((aligned(16)));
extern void *func_L00_0025A420(void *, int, int);
extern char D_0013E533[];
extern int D_L00_0015F5C4 ;
extern float D_0015ED60 ;
extern float D_0015ED6C ;
extern float D_0015ED70 ;
extern float D_L00_00166DC0[];
extern int D_L00_00173E40[];
extern char D_L00_00173E70[];
extern char *func_L00_002CDF28(char *, int *);
extern int func_L00_0025C338(char *);
extern float func_001F9D48_07408_c00_2ce358(void *, void *) __asm__("func_001F9B80");
extern int func_001160D8(void);
extern void func_0022DA68(int, int, char *);
extern float func_001FA6C0(int);
extern int func_001F9770(void *);
extern void func_L00_002CDE78(char *);
extern float func_001FA580(float, float);
extern float func_001FA5C8(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001FA050(float *, float *);
extern void func_001F9D20(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_L00_001FF598(void *, void *, float);
extern int func_L00_00258830(void *, void *, float, float, float, int);
extern int func_L00_001F0D60(float, void *, int, void *);
extern void func_L00_00259858(char *a, int b, int c, float d);
extern void func_L00_001FF550(void *, void *, float);
extern void func_L00_00259A88(void *, void *);
extern int func_001FA898_r_c00_2ce358(float) __asm__("func_001FA6D0");
extern float func_L00_001FF8B0(float, float);
extern float func_L00_00258110(float *vel, float cur, float target, float k, float d, float max);
extern void func_0020C828(void *);
extern void func_L00_00259FE8(unsigned char *, float);
extern void func_L00_0025F4A8_alt_c00_2ce358(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025E450");
void FUN_L00_002ce358(char *m) {
    char *p;
    char *P;
    char *G;
    char *best = 0;
    char *r19;
    char *obj;
    char *q;
    char *pos;
    char *pb4;
    char *p2;
    char *cls;
    char *s;
    int st;
    int i, k;
    int i22 = 0;
    int i6;
    int *p4;
    int *p5;
    int n16;
    int n18;
    int cnt;
    int idx;
    int k16;
    int t;
    int r2;
    int r3;
    int r5;
    int r16;
    int rr;
    int rz;
    float f20;
    float f21;
    float f22;
    float f23;
    float d;
    float a, b, kk;
    float A, B2, C, Gf, H, I;
    float r1;
    float r2f;
    Q4_F808_c00_2ce358 t20, t30, t40, t50, t60, t70;
    st = D_L00_0015F5C4;
    p = *(char **)(m + 0x78);
    if (st == 2 || st == 5 || st == 6) goto L_884;
    P = D_0013E533 + 0xE1D;
    if (*(int *)(P + 0x2084) == 0x32) goto L_884;
    goto L_894;
L_884:
    *(unsigned short *)(m + 0x34) |= 0x41;
    return;
L_894:
    *(unsigned short *)(m + 0x34) &= 0xFFBE;
    if (*(unsigned char *)(m + 0x20) >= 5) goto L_C70;
    {
        unsigned char c = *(unsigned char *)(P + 0x1FF4) + 1;
        *(unsigned char *)(P + 0x1FF4) = c;
        if (*(unsigned char *)(P + 0x1FF6) < c) {
            *(unsigned char *)(P + 0x1FF4) = 1;
            best = 0;
            i22 = 0;
            d = func_001FA580(*(float *)(P + 0x1FF8), D_0015ED60 * 0.05f);
            *(unsigned char *)(P + 0x1FF7) = 0;
            *(float *)(P + 0x1FF8) = d;
            f20 = 1000000.0f;
            p4 = (int *)(P + 0x2020);
            p5 = (int *)(P + 0x1FFC);
            for (i6 = 0; i6 < 6; i6++, p4++) {
                char *mm = (char *)*p4;
                if (mm == 0) continue;
                if (*(short *)(mm + 0xA6) != 0x1DF) {
                    *p4 = 0;
                    continue;
                }
                if (*(unsigned char *)(mm + 0x20) == 0xFE) {
                    *p4 = 0;
                    continue;
                }
                if (*(unsigned char *)(mm + 0x20) == 0xFD) {
                    *p4 = 0;
                    continue;
                }
                i22++;
                *p5 = *(int *)(*(char **)(mm + 0x78) + 0x30);
                if (*(unsigned char *)(mm + 0x20) < 4) {
                    *(unsigned char *)(P + 0x1FF7) += 1;
                }
                p5++;
                if (!(i22 < 9)) goto L_9E4;
            }
            if (i22 < 9) {
                for (k = i22; k < 9; k++) *(int *)(P + 0x1FFC + k * 4) = 0;
            }
L_9E4:
        G = D_0013E533 + 0xE9D;
        r19 = func_L00_002CDF28(G, (int *)(G + 0x1F7C));
        if (r19 == 0) goto L_B54;
        rz = func_L00_0025C338(r19);
        qcopy_c00_2ce358(&t30, r19 + 0x10);
        pb4 = r19 + 0x10;
        if (rz) t30.v[2] = t30.v[2] + *(float *)(rz + 0x10);
        p4 = (int *)(P + 0x2020);
        for (i = 0; i < 6; i++, p4++) {
            char *mm = (char *)*p4;
            if (mm == 0) continue;
            q = *(char **)(*(char **)(mm + 0x78) + 0x30);
            if (q != 0 && *(unsigned char *)(q + 0x20) != 0xFE && *(unsigned char *)(q + 0x20) != 0xFD) continue;
            d = func_001F9D48_07408_c00_2ce358(mm + 0x10, &t30);
            if (d < f20) {
                best = mm;
                f20 = d;
            }
        }
        if (best == 0) goto L_B54;
        p2 = *(char **)(best + 0x78);
        *(int *)(p2 + 0x30) = (int)r19;
        *(int *)(p2 + 0x48) = *(short *)(r19 + 0xA6);
        r3 = func_L00_0025C338(r19);
        qcopy_c00_2ce358(&t30, pb4);
        if (r3) t30.v[2] = t30.v[2] + *(float *)(r3 + 0x10);
        qcopy_c00_2ce358(p2 + 0x10, &t30);
        *(unsigned char *)(best + 0x20) = 3;
        r2 = func_001160D8();
        rr = r2 % 3;
        func_0022DA68(rr + (rr > 0), 0, m);
        *(int *)(P + 0x1FFC + i22 * 4) = *(int *)(p2 + 0x30);
        goto L_B54;
        }
    }
L_B54:
    P = D_0013E533 + 0xE1D;
    n16 = *(unsigned char *)(P + 0x1FF6);
    idx = *(unsigned char *)(P + 0x1FF4);
    k16 = n16 + 1;
    cnt = n16 ? n16 : 1;
    if (k16 == 0) k16 = 1;
    f20 = (float)cnt;
    f22 = 6.2831855f;
    d = func_001FA6C0(idx) / f20;
    *(float *)(p + 0x40) = d * f22 - 3.1415927f;
    idx = idx * (k16 / 5);
    idx = idx % k16;
    d = func_001FA6C0(idx) / f20;
    d = (d * f22 - 3.1415927f) * 0.25f;
    *(float *)(p + 0x34) = d;
    r5 = (int)func_L00_0025A420(m, 1, 0);
    if (r5 == 0) {
        st = *(unsigned char *)(m + 0x20);
        goto L_C74;
    }
    obj = *(char **)(r5 + 0x20);
    t = 0;
    if (obj != 0) {
        if (*(char **)(obj + 0x24) != 0) t = *(short *)(*(char **)(obj + 0x24) + 0x46);
    }
    if (t >= 9) goto L_C70;
    if (t < 5) {
        st = *(unsigned char *)(m + 0x20);
        goto L_C74;
    }
    if (obj == 0 || *(short *)(obj + 0xA6) != 0x4D6) {
        *(unsigned char *)(m + 0x20) = 4;
    } else if (func_001F9770(p + 0x56) != 0) {
        *(unsigned char *)(m + 0x20) = 4;
    }
L_C70:
    st = *(unsigned char *)(m + 0x20);
L_C74:
    if (st == 3) goto L_D1BC;
    if (st < 4) {
        if (st == 2) {
            *(int *)(m + 0x94) = 0;
            goto L_CB4;
        }
        goto L_D674;
    }
    if (st == 4) goto L_D538;
    if (st == 5) goto L_D644;
    goto L_D674;
L_CB4:
    qcopy_c00_2ce358(&t30, m + 0x10);
    f20 = 0.05f;
    func_L00_002CDE78(m);
    pos = m + 0x10;
    P = D_0013E533 + 0xE1D;
    kk = D_0015ED60 * f20;
    a = *(float *)(m + 0x2C);
    b = *(float *)(*(char **)(m + 0x24) + 0x24);
    *(float *)(m + 0x2C) = a + (b - a) * kk;
    {
        float pf20 = *(float *)(p + 0x20);
        *(float *)(p + 0x20) = pf20 + (1.0f - pf20) * kk;
    }
    d = func_001FA580(*(float *)(p + 0x40), *(float *)(P + 0x1FF8));
    A = func_001FA5C8(d, *(float *)(p + 0x28));
    B2 = func_001FA580(*(float *)(p + 0x28), A * (D_0015ED60 * f20));
    *(float *)(p + 0x28) = B2;
    C = func_001FA5C8(*(float *)(p + 0x34), *(float *)(p + 0x24));
    Gf = func_001FA580(*(float *)(p + 0x24), C * (D_0015ED60 * 0.01f));
    *(float *)(p + 0x24) = Gf;
    H = func_001F9DC8(*(float *)(p + 0x28));
    t40.v[0] = H * *(float *)(p + 0x20);
    I = func_001F9DE0(*(float *)(p + 0x28));
    t40.v[1] = I * *(float *)(p + 0x20);
    t40.v[2] = 0.0f;
    t50.v[0] = 0.0f;
    t50.v[1] = *(float *)(p + 0x24);
    t50.v[2] = 0.0f;
    t50.v[3] = 0.0f;
    func_001FA050(t70.v, t50.v);
    func_001F9D20(t40.v, t40.v, t70.v);
    func_001F9A10(t60.v, P + 0x80, *(char **)(P + 0x2080) + 0xE0);
    t60.v[0] = t60.v[0] + t40.v[0];
    t60.v[1] = t60.v[1] + t40.v[1];
    {
        float zz = *(float *)(p + 0x3C);
        *(float *)(p + 0x3C) = zz + (t60.v[2] - zz) * kk;
        t60.v[2] = *(float *)(p + 0x3C) + t40.v[2];
    }
    func_001F9A28(t60.v, t60.v, pos);
    func_L00_001FF598(t60.v, t60.v, D_0015ED6C * 10.0f);
    r2 = func_L00_00258830(m, t60.v, 0.0f, 0.18f, 0.0f, 0);
    if (r2 & 1) {
        if (*(unsigned char *)(m + 0x31) == 0) {
            func_001F9A10(pos, D_L00_00166DC0, (char *)D_L00_00166DC0 + 0x230);
            func_001F9A28(pos, pos, (char *)D_L00_00166DC0 + 0x210);
        }
    }
    qcopy_c00_2ce358(&t20, pos);
    r2 = func_L00_001F0D60(0.2f, &t20, 0, m);
    if (r2 == 0) goto L_D0EC;
    G = (char *)D_L00_00173E40;
    obj = *(char **)(G + 0x18);
    n16 = 0;
    if (obj == 0) goto L_CF74;
    if (*(short *)(obj + 0xA6) == 0) {
        if (*(unsigned char *)(obj + 0x20) == 0xFE) goto L_CF54;
        if (*(unsigned char *)(obj + 0x20) != 0xFD) goto L_D0F0;
        goto L_CF54;
    }
    goto L_CF58;
L_CF54:
L_CF58:
    obj = *(char **)(G + 0x18);
    if (obj == 0) {
        n16 = 0;
        goto L_CF74;
    }
    if (*(char **)(obj + 0x24) == 0) goto L_CF78;
    n16 = *(unsigned char *)(*(char **)(obj + 0x24) + 0x46);
L_CF74:
L_CF78:
    obj = *(char **)(G + 0x18);
    if (obj == 0) goto L_D0F0;
    if (*(unsigned char *)(obj + 0x20) == 0xFE) goto L_D0F0;
    if (*(unsigned char *)(obj + 0x20) == 0xFD) goto L_D0F0;
    if (n16 == 5 || n16 == 7 || n16 == 8) goto L_CFB4;
    goto L_D0F4;
L_CFB4:
    f20 = 1.0f;
    func_L00_00259858((char *)&t40, (int)m, 0x10000, 1.0f);
    func_L00_001FF550(t40.v, p, 1.0f);
    s = (char *)&t40;
    *(float *)(s + 0x8) = 1.0f;
    *(float *)(s + 0xC) = 5627.9248046875f;
    s[0x18] = 1;
    s[0x19] = 2;
    *(unsigned short *)(s + 0x1A) = *(unsigned short *)(m + 0xA6);
    func_L00_00259A88(*(char **)(G + 0x18), s);
    if (n16 != 5) goto L_D0F0;
    obj = *(char **)(G + 0x18);
    if (obj == 0) goto L_D074;
    if (*(short *)(obj + 0xA6) != 0x4D6) goto L_D074;
    r16 = *(short *)(p + 0x56);
    if (r16 <= 0) goto L_D074;
    r2 = (int)func_001FA898_r_c00_2ce358(*(float *)((char *)&t40 + 0x1C));
    if (r2 <= 0) {
        r2 = r16 - 1;
    } else {
        r2 = r16 - (int)func_001FA898_r_c00_2ce358(*(float *)((char *)&t40 + 0x1C));
    }
    *(short *)(p + 0x56) = r2;
    goto L_D0EC;
L_D074:
    func_L00_0025F4A8_alt_c00_2ce358(m, p, 0, 0.0f, 0.0f, 0, 1, 1, 0.4f, 0.2f, 4.0f, 1.0f, 1, 8.0f, 1, 1, -1, 0);
    *(unsigned char *)(m + 0x20) = 4;
L_D0EC:
L_D0F0:
L_D0F4:
    func_001F9A28(p, pos, t30.v);
    f23 = 3.1415927f;
    f20 = 6.2831855f;
    r1 = func_L00_001FF8B0(*(float *)(m + 0x10) - t30.v[0], *(float *)(m + 0x14) - t30.v[1]);
    f21 = r1;
    r2f = func_001F9D48_07408_c00_2ce358(t30.v, pos);
    f22 = -func_L00_001FF8B0(r2f, *(float *)(m + 0x18) - t30.v[2]);
    *(float *)(m + 0x48) = func_L00_00258110((float *)(p + 0x58), *(float *)(m + 0x48), f21, D_0015ED70 * f20, D_0015ED70 * f23, D_0015ED6C * f20);
    *(float *)(m + 0x44) = func_L00_00258110((float *)(p + 0x5C), *(float *)(m + 0x44), f22, D_0015ED70 * f20, D_0015ED70 * f23, D_0015ED6C * f20);
    goto L_D674;
L_D1BC:
    cls = *(char **)(m + 0x24);
    pos = m + 0x10;
    *(int *)(m + 0x94) = *(int *)(cls + 0x10);
    qcopy_c00_2ce358(&t30, pos);
    kk = D_0015ED60 * 0.05f;
    a = *(float *)(m + 0x2C);
    b = *(float *)(cls + 0x24);
    *(float *)(m + 0x2C) = a + (b - a) * kk;
    *(int *)(m + 0x94) = *(int *)(cls + 0x10);
    func_L00_002CDE78(m);
    q = *(char **)(p + 0x30);
    if (q == 0) goto L_D4DC;
    if (*(short *)(q + 0xA6) != *(int *)(p + 0x48)) goto L_D4DC;
    if (*(unsigned char *)(q + 0x20) == 0xFE) goto L_D4D8;
    if (*(unsigned char *)(q + 0x20) == 0xFD) goto L_D4DC;
    rz = func_L00_0025C338(q);
    qcopy_c00_2ce358(&t60, *(char **)(p + 0x30) + 0x10);
    if (rz) t60.v[2] = t60.v[2] + *(float *)(rz + 0x10);
    f20 = 1.0f;
    func_001F9A28(t40.v, t60.v, p + 0x10);
    func_001F9A68(t40.v, t40.v, 3.1f);
    func_001F9A10(t50.v, t60.v, t40.v);
    func_001F9A28(p, t50.v, pos);
    func_001F9A68(p, p, D_0015ED60 * -0.8f + 1.0f);
    func_001F9A10(pos, pos, p);
    qcopy_c00_2ce358(p + 0x10, &t60);
    qcopy_c00_2ce358(&t20, pos);
    r2 = func_L00_001F0D60(0.25f, &t20, 0, m);
    if (r2 == 0) goto L_D4EC;
    G = (char *)D_L00_00173E40;
    obj = *(char **)(G + 0x18);
    if (obj == 0) goto L_D388;
    if (*(unsigned char *)(obj + 0x20) == 0xFE) goto L_D388;
    if (*(unsigned char *)(obj + 0x20) == 0xFD) goto L_D388;
    if (obj != *(char **)(p + 0x30)) goto L_D388;
    if (*(short *)(obj + 0xA6) == 0) goto L_D388;
    if (*(char **)(obj + 0x24) != 0) n18 = *(unsigned char *)(*(char **)(obj + 0x24) + 0x46);
    else n18 = 0;
    goto L_D3A8;
L_D388:
    qcopy_c00_2ce358(pos, D_L00_00173E70);
    goto L_D4F0;
L_D3A8:
    func_L00_00259858((char *)&t40, (int)m, 0x10000, 1.0f);
    func_L00_001FF550(t40.v, p, 1.0f);
    s = (char *)&t40;
    *(float *)(s + 0x8) = 1.0f;
    *(float *)(s + 0xC) = 5627.9248046875f;
    s[0x18] = 1;
    s[0x19] = 2;
    *(unsigned short *)(s + 0x1A) = *(unsigned short *)(m + 0xA6);
    func_L00_00259A88(*(char **)(G + 0x18), s);
    if (n18 != 5) goto L_D4F0;
    obj = *(char **)(G + 0x18);
    if (obj == 0) goto L_D45C;
    if (*(short *)(obj + 0xA6) != 0x4D6) goto L_D45C;
    r16 = *(short *)(p + 0x56);
    if (r16 <= 0) goto L_D45C;
    r2 = (int)func_001FA898_r_c00_2ce358(*(float *)((char *)&t40 + 0x1C));
    if (r2 <= 0) {
        r2 = r16 - 1;
    } else {
        r2 = r16 - (int)func_001FA898_r_c00_2ce358(*(float *)((char *)&t40 + 0x1C));
    }
    *(short *)(p + 0x56) = r2;
    goto L_D4EC;
L_D45C:
    func_L00_0025F4A8_alt_c00_2ce358(m, p, 0, 0.0f, 0.0f, 0, 1, 1, 0.4f, 0.2f, 4.0f, 1.0f, 1, 8.0f, 1, 1, -1, 0);
    *(unsigned char *)(m + 0x20) = 4;
    goto L_D4EC;
L_D4D8:
L_D4DC:
    *(unsigned char *)(m + 0x20) = 2;
    *(int *)(p + 0x48) = -1;
    *(int *)(p + 0x30) = 0;
L_D4EC:
L_D4F0:
    r1 = func_L00_001FF8B0(*(float *)(m + 0x10) - t30.v[0], *(float *)(m + 0x14) - t30.v[1]);
    *(float *)(m + 0x48) = r1;
    r2f = func_001F9D48_07408_c00_2ce358(t30.v, pos);
    *(float *)(m + 0x44) = -func_L00_001FF8B0(r2f, *(float *)(m + 0x18) - t30.v[2]);
    goto L_D674;
L_D538:
    q = *(char **)(p + 0x30);
    if (q == 0) {
        *(int *)(p + 0x30) = 0;
        goto L_D620;
    }
    if (*(short *)(q + 0xA6) != *(int *)(p + 0x48)) {
        *(int *)(p + 0x30) = 0;
        goto L_D620;
    }
    if (*(unsigned char *)(q + 0x20) == 0xFE) goto L_D61C;
    if (*(unsigned char *)(q + 0x20) == 0xFD) {
        *(int *)(p + 0x30) = 0;
        goto L_D620;
    }
    rz = func_L00_0025C338(q);
    qcopy_c00_2ce358(&t30, *(char **)(p + 0x30) + 0x10);
    if (rz) t30.v[2] = t30.v[2] + *(float *)(rz + 0x10);
    pos = m + 0x10;
    func_001F9A28(p, t30.v, pos);
    func_001F9A68(p, p, D_0015ED60 * -0.8f + 1.0f);
    func_L00_001FF598(p, p, D_0015ED60 * 0.9f);
    func_001F9A10(pos, pos, p);
    goto L_D624;
L_D61C:
    *(int *)(p + 0x30) = 0;
L_D620:
L_D624:
    {
        unsigned char v = *(unsigned char *)(m + 0x23) - 4;
        *(unsigned char *)(m + 0x23) = v;
        if (v < 6) *(unsigned char *)(m + 0x20) = 5;
    }
    goto L_D674;
L_D644:
    P = D_0013E533 + 0xE1D;
    *(unsigned char *)(P + 0x1FF6) = *(unsigned char *)(P + 0x1FF6) - 1;
    *(int *)(P + 0x2020 + (*(short *)(p + 0x54) << 2)) = 0;
    func_0020C828(m);
    return;
L_D674:
    func_L00_00259FE8((unsigned char *)m, 0.17f);
    return;
}
