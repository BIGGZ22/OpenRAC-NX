/* func_L10_002EA578 -- src/overlays/shared/vendor_00299AF0.c (functional C for the port, not a match)
 * Draw callback (queued through func_001F49B0; levels 10 and 17) of a moby drawn as stacked pairs of
 * textured quads: the moby's matrix (rotation from +0xC0, position, w 1) places two quads whose
 * corners come from D_L10_001DA620 and D_L10_001DA5E0, scaled in x and z by the parent's pvars
 * (+0x00, +0x04). For each of the D_L10_00161FC0 layers it sets the GS alpha/test tag from the
 * D_L10_00161FC8..FE0 pairs (by layer parity), colours from D_L10_001DA520, wobbles the first quad's
 * uvs (D_L10_001DA5C0 plus sin/cos of a per-layer frame phase, D_L10_001DA540/580), derives the
 * second quad's uvs from them, draws both, and raises both quads by 0.1 for the next layer.
 * The source file declares this function `void (void)`; the definition takes its real argument
 * under another C name. equiv: NEAR (only which call a leftover argument register is credited to). */
typedef unsigned int Q_2EA578 __attribute__((mode(TI), aligned(16)));
typedef struct {
    float v[4][4];
    int col[4];
    float uv[4][2];
    long pk[4];
} Quad_2EA578;

extern Q_2EA578 D_L10_001DA620_2EA578[] __asm__("D_L10_001DA620");
extern Q_2EA578 D_L10_001DA5E0_2EA578[] __asm__("D_L10_001DA5E0");
extern int D_L10_001DA520_2EA578[] __asm__("D_L10_001DA520");
extern float D_L10_001DA540_2EA578[][2] __asm__("D_L10_001DA540");
extern int D_L10_001DA580_2EA578[][2] __asm__("D_L10_001DA580");
extern float D_L10_001DA5C0_2EA578[][2] __asm__("D_L10_001DA5C0");
extern int D_L10_0015F4F8_2EA578 __asm__("D_L10_0015F4F8") MACRO_ADDR;
extern int D_L10_00161FC0_2EA578 SDATA(D_L10_00161FC0);
extern int D_L10_00161FC8_2EA578[2] __asm__("D_L10_00161FC8__gp") MACRO_ADDR;
extern int D_L10_00161FD0_2EA578[2] __asm__("D_L10_00161FD0__gp") MACRO_ADDR;
extern int D_L10_00161FD8_2EA578[2] __asm__("D_L10_00161FD8__gp") MACRO_ADDR;
extern int D_L10_00161FE0_2EA578[2] __asm__("D_L10_00161FE0__gp") MACRO_ADDR;

extern void func_001FA460_2EA578(void *, void *) __asm__("func_001FA460");
extern long func_001F4868_2EA578(int) __asm__("func_001F4868");
extern int func_001FA898_2EA578(float) __asm__("func_001FA898");
extern float func_001FA888_2EA578(int) __asm__("func_001FA888");
extern float func_001F9F90_2EA578(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2EA578(float) __asm__("func_001F9FA8");
extern void func_L00_001FD1D8_2EA578(void *, void *, int) __asm__("func_L00_001FD1D8");

void func_L10_002EA578_m(char *m) __asm__("func_L10_002EA578");
void func_L10_002EA578_m(char *m) {
    Quad_2EA578 a;
    Quad_2EA578 b;
    float mtx[4][4];
    char *d = *(char **)(m + 0x78);
    int i, j, k, h, t, col, par;
    float f, s;

    func_001FA460_2EA578(mtx, m + 0xC0);
    qcopy(mtx[3], m + 0x10);
    mtx[3][3] = 1.0f;
    a.pk[1] = func_001F4868_2EA578(0x2C);
    a.pk[2] = 0xFF9000000260L;
    a.pk[0] = 0;
    b.pk[1] = func_001F4868_2EA578(0x2C);
    b.pk[2] = 0xFF9000000260L;
    b.pk[0] = 0;
    b.col[3] = 0;
    b.col[1] = 0;
    {
        Q_2EA578 *sa = D_L10_001DA620_2EA578;
        Q_2EA578 *sb = D_L10_001DA5E0_2EA578;
        float *pa = a.v[0];
        float *pb = b.v[0];
        float *qb = b.v[0];
        for (k = 3; k >= 0; k--) {
            qcopy(pa, sa);
            qcopy(qb, sb);
            pa[0] = pa[0] * *(float *)(d + 0);
            pb[0] = pb[0] * *(float *)(d + 0);
            pa[2] = pa[2] * *(float *)(d + 4);
            pb[2] = pb[2] * *(float *)(d + 4);
            sa++;
            sb++;
            qb += 4;
            pa += 4;
            pb += 4;
        }
    }

    for (i = 0; i < D_L10_00161FC0_2EA578; i++) {
        par = i & 1;
        a.pk[3] = (long)D_L10_00161FC8_2EA578[par] | ((long)D_L10_00161FD0_2EA578[par] << 2) |
                  ((long)D_L10_00161FD8_2EA578[par] << 4) | ((long)D_L10_00161FE0_2EA578[par] << 6) |
                  0x8000000000L;
        b.pk[3] = (long)D_L10_00161FC8_2EA578[par] | ((long)D_L10_00161FD0_2EA578[par] << 2) |
                  ((long)D_L10_00161FD8_2EA578[par] << 4) | ((long)D_L10_00161FE0_2EA578[par] << 6) |
                  0x8000000000L;
        col = D_L10_001DA520_2EA578[i];
        a.col[0] = col;
        b.col[2] = col;
        b.col[0] = col;
        a.col[3] = col;
        a.col[2] = col;
        a.col[1] = col;
        for (j = 0; j < 4; j++) {
            for (h = 0; h < 2; h++) {
                t = D_L10_0015F4F8_2EA578 % func_001FA898_2EA578(D_L10_001DA540_2EA578[i][h] * 360.0f);
                f = func_001FA888_2EA578(t) / D_L10_001DA540_2EA578[i][h] - 180.0f;
                if (D_L10_001DA580_2EA578[i][h] != 0) {
                    s = func_001F9F90_2EA578(f * 0.017453292f);
                } else {
                    s = func_001F9FA8_2EA578(f * 0.017453292f);
                }
                a.uv[j][h] = D_L10_001DA5C0_2EA578[j][h] + s * 0.125f;
            }
        }
        for (h = 0; h < 2; h++) {
            f = (float)h * 0.125f;
            b.uv[0][h] = a.uv[1][h];
            b.uv[1][h] = a.uv[1][h] + f;
            b.uv[2][h] = a.uv[3][h];
            b.uv[3][h] = a.uv[3][h] + f;
        }
        func_L00_001FD1D8_2EA578(&a, mtx, 0);
        func_L00_001FD1D8_2EA578(&b, mtx, 0);
        {
            float *pa = a.v[0];
            float *pb = b.v[0];
            for (k = 3; k >= 0; k--) {
                pa[1] = pa[1] + 0.1f;
                pb[1] = pb[1] + 0.1f;
                pa += 4;
                pb += 4;
            }
        }
    }
}
