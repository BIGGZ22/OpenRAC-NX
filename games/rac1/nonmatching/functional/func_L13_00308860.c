/* functional: high. Gemlik vendor moby effect: builds a frame vector from the moby's position and the level data, spreads 10 + 4 + 1 particle mobys (func_L13_0030D600) in random directions around an up vector, a counted run of func_L00_0026B890 sparks (count from the distance f22) and up to five func_L00_002E0CB8 glows. corrected from the previous copy: it did not compile (u128, missing declarations, an undeclared float helper); behaviour re-checked against the assembly call by call (constants, argument order, loop counts), no differences found. */
extern float D_0015EE6C_8860 __asm__("D_0015EE6C");
extern char D_L13_001F5850_8860[] __asm__("D_L13_001F5850");
extern char D_L13_001F5868_8860[] __asm__("D_L13_001F5868");
extern char D_L13_00167140_8860[] __asm__("D_L13_00167140");
extern float dot_8860(void *, void *) __asm__("func_001F9C78");
extern void vscale_8860(void *, void *, float) __asm__("func_001F9C30");
extern void vsub_8860(void *, void *, void *) __asm__("func_001F9BF0");
extern void vadd_8860(void *, void *, void *) __asm__("func_001F9BD8");
extern float vlen_8860(void *) __asm__("func_001F9CB8");
extern int scale_8860(int) __asm__("func_001F9850");
extern float rndf_8860(float, float) __asm__("func_002140F8");
extern int rndi_8860(int) __asm__("func_002140B0");
extern int rndr_8860(int, int) __asm__("func_L00_00258BC8");
extern int f2i_8860(float) __asm__("func_001FA898");
extern void norm_8860(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void norm2_8860(void *, void *, float) __asm__("func_L00_001FF548");
extern void part_8860(void *, void *, int, int) __asm__("func_L13_0030D600");
extern void spark_8860(void *, void *, int, int, float, int, int, int, int, float) __asm__("func_L00_0026B890");
extern void glow_8860(void *, void *, void *, float, int, int, int, int, int) __asm__("func_L00_002E0CB8");

void func_L13_00308860(char *moby) {
    float a[4], b[4], up[4], e[4], c[4], p[4], v[4];
    int t1[6], t2[6];
    float f22, f23, f20, f21, r;
    int i, n, s16, s19, s9, x;
    int *p18, *p17;
    float dt = D_0015EE6C_8860;
    int q;

    /* b = {0,0,2*dt,0} scaled by 1.5; up = {0,0,1,0} */
    b[0] = 0.0f; b[1] = 0.0f; b[2] = dt + dt; b[3] = 0.0f;
    up[0] = 0.0f; up[1] = 0.0f; up[2] = 1.0f; up[3] = 0.0f;
    for (q = 0; q < 4; q++) c[q] = ((float *)(moby + 0x10))[q];
    c[2] = c[2] + 1.0f;
    vscale_8860(b, b, 1.5f);
    vsub_8860(a, D_L13_00167140_8860, c);
    f22 = vlen_8860(a);

    i = 9;
    do {
        v[0] = rndf_8860(-1.0f, 1.0f);
        i--;
        v[1] = rndf_8860(-1.0f, 1.0f);
        v[2] = 0.0f; v[3] = 0.0f;
        for (q = 0; q < 4; q++) p[q] = v[q];
        norm_8860(e, up, dot_8860(p, up));
        vsub_8860(p, p, e);
        r = rndf_8860(0.0f, 1.0f);
        norm_8860(e, up, r);
        vadd_8860(p, p, e);
        r = rndf_8860(3.5f, 6.5f) * 1.5f;
        norm_8860(p, p, r * dt);
        vadd_8860(p, p, b);
        s16 = scale_8860(0x3C);
        x = scale_8860(0x78);
        part_8860(c, p, rndr_8860(s16, x), 0);
    } while (i >= 0);

    f23 = f22 + f22;
    i = 3;
    do {
        v[0] = rndf_8860(-1.0f, 1.0f);
        i--;
        v[1] = rndf_8860(-1.0f, 1.0f);
        v[2] = 0.0f; v[3] = 0.0f;
        for (q = 0; q < 4; q++) p[q] = v[q];
        norm_8860(e, up, dot_8860(p, up));
        vsub_8860(p, p, e);
        r = rndf_8860(0.0f, 1.0f);
        norm_8860(e, up, r);
        vadd_8860(p, p, e);
        r = rndf_8860(6.5f, 10.0f) * 1.5f;
        norm_8860(p, p, r * dt);
        vadd_8860(p, p, b);
        s16 = scale_8860(0x3C);
        x = scale_8860(0x5A);
        part_8860(c, p, rndr_8860(s16, x), 1);
    } while (i >= 0);

    v[0] = rndf_8860(-1.0f, 1.0f);
    v[1] = rndf_8860(-1.0f, 1.0f);
    v[2] = rndf_8860(-1.0f, 1.0f);
    v[3] = 0.0f;
    for (q = 0; q < 4; q++) p[q] = v[q];
    a[2] = a[2] + f22 * 0.5f;
    norm_8860(p, p, (f22 / 5.0f) * dt);
    norm_8860(a, a, f23 * dt);
    vadd_8860(p, p, a);
    norm2_8860(p, p, dt * 10.0f);
    s16 = scale_8860(0x3C);
    x = scale_8860(0x5A);
    part_8860(c, p, rndr_8860(s16, x), 1);

    if (f22 < 6.0f) {
        n = f2i_8860(f22) + 1;
    } else {
        n = 4;
    }
    f23 = 0.0f;
    if (f22 < 7.0f) {
        f23 = 7.0f - f22;
    }

    if (n > 0) {
        i = n;
        do {
            f21 = 1.5f * 400000.0f;
            r = rndf_8860(8.0f, 10.0f);
            i--;
            f20 = 1.5f * r;
            for (q = 0; q < 6; q++) t1[q] = ((int *)D_L13_001F5850_8860)[q];
            f20 = f20 * dt;
            for (q = 0; q < 6; q++) t2[q] = ((int *)D_L13_001F5868_8860)[q];
            f20 = f20 - f23 * dt;
            p18 = &t1[rndi_8860(6)];
            p17 = &t2[rndi_8860(6)];
            s16 = scale_8860(0xF);
            x = scale_8860(0x14);
            s19 = rndr_8860(s16, x);
            s16 = scale_8860(0x19);
            x = scale_8860(0x1E);
            s9 = rndr_8860(s16, x);
            spark_8860(c, b, *p18, *p17, f21, s19, s9, 0, 0, f20);
        } while (i != 0);
    }

    if (9.0f < f22) {
        f20 = 1.5f * 4.0f;
        glow_8860(moby, c, b, f20, scale_8860(0xF), 0x7F, 0x7F, 0x7F, 0x20);
        glow_8860(moby, c, b, f20, scale_8860(0x18), 0x20, 0x7F, 0, 0x20);
    }

    f20 = 1.5f * 4.0f;
    glow_8860(moby, c, b, f20, scale_8860(0x14), 0x3F, 0x7F, 0, 0x30);
    f20 = 1.5f * 3.5f;
    glow_8860(moby, c, b, f20, scale_8860(0x1B), 0x10, 0x60, 0, 0x40);
    f20 = 1.5f * 3.0f;
    glow_8860(moby, c, b, f20, scale_8860(0x1D), 0, 0x20, 0, 0x20);
}
