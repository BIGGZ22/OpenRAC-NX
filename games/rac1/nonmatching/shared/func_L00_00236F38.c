/* NON_MATCHING func_L00_00236F38 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 1380 / retail 1384, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   HUD update: sets rec+0x58/0x5C from two gp globals, calls 00236400/00236468, then draws up to 100 sprite pairs
 *   Still differing: which saved register holds each pointer and the flag order at the top, the add.s placement in
 *   Unblock: a source order that gets the 4 missing bytes (likely an extra store or branch shape in the i==0 case)
 */
extern int func_L00_00236400(HudElem *rec, int *x, int *y);
extern void func_L00_00236468(HudElem *, int *, int *, int, int);
extern void func_00201640(int, int, int, int, long, int);
extern int func_001F9850(int);
extern float func_001F9878(float);
extern int func_L00_00217570(int, int);
extern int func_00200198(int, int);
extern void func_001F9BC0(float *);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern void func_L00_001FF2F8(float *, float *, float *);
extern void func_001F9BD8(float *, float *, float *);
extern void func_002008B8(int, int, int, int, int, int);
extern void func_L00_0023C458(int tex, int x, int y, int w, int h, int alpha);
extern int D_L00_0015F4F8 MACRO_ADDR;
extern float D_L00_0017E760[][4];
extern int D_L00_0015F840 SDATA(D_L00_0015F840);
extern int D_L00_0015F844 SDATA(D_L00_0015F844);
extern float D_L00_0015F848 SDATA(D_L00_0015F848);
extern float D_L00_0015F84C SDATA(D_L00_0015F84C);
extern short D_L00_0015F830;
extern short D_L00_0015F834;
extern short D_L00_0015F838;

// HUD element update: places sprite pairs along a 100-step arc
int func_L00_00236F38(HudElem *rec) {
    char *r = (char *)rec;
    int xs, ys;
    int x, y, a, b, n, i, flagA, flagB, s, t21, tex, w;
    float v0[4], v10[4], v20[4], v30[4], v40[4], v50[4];
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f20, f21, f22, fA, fB;

    x = *(int *)(r + 0x50);
    y = *(int *)(r + 0x54);
    a = D_L00_0015F840;
    b = D_L00_0015F844;
    *(int *)(r + 0x58) = a;
    *(int *)(r + 0x5C) = b;
    func_L00_00236400(rec, &x, &y);
    func_L00_00236468(rec, &x, &y, *(int *)(r + 0x6C), 0);

    a = D_L00_0015F840;
    b = D_L00_0015F844;
    {
        int X1 = (x + *(short *)(r + 0x48)) * 16 + (208 * a) / 64;
        int Y1 = (y + *(short *)(r + 0x4A)) * 16 + (160 * b) / 256;
        int X2 = X1 + (608 * a) / 64;
        int Y2 = Y1 + (3744 * b) / 256;
        func_00201640(X1, Y1, X2, Y2, 0x80684C2F, 1);
    }

    n = *(int *)(r + 0x74);
    if (n * 3 < 10001) {
        s = func_001F9850(1);
        f0 = func_001F9878(0.75f);
        flagA = (D_L00_0015F4F8 - 1) % (int)((float)s * 60.0f) < (int)(f0 * 60.0f);
    } else {
        flagA = 0;
    }
    n = *(int *)(r + 0x74);
    if (n * 3 < 10001) {
        s = func_001F9850(1);
        f0 = func_001F9878(0.75f);
        flagB = D_L00_0015F4F8 % (int)((float)s * 60.0f) < (int)(f0 * 60.0f);
    } else {
        flagB = 0;
    }
    if (!flagA && flagB == 1) {
        func_L00_00217570(0xB, 0);
    }
    w = 0;
    tex = func_00200198(*(int *)r, flagB);
    f20 = 16.0f;
    f22 = 32.0f;
    xs = x + *(short *)(r + 0x48);
    ys = y + *(short *)(r + 0x4A);
    func_L00_0023C458(tex, xs, ys, D_L00_0015F840, D_L00_0015F844, 0x80);

    fB = (float)D_L00_0015F844;
    fA = (float)D_L00_0015F840;
    n = *(int *)(r + 0x74);
    f4 = D_L00_0015F84C;
    f3 = fB * f4;
    f2 = 1.0f;
    f1 = D_L00_0015F848;
    f7 = (float)(10000 - n);
    f5 = 0.5f;
    f6 = f2 - f1;
    f1 = fA * f1;
    f0 = 10000.0f;
    f4 = f2 - f4;
    f3 = f3 * f5;
    f21 = f7 / f0;
    xs = x + *(short *)(r + 0x48);
    ys = y + *(short *)(r + 0x4A);
    f1 = f1 * f5;
    f4 = f4 * f5;
    f8 = 116.0f;
    f0 = (float)xs;
    f9 = 17.0f;
    f6 = f6 * f5;
    f7 = (float)ys;
    v0[2] = f2;
    v0[3] = f2;
    f5 = -f3;
    f0 = f1 + f0;
    v10[2] = 0.0f;
    v10[3] = 0.0f;
    f3 = f3 + f7;
    f6 = f6 * fA;
    f4 = f4 * fB;
    f1 = f1 / f9;
    f5 = f5 / f8;
    f0 = f0 + f6;
    f3 = f3 + f4;
    f2 = f21 + f21;
    f0 = f0 * f20;
    f3 = f3 * f20;
    f1 = f1 * f20;
    f2 = f2 * f8;
    v10[0] = f0;
    v10[1] = f3;
    v0[0] = f1;
    f5 = f5 * f20;
    v0[1] = f5;
    f21 = f8 - f2;
    t21 = func_00200198(*(int *)r, 2);

    func_001F9BC0(v20);
    func_001F9BC0(v30);
    func_001F9BC0(v40);
    f0 = *(float *)&D_L00_0015F830;
    f2 = *(float *)&D_L00_0015F834;
    f1 = *(float *)&D_L00_0015F838;
    f3 = f0 * f20;
    f0 = f2 - f0;
    f1 = f1 - f2;
    v20[0] = f3;
    v20[1] = f3;
    f0 = f0 * f20;
    v30[0] = f0;
    v30[1] = f0;
    f1 = f1 * f20;
    v40[0] = f1;
    v40[1] = f1;

    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9BF0(v10, v10, v20);
            f0 = *(float *)&D_L00_0015F830;
            f21 = f21 + f0;
            f1 = f0 * f22;
            w = (int)f1;
        } else if (i == 10) {
            func_001F9BF0(v10, v10, v30);
            f0 = *(float *)&D_L00_0015F834;
            f1 = *(float *)&D_L00_0015F830;
            f2 = f0 * f22;
            f0 = f0 - f1;
            w = (int)f2;
            f21 = f21 + f0;
        } else if (i == 30) {
            func_001F9BF0(v10, v10, v40);
            f0 = *(float *)&D_L00_0015F838;
            f1 = *(float *)&D_L00_0015F834;
            f2 = f0 * f22;
            f0 = f0 - f1;
            w = (int)f2;
            f21 = f21 + f0;
        }
        if (f21 < D_L00_0017E760[i][1]) continue;
        func_L00_001FF2F8(v50, D_L00_0017E760[i], v0);
        func_001F9BD8(v50, v50, v10);
        func_002008B8(t21, (int)v50[0], (int)v50[1], w, w, 0x80);
    }
    return *(int *)(r + 0x58);
}
