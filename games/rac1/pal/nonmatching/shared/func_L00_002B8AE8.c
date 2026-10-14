/* NON_MATCHING func_L00_002B8AE8 -- src/overlays/shared/vendor_002B33E8.c
 * Best so far: SIZE ours 2684 / retail 2624, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Pyrocitor update (class 176): state machine on m[0x20] with a 2.6 KB body, many float constants and gp-relativ
 *   Not run further: after three runs the first differences were in the save set and the frame, so a rewording wou
 */
typedef int u128_B8AE8 __attribute__((mode(TI)));

extern void func_L00_002B9528(unsigned char *m);
extern void func_L00_00250800(void *, int, void *);
extern float func_0020D830(void *);
extern int func_001F9850_4198(int) __asm__("func_001F9850");
extern int func_L00_002BC668_2b58d8(void *) __asm__("func_L00_002BC668");
extern void func_001F9C08(void *, void *, void *, float);
extern float func_001F9CB8_2b58d8(void *) __asm__("func_001F9CB8");
extern void func_L00_0023F1D0(int);
extern int func_L00_00234718(int);
extern void func_L00_0020EB60(void);
extern int func_L00_0023EF78(float *, float, float, int);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9FA8(float);
extern float func_001FA888(int);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern float func_001F9D10(void *, void *);
extern float func_00214D88(float *, float *, float, float, float, float);
extern void func_L00_00258DB0(float *, float, float);
extern float func_002140F8(float, float);
extern void func_001F9C30(void *, void *, float);
extern int func_L00_0026C0D0(void *, void *, float, int);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_001F2BE8(void *, float, int, void *, void *);
extern int func_001F9938(void *);
extern float func_001F9878(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_L00_0026E438(char *, char *, int);
extern int func_L00_00234638(int, int);
extern void func_L00_0020ED30(void);
extern void func_L00_0028EBF0(int);
extern void func_001FA1F8(void *, void *);
extern void func_001FA4F0(void *, void *, void *);
extern void func_L00_00251E30_2b58d8(void *) __asm__("func_L00_00251E30");

extern char D_0013E15A[];
extern unsigned char D_L00_001803C0[];
extern unsigned char D_L00_001803D0[];
extern unsigned char D_L00_001670F0[];
extern unsigned char D_L00_00173F60[];
extern float D_L00_0015F660[] MACRO_ADDR;
extern int D_L00_0015F6B0_i __asm__("D_L00_0015F6B0") MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_L00_001602A8 MACRO_ADDR;
extern short D_L00_001615C0;
extern short D_L00_001615D0;
extern short D_L00_001615E0;
extern short D_L00_001615E4;
extern short D_L00_001615E8;
extern short D_L00_001615EC;
extern short D_L00_001615F0;
extern short D_L00_001615F4;
extern short D_L00_001615F8;
extern short D_L00_00161600;

typedef struct {
    float v[4];
    float a;
    float b;
    unsigned char *m;
    int x;
    unsigned char c;
    float f;
    int i;
} VCs;

/* Pyrocitor update: steps the moby's state machine, moves its effect points and runs the per-frame blend. */
void func_L00_002B8AE8(unsigned char *m) {
    unsigned char *d;
    unsigned char *p;
    unsigned char *e;
    int st;
    int t;
    int r;
    int hh;
    int i;
    int n;
    int n98;
    int n9c;
    float fa;
    float f20;
    float f22;
    float f1v;
    float f0g;
    float f90;
    float f94;
    float *p4;
    float buf[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    unsigned char *G2;
    unsigned char *H;
    unsigned char *R;
    unsigned char *Q;
    unsigned char *P;
    unsigned char *W;
    VCs vc;

    d = *(unsigned char **)(m + 0x78);
    func_L00_002B9528(m);
    func_L00_00250800(m, 0, buf);
    st = m[0x20];
    if (st == 1) goto L8B8C;
    if (st < 2) {
        if (st == 0) {
            *(short *)(d + 0x4A) = -1;
            *(short *)(d + 0x44) = -1;
            m[0x20] = 1;
            goto L94A4;
        }
        goto L94A8;
    }
    if (st == 2) goto L8C18;
    if (st == 3) goto L8D94;
    goto L94A8;

L8B8C:
    if (m[0x52] == st) {
        m[0x20] = 2;
    }
    t = m[0x52];
    if (t != 0) goto L94A8;
    fa = func_0020D830(m);
    if (!(10.0f < fa)) goto L94A4;
    if ((*(int *)(D_0013A5E0 + 0x2600) & *(int *)(D_0013E633 + 0x1EBD)) == 0) goto L94A8;
    if (m[0x53] == st) goto L94A8;
    func_00213DE0_B8208(m, 1, 0, func_001F9850_4198(5));
    goto L94A8;

L8C18:
    if (*(void **)d == 0) {
        p = (unsigned char *)func_L00_002BC668_2b58d8(m);
        if (p != 0) {
            *(unsigned char **)d = p;
            *(unsigned short *)(p + 0x34) = *(unsigned short *)(p + 0x34) | 0x100;
            *(float *)(*(unsigned char **)(p + 0x78) + 0xC) = *(float *)&D_L00_001615E0;
        }
    }
    hh = *(short *)(d + 0x44);
    if (hh == -1) goto L8CC8;
    e = D_L00_001803C0 + (hh << 5);
    func_001F9C08(e, e, D_L00_0015F660, 0.1f);
    hh = *(short *)(d + 0x44);
    e = D_L00_001803C0 + (hh << 5);
    if (func_001F9CB8_2b58d8(e) < 0.001f) {
        func_L00_0023F1D0(*(short *)(d + 0x44));
        *(short *)(d + 0x44) = -1;
    }
    goto L8CCC;

L8CC8:
L8CCC:
    P = D_0013A5E0 + 0x2460;
    Q = D_0013E633 + 0xE1D;
    if ((*(int *)(P + 0x1A0) & *(int *)(Q + 0x10A0)) == 0) goto L8D54;
    if (*(unsigned char *)(Q + 0x20AC) != 0) goto L8D58;
    r = func_L00_00234718(-1);
    if (r == 0) goto L8D50;
    m[0x20] = 3;
    func_L00_0020EB60();
    t = func_001F9850_4198(10);
    *(short *)(d + 0x4C) = (short)t;
    if (*(short *)(d + 0x44) != -1) goto L94A4;
    *(short *)(d + 0x44) = (short)func_L00_0023EF78((float *)(m + 0x10), 7.5f, 0.0f, 0);
    goto L94A4;

L8D50:
    P = D_0013A5E0 + 0x2460;
L8D54:
    Q = D_0013E633 + 0xE1D;
L8D58:
    if ((*(int *)(P + 0x1A4) & *(int *)(Q + 0x10A0)) == 0) goto L94A8;
    if (func_L00_00234718(-1) != 0) goto L94A8;
    func_0022ED80_B8208(0, 0, m);
    goto L94A8;

L8D94:
    r = func_L00_0028EB98(m, 0);
    if (r == 0) {
        r = func_0022ED80_B8208(1, 4, m);
        *(short *)(d + 0x4A) = (short)r;
    }
    hh = *(short *)(d + 0x44);
    if (hh == -1) goto L8EE0;
    if (m[0x20] != st) goto L8EE4;
    v10[0] = 2.0f;
    v10[1] = 0.0f;
    v10[2] = 2.0f;
    v10[3] = 0.0f;
    R = D_0013E633 + 0x145D;
    func_001F9EC0(v10, v10, R);
    hh = *(short *)(d + 0x44);
    H = D_L00_001803D0;
    func_001F9BD8(H + (hh << 5), R - 0x5C0, v10);
    G2 = D_L00_001803D0 - 0x10;
    hh = *(short *)(d + 0x44);
    *(float *)(G2 + (hh << 5) + 0x1C) = 7.5f;
    {
        int x = D_L00_0015F6B0_i % 120;
        float fv = ((float)x / 120.0f) * 6.2831802f - 3.1415901f;
        float g = func_001F9FA8(fv);
        float h = g * 0.5f + 0.5f;
        func_001F9C08(v20, &D_L00_001615C0, &D_L00_001615D0, h);
    }
    hh = *(short *)(d + 0x44);
    func_001F9C08(G2 + (hh << 5), G2 + (hh << 5), v20, 0.1f);

L8EE0:
L8EE4:
    W = D_0013E15A + 0x4C6;
    f90 = 0.0f;
    f94 = 0.0f;
    fa = func_001FA888(W[0x10]);
    n98 = 1;
    {
        int z4 = *(int *)(D_0013E633 + 0x2EA1);
        int t3 = 1;
        if ((D_L00_0015F6B0_i & 3) != 0) t3 = 0;
        f22 = fa * 4.0f;
        n98 = t3;
        if (z4 != 1) goto L8FA8;
        if ((*(unsigned short *)(m + 0x34) & 1) == 0) goto L8FA8;
    }
    H = D_L00_001670F0;
    func_L00_001FF4B0(v20, H, -1.0f);
    func_001F9BD8(v20, v20, H - 0x230);
    f0g = *(float *)&D_L00_001615F0 + f22;
    func_L00_001FF4B0(v10, H - 0x20, f0g + 0.75f);
    func_001F9BD8(v30, v10, v20);
    goto L8FEC;

L8FA8:
    *(u128_B8AE8 *)v20 = *(u128_B8AE8 *)buf;
    f0g = *(float *)&D_L00_001615F0 + f22;
    func_L00_001FF4B0(v10, D_0013E633 + 0x148D, f0g + 0.75f);
    func_001F9BD8(v30, v10, buf);

L8FEC:
    r = func_L00_001EFFF0(D_0013E633 + 0xEED, v20, 2, 0, 0);
    if (r != 0) {
        f20 = 0.0f;
    } else {
        r = func_L00_001EFFF0(v20, v30, 2, 0, 0);
        if (r == 0) {
            f20 = *(float *)&D_L00_001615F0 + f22;
        } else {
            f20 = func_001F9D10(v20, D_L00_00173F60) - 0.75f;
            if (f20 < 0.0f) f20 = 0.0f;
        }
    }
    f1v = D_L00_001602A8;
    p4 = (float *)&f90;
    f0g = *(float *)&D_L00_001615E4;
    func_00214D88(p4, &f94, f20, 1.0f, f1v * D_0015EE70, f0g * D_0015EE6C);
    func_L00_001FF4B0(v10, v10, f94);
    if (f20 < 4.0f || (*(unsigned short *)(m + 0x34) & 1)) {
        n9c = 1;
    } else {
        n9c = *(int *)&D_L00_001615E8;
    }
    n = *(int *)&D_L00_001615EC + n9c;
    if (n > 0) {
        for (i = 0; i < n; i++) {
            float x0 = *(float *)&D_L00_001615F0 + f22;
            float z = (*(float *)&D_L00_001615F8);
            float y = *(float *)&D_L00_001615F4 - z;
            float q = f20 / x0;
            float w = (y * q + z) * D_0015EE6C;
            func_L00_00258DB0(v50, 0.0f, w);
            func_001F9BD8(v50, v50, v10);
            {
                float s = func_002140F8(0.0f, 1.0f);
                func_001F9C30(v40, v10, s);
            }
            func_001F9BD8(v40, v40, v20);
            {
                int c6 = ((i < n9c) ? 1 : 0) | 4;
                r = func_L00_0026C0D0(v40, v50, f20, c6);
            }
            if (i == 2 && n98 != 0) {
                func_L00_002B9A90(m, r);
            }
        }
    }
    v40[0] = -0.3f;
    v40[1] = 0.0f;
    v40[2] = 0.0f;
    v40[3] = 0.0f;
    R = D_0013E633 + 0x145D;
    func_001F9EC0(v50, v40, R);
    W = D_0013E15A + 0x4C6;
    func_001F9BD8(v50, v50, buf);
    Q = R - 0x640;
    fa = func_001FA888(W[0x10]);
    vc.m = m;
    vc.x = 0x10000;
    *(u128_B8AE8 *)vc.v = *(u128_B8AE8 *)v10;
    vc.f = fa + f20;
    vc.a = f20;
    vc.b = 5627.9248046875f;
    vc.c = 5;
    vc.i = 1;
    func_L00_001FF500(v50, &vc, f20);
    func_L00_001F2BE8(v50, 0.42f, 5, *(void **)(Q + 0x2080), &vc);
    v40[0] = -0.8f;
    func_001F9EC0(v50, v40, R);
    func_001F9BD8(v50, v50, buf);
    func_L00_001F2BE8(v50, 0.42f, 5, *(void **)(Q + 0x2080), &vc);
    if (func_001F9938(d + 0x4E) != 0) {
        float q1 = func_002140F8(10.0f, 30.0f);
        float q2 = func_001F9878(q1);
        *(short *)(d + 0x4E) = (short)func_001FA898_r(q2);
        func_L00_00258DB0(v50, 0.0f, D_0015EE6C + D_0015EE6C);
    }
    {
        float q3 = func_002140F8(0.5f, 0.1f);
        func_001F9C30(v40, v10, q3);
        func_001F9BD8(v40, v40, v50);
        func_L00_0026E438((char *)v20, (char *)v40, W[0x10]);
    }
    r = func_001F9850_4198(D_L00_00161600);
    if (r != 0) {
        if (*(int *)&D_L00_0015F6B0_i % r != 0) goto L93F8;
    }
    func_L00_00234638(-1, 1);

L93F8:
    func_L00_00234718(-1);
    r = func_001F9938(d + 0x4C);
    if (r == 0) goto L94A4;
    if ((*(int *)(D_0013A5E0 + 0x2600) & *(int *)(Q + 0x10A0)) == 0) goto L9440;
    if (*(unsigned char *)(Q + 0x20AC) != 0) goto L9440;
    if (r != 0) goto L94A8;
L9440:
    m[0x20] = 2;
    func_L00_0020ED30();
    {
        int t5 = *(short *)(d + 0x4A);
        if (t5 != -1) {
            unsigned char *e3 = D_0013E633 + 0x1D + t5 * 0x70;
            if (*(unsigned char **)(e3 + 0x88) == m && *(unsigned char *)(e3 + 0x74) != 0) {
                func_L00_0028EBF0(t5);
            }
        }
    }
    *(short *)(d + 0x4A) = -1;
    if (r == 0) func_0022ED80_B8208(0, 0, m);

L94A4:
L94A8:
    p = *(unsigned char **)d;
    if (p == 0) goto L94E4;
    func_001FA1F8(p + 0xC0, p + 0x40);
    p = *(unsigned char **)d;
    func_001FA4F0(p + 0xC0, m + 0xC0, p + 0xC0);
    p = *(unsigned char **)d;
    *(u128_B8AE8 *)(p + 0x10) = *(u128_B8AE8 *)buf;
    func_L00_00251E30_2b58d8(p);

L94E4:
    func_L00_002B9730(m);
}
