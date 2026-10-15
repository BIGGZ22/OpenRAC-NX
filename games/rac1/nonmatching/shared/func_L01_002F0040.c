/* NON_MATCHING func_L01_002F0040 -- src/overlays/shared/vendor_002B90A8.c
 * Best so far: SIZE ours 1752 / retail 1768, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Moby update: aims a moby at a target (func_L00_0025B478, the aim calls 001FA898/0025BBA0/0025D5B0), runs the L
 *   Best candidate p5.c: SIZE 1736 against retail's 1768 (32 bytes short). Closed so far: the cvt.w.s round trip (
 *   Unblock: a regalloc dump for overlay functions (tools/regalloc.py printed nothing here) to see which local tak
 */
typedef int Q910 __attribute__((mode(TI)));
typedef union { Q910 q; float f[4]; } U910;
extern short D_L01_00160058;
extern void func_0022ED80(int, int, char *);
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern void func_L00_0025BBA0(void *, float *, void *, void *);
extern void func_L00_0025D5B0(void *, void *, float, int, int, int);
extern void func_L00_002584A8(void *, int, int);
extern int func_L00_00260FB0(float, char *, void *, int, int, void *, int);
extern float func_001FA790(float, float);
extern char *func_L01_002F0728(void *);
extern void func_L01_002F0B48(void *, void *);
extern char *func_L01_002F0850(char *, int);
extern int func_001FA8A8(int, int, float);
extern int D_L01_00161350 MACRO_ADDR;
extern short D_L01_00161A28;
extern short D_L01_00161A2C;
extern short D_L01_00161A30;
extern short D_L01_00161A34;
/* Moby update: picks a target, runs the aim and blend steps, writes the moby's state (1768 bytes). */
void func_L01_002F0040(void *mp) {
    char *m = (char *)mp;
    char *d;
    char *t;
    char *p;
    char *x;
    char *w;
    char *t2;
    char *p16;
    char *b;
    char *e;
    int c5;
    int r;
    int r2;
    int cls;
    int v2;
    int i_2;
    int idx;
    float f20;
    float f21;
    float f22;
    float q;
    float z;
    float f0;
    U910 V;
    U910 W;
    int i20;
    float f24;
    float f28;
    float f2c;

    d = *(char **)(m + 0x78);
    *(int *)&f24 = 0;
    t = func_L00_0025B478(m, 0x330000, 0);
    if (t == 0) goto L184;
    V.q = *(Q910 *)(m + 0x10);
    V.f[2] = V.f[2] + 0.5f;
    p = *(char **)(t + 0x20);
    if (p == 0) goto L134;
    cls = *(short *)(p + 0xA6);
    if (cls == 0x47) {
        W.q = *(Q910 *)(D_0013E633 + 0xEED);
        w = (char *)&W;
        goto L144;
    }
    x = *(char **)(p + 0x24);
    if (x == 0) goto L104;
    if (*(short *)(x + 0x46) == 5) goto L10C;
L104:
    if (cls != 0xBA) goto L120;
L10C:
    W.q = V.q;
    w = (char *)&W;
    goto L144;
L120:
    W.q = *(Q910 *)(p + 0x10);
    w = (char *)&W;
    goto L144;
L134:
    W.q = *(Q910 *)t;
    w = (char *)&W;
L144:
    r = func_001F9D10(&V, w);
    if (!(r < 24.0f)) goto L184;
    r2 = func_L00_001EFFF0(&V, w, 2, (int)m, 0);
    if (r2 != 0) t = 0;
L184:
    func_L00_0025B4D0(m, t, d + 0x20, 0, &i20, &f24, 0, 4);
    if (i20 == 1) goto L4A4;
    if (f24 == 0.0f) goto L4A8;
    if (*(unsigned char *)(m + 0x20) == 0xB) goto L4A4;
    if (*(unsigned char *)(m + 0x20) == 0xA) goto L4A4;
    if (*(unsigned char *)(m + 0x20) == 8) goto L4A4;
    f20 = 0.8f;
    func_0022ED80(0, 0, m);
    f22 = 1024.0f;
    f21 = 0.008f;
    f0 = *(float *)(d + 0x250) * f20;
    *(float *)(d + 0x20) = *(float *)(d + 0x20) - 1.0f;
    *(unsigned char *)(d + 0x9D) = 0;
    *(float *)(d + 0x70) = f21;
    *(int *)(d + 0x84) = 8;
    *(int *)(d + 0x80) = func_001FA898_r(f0 * f22);
    *(float *)(d + 0x88) = *(float *)(d + 0x250) * f20;
    func_L01_002F0B48(m, t + 0x10);
    if (!(0.0f < *(float *)(d + 0x20))) goto L48C;

    t2 = func_L01_002F0728(m);
    if (t2 == 0) goto L35C;
    w = *(char **)(t2 + 0x78);
    *(unsigned char *)(w + 0x9D) = 3;
    *(float *)(w + 0x70) = f21;
    *(int *)(w + 0x84) = 8;
    *(int *)(w + 0x80) = func_001FA898_r(*(float *)(d + 0x250) * f20 * f22);
    *(float *)(w + 0x88) = *(float *)(d + 0x250) * f20;
    *(float *)(w + 0x78) = *(float *)&D_L01_00161A30 * D_0015EE6C;
    *(float *)(w + 0xB0) = 7.0f;
    *(float *)(w + 0xB4) = 13.0f;
    *(float *)(w + 0x7C) = *(float *)&D_L01_00161A34 * D_0015EE6C;
    *(int *)(w + 0x84) = *(int *)(w + 0x84) | 4;
    V.q = *(Q910 *)(t + 0x10);
    func_L00_0025BBA0(&V, &f28, w + 0x78, w + 0x7C);
    f28 = func_001FA790(f28, 0.7853982f);
    func_L00_0025D5B0(t2, w + 0x60, f28, 4, 8, 0);
    *(float *)(t2 + 0x58) = 1.0f;
    *(unsigned char *)(t2 + 0x20) = 8;

L35C:
    t2 = func_L01_002F0728(m);
    if (t2 == 0) goto L43C;
    w = *(char **)(t2 + 0x78);
    *(float *)(w + 0x70) = f21;
    *(unsigned char *)(w + 0x9D) = 3;
    *(int *)(w + 0x84) = 8;
    *(int *)(w + 0x80) = func_001FA898_r(*(float *)(d + 0x250) * f20 * f22);
    *(float *)(w + 0x88) = *(float *)(d + 0x250) * f20;
    *(float *)(w + 0x78) = *(float *)&D_L01_00161A30 * D_0015EE6C;
    *(float *)(w + 0xB0) = 7.0f;
    *(float *)(w + 0xB4) = 13.0f;
    *(float *)(w + 0x7C) = *(float *)&D_L01_00161A34 * D_0015EE6C;
    *(int *)(w + 0x84) = *(int *)(w + 0x84) | 4;
    V.q = *(Q910 *)(t + 0x10);
    func_L00_0025BBA0(&V, &f2c, w + 0x78, w + 0x7C);
    f2c = func_001FA748(f2c, 0.7853982f);
    func_L00_0025D5B0(t2, w + 0x60, f2c, 4, 8, 0);
    *(float *)(t2 + 0x58) = 1.0f;
    *(unsigned char *)(t2 + 0x20) = 8;

L43C:
    p = *(char **)(t + 0x20);
    if (p == 0) goto L478;
    x = *(char **)(p + 0x24);
    if (x == 0) goto L47C;
    if (*(short *)(x + 0x46) != 5) goto L47C;
    func_L00_002584A8(m, 0x800, -1);
    goto L500;
L478:
    c5 = 0;
L47C:
    func_L00_002584A8(m, c5, -1);
    goto L500;

L48C:
    *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
    func_0020D678(m);
    return;

L4A4:
L4A8:
    *(unsigned char *)(m + 0xA4) = 0xFF;
    if (*(int *)(d + 0x258) == 0) goto L510;
    if (*(unsigned char *)(m + 0x20) == 0) goto L510;
    if (!(*(float *)(m + 0x18) < *(float *)((char *)(*(int *)(d + 0x258) * 0x1190 + D_L01_00161350) + 8))) goto L510;
    func_001F9BC0(&V);
    func_L01_002F0B48(m, &V);
L500:
    func_0020D678(m);
    return;

L510:
    v2 = *(int *)(d + 0x38);
L514:
    if (v2 == 0) goto L52C;
    *(int *)(d + 0x22C) = func_001F9850(240);
    *(int *)(d + 0x38) = 0;

L52C:
    r = func_001F9908(d + 0x22C);
    if (r == 0) goto L544;
    *(float *)(d + 0x228) = *(float *)(d + 0x224);
    goto L558;
L544:
    *(float *)(d + 0x228) = *(float *)(d + 0x224) + 20.0f;

L558:
    if (*(int *)(d + 0x260) <= 0) goto L578;
    if (*(unsigned char *)((*(int *)(d + 0x260) << 8) + *(int *)&D_L01_00160058 + 0xBC) != 0) goto L578;
    c5 = *(int *)(d + 0x248);
    goto L610;

L578:
    idx = *(int *)(d + 0x24C);
    e = D_L01_001B0C30[idx];
    b = d + 0x1C0;
    r = func_L00_00260FB0(*(float *)(d + 0x228), m, b, 0, 0, e + 0x10, *(int *)e);
    if (r == 2) goto L60C;
    f0 = func_001F9D48(d + 0x210, b);
    if (*(float *)(d + 0x228) < f0) {
        *(int *)(d + 0x204) = 2;
        goto L60C;
    }
    f0 = func_001F9B88(*(float *)(m + 0x18) - *(float *)(d + 0x1C8));
    if (!(3.0f < f0)) {
        c5 = *(int *)(d + 0x248);
        goto L610;
    }
    *(int *)(d + 0x204) = 2;

L60C:
    c5 = *(int *)(d + 0x248);

L610:
    if (c5 < 0) {
        i_2 = *(int *)(d + 0x200);
        goto L688;
    }
    p16 = func_L01_002F0850(m, c5);
    if (p16 == 0) goto L684;
    f20 = func_001F9D48(m + 0x10, p16 + 0x10);
    if (!(f20 < *(float *)(d + 0x228))) goto L684;
    if (*(int *)(d + 0x204) == 2) goto L678;
    f0 = func_001F9D48(m + 0x10, *(char **)(d + 0x200) + 0x10);
    if (!(f20 < f0)) {
        i_2 = *(int *)(d + 0x200);
        goto L688;
    }

L678:
    *(char **)(d + 0x200) = p16;
    *(int *)(d + 0x204) = 1;

L684:
    i_2 = *(int *)(d + 0x200);

L688:
    f0 = D_0015EE6C;
    if (i_2 != 0) goto L6A4;
    *(int *)(d + 0x200) = *(int *)(D_0013E633 + 0x2E9D);
    f0 = D_0015EE6C;

L6A4:
    q = func_001FA748(*(float *)(d + 0x254), f0 * 9.424778f);
    *(float *)(d + 0x254) = q;
    z = func_001F9FA8(q);
    *(int *)(m + 0x90) = func_001FA8A8(*(int *)&D_L01_00161A28, *(int *)&D_L01_00161A2C, (z + 1.0f) * 0.5f);
    return;
}
