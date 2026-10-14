/* functional: medium. Gemlik vendor moby effect: builds a frame vector from the moby's position and the level data, then spreads 10 + 4 particle mobys (func_L13_0030D600) in random directions around an up vector, one larger burst, a counted run of func_L00_0026B890 sparks (count from the distance f22) and up to five func_L00_002E0CB8 glows. Started from the staged near miss; compared statically, not run on real calls. */
extern char D_L13_001F5850[];
extern char D_L13_001F5868[];
extern char D_L13_00167140[];
extern float func_001F9C78(void *a, void *b);
extern char *func_L13_0030D600(void *a, void *b, int c, int d);
extern void func_L00_0026B890(void *, void *, int, int, float, int, int, int, int, float);
extern void func_L00_001FF548(void *, void *, float);
extern char *func_L00_002E0CB8(void *, void *, void *, float, int, unsigned char, unsigned char, unsigned char, int);
typedef struct {
    char b[24];
} __attribute__((packed)) blk24;

/* Gemlik vendor moby: spreads particle mobys around the vendor in three passes, then emits four fixed bursts. */
void func_L13_00308860_impl(char *moby) __asm__("func_L13_00308860");
void func_L13_00308860_impl(char *moby) {
    float a[4];
    float b[4];
    float up[4];
    float e[4];
    float c[4];
    float p[4];
    float v[4];
    int t1[6];
    int t2[6];
    float f22;
    float f23;
    float f20;
    float f21;
    float r;
    float dot;
    int i;
    int n;
    int s16;
    int s19;
    int s9;
    int x;
    int *p18;
    int *p17;
    float one = 1.0f;
    float neg;
    float k15 = 1.5f;
    float big;
    float t;

    *(u128 *)up = 0;
    up[2] = D_0015EE6C + D_0015EE6C;
    *(u128 *)b = *(u128 *)up;
    *(u128 *)up = 0;
    up[2] = one;
    qcopy(c, moby + 0x10);
    c[2] = c[2] + one;
    func_001F9C30(b, b, k15);
    func_001F9BF0(a, D_L13_00167140, c);
    f22 = func_001F9CB8(a);
    neg = -1.0f;

    i = 9;
    do {
        float neg = -1.0f;
        float one = 1.0f;
        *(u128 *)v = 0;
        v[0] = func_002140F8(neg, one);
        i--;
        v[1] = func_002140F8(neg, one);
        qcopy(p, v);
        dot = func_001F9C78(p, up);
        func_L00_001FF4B0(e, up, dot);
        func_001F9BF0(p, p, e);
        r = func_002140F8(0.0f, one);
        func_L00_001FF4B0(e, up, r);
        func_001F9BD8(p, p, e);
        r = func_002140F8(3.5f, 6.5f) * k15;
        func_L00_001FF4B0(p, p, r * D_0015EE6C);
        func_001F9BD8(p, p, b);
        s16 = func_001F9850(0x3C);
        x = func_001F9850(0x78);
        func_L13_0030D600(c, p, func_L00_00258BC8(s16, x), 0);
    } while (i >= 0);

    f23 = f22 + f22;
    i = 3;
    do {
        float neg = -1.0f;
        float one = 1.0f;
        *(u128 *)v = 0;
        v[0] = func_002140F8(neg, one);
        i--;
        v[1] = func_002140F8(neg, one);
        qcopy(p, v);
        dot = func_001F9C78(p, up);
        func_L00_001FF4B0(e, up, dot);
        func_001F9BF0(p, p, e);
        r = func_002140F8(0.0f, one);
        func_L00_001FF4B0(e, up, r);
        func_001F9BD8(p, p, e);
        r = func_002140F8(6.5f, 10.0f) * k15;
        func_L00_001FF4B0(p, p, r * D_0015EE6C);
        func_001F9BD8(p, p, b);
        s16 = func_001F9850(0x3C);
        x = func_001F9850(0x5A);
        func_L13_0030D600(c, p, func_L00_00258BC8(s16, x), 1);
    } while (i >= 0);

    *(u128 *)v = 0;
    v[0] = func_002140F8(neg, one);
    v[1] = func_002140F8(neg, one);
    v[2] = func_002140F8(neg, one);
    qcopy_nc(p, v);
    a[2] = a[2] + f22 * 0.5f;
    func_L00_001FF4B0(p, p, (f22 / 5.0f) * D_0015EE6C);
    func_L00_001FF4B0(a, a, f23 * D_0015EE6C);
    func_001F9BD8(p, p, a);
    func_L00_001FF548(p, p, 10.0f * D_0015EE6C);
    s16 = func_001F9850(0x3C);
    x = func_001F9850(0x5A);
    func_L13_0030D600(c, p, func_L00_00258BC8(s16, x), 1);

    if (f22 < 6.0f) {
        n = func_001FA898_r(f22) + 1;
    } else {
        n = 4;
    }
    f23 = (f22 < 7.0f) ? 7.0f - f22 : 0.0f;

    big = 400000.0f;
    if (n > 0) {
        i = n;
        do {
            float neg = -1.0f;
            f21 = k15 * big;
            r = func_002140F8(8.0f, 10.0f);
            i--;
            f20 = k15 * r;
            t = f23 * D_0015EE6C;
            *(blk24 *)t1 = *(blk24 *)D_L13_001F5850;
            f20 = f20 * D_0015EE6C;
            *(blk24 *)t2 = *(blk24 *)D_L13_001F5868;
            f20 = f20 - t;
            p18 = &t1[func_002140B0(6)];
            p17 = &t2[func_002140B0(6)];
            s16 = func_001F9850(0xF);
            x = func_001F9850(0x14);
            s19 = func_L00_00258BC8(s16, x);
            s16 = func_001F9850(0x19);
            x = func_001F9850(0x1E);
            s9 = func_L00_00258BC8(s16, x);
            func_L00_0026B890(c, b, *p18, *p17, f21, s19, s9, 0, 0, f20);
        } while (i != 0);
    }

    if (9.0f < f22) {
        f20 = k15 * 4.0f;
        func_L00_002E0CB8(moby, c, b, f20, func_001F9850(0xF), 0x7F, 0x7F, 0x7F, 0x20);
        func_L00_002E0CB8(moby, c, b, f20, func_001F9850(0x18), 0x20, 0x7F, 0, 0x20);
    }

    f20 = k15 * 4.0f;
    func_L00_002E0CB8(moby, c, b, f20, func_001F9850(0x14), 0x3F, 0x7F, 0, 0x30);
    f20 = k15 * 3.5f;
    func_L00_002E0CB8(moby, c, b, f20, func_001F9850(0x1B), 0x10, 0x60, 0, 0x40);
    f20 = k15 * 3.0f;
    func_L00_002E0CB8(moby, c, b, f20, func_001F9850(0x1D), 0, 0x20, 0, 0x20);
}
