/* NON_MATCHING func_L00_002DD3D8 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 2596 / retail 2576, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Spray/effect builder for a moby (2576 bytes, four loop bodies plus tail). p3.c is the best run: 2528 bytes aga
 */
extern char D_L00_001EA430[];
extern char D_L00_001EA480[];
extern char D_L00_001EA498[];
extern float D_0015EE6C MACRO_ADDR;
extern char D_L00_00166EC0[] NOT_SDA;
extern unsigned char D_0013E15A[];
extern char D_L00_00166D80[];
extern int *D_L00_00178000[];
extern float func_001FA888(int);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BF0(void *, void *, void *);
extern float func_001F9CB8(void *);
extern float func_002140F8(float, float);
extern float func_001F9C78(void *a, void *b);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9850(int);
extern int func_L00_00258BC8(int, int);
extern char *func_L00_002B0738(char *, char *, int, int, int);
extern void func_L00_001FF548(void *, void *, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_002140B0(int);
extern unsigned func_L00_0025D140(unsigned, int);
extern void func_L00_0026B890(void *, void *, int, int, float, int, int, int, int, float);
extern void func_L00_0025D0E0(int *, int *, int *, int);
extern void func_L00_002D4CE8(void *, void *, int, int);
extern int func_L00_001F2BE8_2FB898(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern void func_L00_0025BA50(void *, void *, void *, int, int, int, int, int, float, float, float);

typedef int u128_DD3D8 __attribute__((mode(TI)));
typedef struct { u128_DD3D8 v[5]; } B50;
typedef struct { int v[6]; } I6;

/* Builds the spray of effect sprites around self: a set of sampled points along the moby's path, each sprite placed by a vector step and drawn through the level tables. */
void func_L00_002DD3D8(char *self, char *q) {
    B50 blk;
    float a[4];
    float b[4];
    float c[4];
    float d[4];
    float e[4];
    float g[4];
    I6 t1;
    I6 t2;
    int tA[3];
    int tB[3];
    char *pos;
    unsigned char *flag;
    float f20, f21, f22, f23, f24, dt;
    int step, n, i, k, h, ii, r, r1, r2, A, B, cnt;
    float two;

    *(B50 *)&blk = *(B50 *)D_L00_001EA430;
    two = D_0015EE6C + D_0015EE6C;
    flag = D_0013E15A + 0x4C6;
    pos = self + 0x10;
    b[0] = 0.0f;
    b[1] = 0.0f;
    b[2] = two;
    b[3] = 0.0f;
    f20 = 1.0f;
    c[0] = 0.0f;
    c[1] = 0.0f;
    c[2] = f20;
    c[3] = 0.0f;
    f22 = func_001FA888(flag[9]) + f20;
    step = (flag[9] == 0) ? 2 : 1;
    func_001F9C30(b, b, f22);
    func_001F9BF0(a, D_L00_00166EC0, pos);
    f23 = func_001F9CB8(a);
    n = 10 / step;
    i = 0;
    if (n > 0) {
        f21 = -1.0f;
        do {
            *(u128_DD3D8 *)g = 0;
            g[0] = func_002140F8(f21, f20);
            i++;
            g[1] = func_002140F8(f21, f20);
            *(u128_DD3D8 *)e = *(u128_DD3D8 *)g;
            dt = func_001F9C78(e, c);
            func_L00_001FF4B0(d, c, dt);
            func_001F9BF0(e, e, d);
            func_L00_001FF4B0(d, c, func_002140F8(0.0f, f20));
            func_001F9BD8(e, e, d);
            func_L00_001FF4B0(e, e, f22 * func_002140F8(3.5f, 6.5f) * D_0015EE6C);
            func_001F9BD8(e, e, b);
            h = func_001F9850(60);
            ii = func_L00_00258BC8(h, func_001F9850(120));
            func_L00_002B0738(pos, (char *)e, ii, 0, flag[9]);
        } while (i < 10 / step);
    }
    cnt = 4 / step;
    i = 0;
    if (cnt > 0) {
        f21 = -1.0f;
        do {
            *(u128_DD3D8 *)g = 0;
            g[0] = func_002140F8(f21, f20);
            i++;
            g[1] = func_002140F8(f21, f20);
            *(u128_DD3D8 *)e = *(u128_DD3D8 *)g;
            dt = func_001F9C78(e, c);
            func_L00_001FF4B0(d, c, dt);
            func_001F9BF0(e, e, d);
            func_L00_001FF4B0(d, c, func_002140F8(0.0f, f20));
            func_001F9BD8(e, e, d);
            func_L00_001FF4B0(e, e, f22 * func_002140F8(6.5f, 10.0f) * D_0015EE6C);
            func_001F9BD8(e, e, b);
            h = func_001F9850(60);
            ii = func_L00_00258BC8(h, func_001F9850(90));
            func_L00_002B0738(pos, (char *)e, ii, 0, flag[9]);
        } while (i < 4 / step);
    }
    f20 = 1.0f;
    f21 = -1.0f;
    f24 = f23 + f23;
    *(u128_DD3D8 *)g = 0;
    g[0] = func_002140F8(f21, f20);
    g[1] = func_002140F8(f21, f20);
    g[2] = func_002140F8(f21, f20);
    *(u128_DD3D8 *)e = *(u128_DD3D8 *)g;
    a[2] += f23 * 0.5f;
    func_L00_001FF4B0(e, e, (f23 / 5.0f) * D_0015EE6C);
    func_L00_001FF4B0(a, a, f24 * D_0015EE6C);
    func_001F9BD8(e, e, a);
    func_L00_001FF548(e, e, D_0015EE6C * 10.0f);
    h = func_001F9850(60);
    ii = func_L00_00258BC8(h, func_001F9850(90));
    func_L00_002B0738(pos, (char *)e, ii, 0, flag[9]);
    k = 4;
    if (f23 < 6.0f) {
        k = func_001FA898_r(f23) + 1;
    }
    f24 = (f23 < 7.0f) ? 7.0f - f23 : 0.0f;
    if (k > 0) {
        cnt = k;
        do {
            f20 = func_002140F8(8.0f, 10.0f);
            cnt--;
            f21 = f22 * f20;
            f20 = D_0015EE6C;
            *(I6 *)&t1 = *(I6 *)D_L00_001EA480;
            *(I6 *)&t2 = *(I6 *)D_L00_001EA498;
            f21 = f21 * f20;
            f21 = f21 - f24 * D_0015EE6C;
            A = func_L00_0025D140(t1.v[func_002140B0(6)], flag[9]);
            B = func_L00_0025D140(t2.v[func_002140B0(6)], flag[9]);
            f20 = f22 * 400000.0f;
            r = func_001F9850(15);
            r1 = func_L00_00258BC8(r, func_001F9850(20));
            r2 = func_L00_00258BC8(func_001F9850(25), func_001F9850(30));
            func_L00_0026B890(pos, b, A, B, f20, r1, r2, 0, 0, f21);
        } while (cnt != 0);
    }
    if (f23 > 9.0f) {
        f20 = 4.0f * f22;
        func_L00_002E0CB8(self, pos, b, f20, func_001F9850(15), 0x7F, 0x7F, 0x7F, 0x20);
        tA[0] = 0x7F;
        tA[1] = 0x20;
        tA[2] = 0;
        func_L00_0025D0E0(&tA[0], &tA[1], &tA[2], flag[9]);
        func_L00_002E0CB8(self, pos, b, f20, func_001F9850(24), tA[0], tA[1], tA[2], 0x20);
    }
    tB[0] = 0x7F;
    tB[1] = 0x3F;
    tB[2] = 0;
    func_L00_0025D0E0(&tB[0], &tB[1], &tB[2], flag[9]);
    f20 = 4.0f * f22;
    func_L00_002E0CB8(self, pos, b, f20, func_001F9850(20), tB[0], tB[1], tB[2], 0x30);
    tB[0] = 0x60;
    tB[1] = 0x10;
    tB[2] = 0;
    func_L00_0025D0E0(&tB[0], &tB[1], &tB[2], flag[9]);
    f20 = 3.5f * f22;
    func_L00_002E0CB8(self, pos, b, f20, func_001F9850(27), tB[0], tB[1], tB[2], 0x40);
    tB[0] = 0x20;
    tB[1] = 0;
    tB[2] = 0;
    func_L00_0025D0E0(&tB[0], &tB[1], &tB[2], flag[9]);
    f20 = 3.0f * f22;
    func_L00_002E0CB8(self, pos, b, f20, func_001F9850(29), tB[0], tB[1], tB[2], 0x20);
    if (f23 < 20.0f) {
        *(float *)(D_L00_00166D80 + 0x160) = 0.4f - f23 * 0.0175f;
    } else {
        *(float *)(D_L00_00166D80 + 0x160) = 0.05f;
    }
    *(int *)(D_L00_00166D80 + 0x168) = func_001F9850(25);
    func_L00_002D4CE8(&blk, pos, 0, 0);
    if (flag[9]) {
        int n2 = func_L00_001F2BE8_2FB898(3.0f, pos, 0x10, self, 0);
        *(u128_DD3D8 *)e = *(u128_DD3D8 *)(self + 0x10);
        func_L00_0025BA50(self, e, D_L00_00178000, n2, 0, 0x830000, 2, 1, 2.0f, 1.0f, 1.0f);
    }
}
