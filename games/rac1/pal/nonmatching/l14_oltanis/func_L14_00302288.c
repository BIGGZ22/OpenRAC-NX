/* NON_MATCHING func_L14_00302288 -- src/overlays/l14_oltanis/vendor_002FF358.c
 * Best so far: SIZE ours 1772 / retail 1756, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - Stopped (hq9/s20, 5 runs). Function: moby update for class 1193: first call decides state 0..3 (lbu 0x20), s
 *   - Best so far: p1/p2 (ours 1780 vs 1756): retail's constants f20..f23 are set once before the loop and saved i
 *   - Unblock: decide the register of moby+0x10 vs sp+0x20 (regalloc.py on p1), then the blend clamp order.
 */
extern int func_L00_001F10E0(float, void *, int, void *);
extern int func_001F9908_r(int *arg0) __asm__("func_001F9908");
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9938(void *);
extern void func_L00_0025C710(void *, void *, void *, float);
extern int func_002140B0(int);
extern int func_001F9850(int);
extern float func_001FA888(int);
extern char *func_L00_0026DEA0(void *, int, void *, int, float, float, float, float);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_L00_001FF610(void *, void *, void *);
extern void func_0022ED80(int, int, int);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9CE8(void *);
extern void func_L00_001FF500(void *, void *, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_0020D678(void *);
extern float func_001FA748(float, float);
extern int D_L14_00174658;
extern float D_L14_0015F660[] MACRO_ADDR;
extern char D_L14_00174640[];
extern float D_0015EE6C MACRO_ADDR;
extern char D_L14_00162178[];
extern float D_0015EE60 MACRO_ADDR;
extern short D_L14_0016215C;
extern short D_L14_00162158;
extern short D_L14_00162160;
extern short D_L14_00162164;
extern short D_L14_00162170;
extern short D_L14_00162174;

typedef int u128_00302288 __attribute__((mode(TI)));
typedef struct { char b[12]; } blk12;

/* Level 14 moby update, class 1193: steps the moby through its state machine. */
void func_L14_00302288(unsigned char *moby) {
    unsigned char *data = *(unsigned char **)(moby + 0x78);
    float v20[4], v30[4], v40[4];
    float k1, k5000, k45000, k1_0, f20, f21;
    int i, r, q, st, h;
    char *p;
    char *src;
    float f0, f1, f3;

    if (func_L00_001F10E0(0.65f, moby + 0x10, 0, moby)) {
        int g = D_L14_00174658;
        if (g != 0 && g != *(int *)(data + 0x24)) moby[0x20] = 3;
    }
    if (func_001F9908_r((int *)(data + 0x20))) moby[0x20] = 3;

    st = moby[0x20];
    if (st == 2) goto L3026EC;
    if (st >= 3) {
        if (st == 3) goto L302800;
        goto L302878;
    }
    if (st < 0) goto L302874;

    func_001F9BD8(v20, moby + 0x10, data + 0x10);
    *(float *)(data + 0x18) = *(float *)(data + 0x18) - *(float *)(data + 0x1C);
    if (moby[0x20] == 1 && *(float *)(data + 0x18) < 0.0f) moby[0x20] = 0;

    func_001F9938(data + 0x28);
    if (*(short *)(data + 0x28) != 0) {
        k1_0 = 1.0f;
        k1 = 0.333299994f;
        k45000 = 45000.0f;
        k5000 = 5000.0f;
        for (i = 0; i < 3; i++) {
            *(u128_00302288 *)v40 = *(u128_00302288 *)(moby + 0x10);
            func_L00_0025C710(v30, v40, v20, (float)i * k1);
            r = func_002140B0(6);
            func_002140B0(2);
            q = func_001F9850(*(int *)&D_L14_0016215C);
            if (q) r = -r;
            f0 = func_001FA888(q);
            h = *(short *)(data + 0x28);
            p = func_L00_0026DEA0(v30, r, D_L14_0015F660, 0x80808080, 0.025f, k1_0, k1_0,
                                  ((float)h / f0) * k45000 + k5000);
            if (p != 0) {
                unsigned char *w = (unsigned char *)p + 0x20;
                *(short *)(p + 0xA) = func_001F9850(0x78);
                w[0xA] = 0x7F;
                *(int *)(w + 4) = 2;
                w[0xB] = *(unsigned char *)(p + 0xA);
            }
        }
    }

L3024CC:
    r = func_L00_001EFFF0(moby + 0x10, v20, 0, moby, 0);
    src = (char *)v20;
    if (r) {
        int d18 = *(int *)(D_L14_00174640 + 0x18);
        src = D_L14_00174640 + 0x20;
        if (d18 != 0 && d18 == *(int *)(data + 0x24)) src = (char *)v20;
    }
    *(u128_00302288 *)(moby + 0x10) = *(u128_00302288 *)src;
    r = func_L00_001F10E0(*(float *)&D_L14_00162158, moby + 0x10, 0, moby);
    if (r == 0) goto L302874;
    if (moby[0x20] == 0) goto L30255C;
    f3 = *(float *)(data + 0x18) - *(float *)(D_L14_00174640 + 0x28);
    if (!(f3 < 0.0f)) goto L302874;

L30255C:
    if (*(int *)(D_L14_00174640 + 0x18) == 0) goto L302578;
    if (*(int *)(D_L14_00174640 + 0x18) == *(int *)(data + 0x24)) goto L302874;

L302578:
    func_L00_001FF4B0(v30, D_L14_00174640 + 0x40, *(float *)&D_L14_00162158);
    func_001F9BD8(moby + 0x10, D_L14_00174640 + 0x20, v30);
    func_L00_001FF610(data + 0x10, data + 0x10, D_L14_00174640 + 0x40);
    func_0022ED80(1, 0, (int)moby);
    if (moby[0x20] == 0) {
        int u;
        moby[0x20] = 1;
        u = *(unsigned short *)(data + 0x2A) + 1;
        *(short *)(data + 0x2A) = u;
        if ((short)u == 1) {
            f0 = *(float *)&D_L14_00162174 * D_0015EE6C;
            func_L00_001FF500(data + 0x10, data + 0x10, f0);
            *(float *)(data + 0x18) = *(float *)(data + 0x18) * *(float *)&D_L14_00162170;
        } else {
            func_001F9C30(v40, data + 0x10, *(float *)&D_L14_00162170);
            f0 = func_001F9CE8(v40);
            if (f0 < *(float *)&D_L14_00162160 * D_0015EE6C) {
                func_L00_001FF500(data + 0x10, data + 0x10, *(float *)&D_L14_00162160 * D_0015EE6C);
                *(float *)(data + 0x18) = *(float *)(data + 0x18) * *(float *)&D_L14_00162170;
            } else {
                *(u128_00302288 *)(data + 0x10) = *(u128_00302288 *)v40;
            }
        }
    }
    h = *(short *)(data + 0x2A);
    if (h < 4) goto L302874;
    moby[0x20] = 2;
    *(int *)(data + 0x18) = 0;
    f0 = func_001F9CE8(data + 0x10);
    f1 = f0 * *(float *)&D_L14_00162170;
    f3 = *(float *)&D_L14_00162160 * D_0015EE6C;
    *(float *)(data + 0x1C) = f1;
    if (f1 < f3) *(float *)(data + 0x1C) = f3;
    func_L00_001FF4B0(data + 0x10, data + 0x10, *(float *)(data + 0x1C));
    f0 = func_001FA888(*(int *)(data + 0x20));
    *(float *)(data + 0x2C) = 1.0f / f0;
    goto L302874;

L3026EC:
    *(blk12 *)v20 = *(blk12 *)D_L14_00162178;
    f20 = (float)*(int *)(data + 0x20) * *(float *)(data + 0x2C);
    f21 = 0.0f;
    {
        int ra = func_001FA898_r((v20[0] - 255.0f) * f20 + 255.0f);
        int rb = func_001FA898_r((v20[1] - f21) * f20 + f21);
        int rc = func_001FA898_r((v20[2] - f21) * f20 + f21);
        *(int *)(moby + 0x90) = (rc << 16) | 0xFF000000 | (rb << 8) | ra;
    }
    f1 = (*(float *)&D_L14_00162164 - 1.0f) * D_0015EE60;
    f3 = *(float *)&D_L14_00162160 * D_0015EE6C;
    f0 = *(float *)(data + 0x1C) * (f1 + 1.0f);
    *(float *)(data + 0x1C) = f0;
    if (f0 < f3) *(float *)(data + 0x1C) = f3;
    p = (char *)(data + 0x10);
    func_L00_001FF4B0(p, p, *(float *)(data + 0x1C));
    func_001F9BD8(moby + 0x10, moby + 0x10, p);
    goto L302874;

L302800:
    func_L00_0025F4A8(moby, data + 0x10, moby + 0x10, 2.0f, 1.0f, 5, 2, 4, 2.0f, 1.0f, 9.0f, 1.0f, 0, 15.0f, 1, 1, -1, 0);
    func_0020D678(moby);
    return;

L302874:
    st = moby[0x20];
L302878:
    if (st >= 2) goto L302894;
    f3 = func_001F9CE8(data + 0x10);
    goto L3028A0;

L302894:
    if (st != 2) return;
    f3 = *(float *)(data + 0x1C);

L3028A0:
    f0 = *(float *)&D_0015EE6C;
    f1 = f0 * 0.200000003f;
    if (f3 < f1) f3 = f1;
    f0 = f0 * 6.0f;
    if (f0 < f3) f3 = f0;
    f1 = f0 - f1;
    f3 = f3 - *(float *)&D_0015EE6C * 0.200000003f;
    *(float *)(moby + 0x44) = func_001FA748((((f3 / f1) * 7.0f) + 1.0f) * 0.0174532924f, *(float *)(moby + 0x44));
    return;
}
