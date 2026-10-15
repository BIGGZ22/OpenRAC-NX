/* func_L00_002A62D0 -- src/overlays/shared/vendor_002A5138.c (functional C for the port, not a match)
 * Draw callback of the pulsing light-beam moby (queued through func_001F49B0 by func_L00_002A6A38).
 * Scrolls the beam's texture offset (D_L00_0016145C), draws four quads of the beam from the
 * D_L00_001D7660 corner table (indices D_L00_001D7760, colours D_L00_001D7738, uvs D_L00_001D76F0/F4)
 * rotated by the camera-facing matrix and scaled by the parent's 0x90; then grows a glow quad with
 * D_L00_00161460 (alpha fading as it grows) and puts four sparkles on a ring around the moby.
 * The source file declares this function `void (void)`; the definition takes its real argument
 * under another C name. First pass shape from an earlier worker's attempt.
 * equiv: NEAR (only which call a leftover argument register is credited to). */
typedef unsigned int Q_2A62D0 __attribute__((mode(TI), aligned(16)));
typedef union { Q_2A62D0 q; float f[4]; } V_2A62D0;
typedef struct {
    float v[4][4];
    int col[4];
    float uv[4][2];
} Quad_2A62D0;

extern char D_L00_00166D80_2A62D0[] __asm__("D_L00_00166D80");
extern float D_L00_0016145C_2A62D0 SDATA(D_L00_0016145C);
extern float D_L00_00161460_2A62D0 SDATA(D_L00_00161460);
extern float D_L00_001D7660_2A62D0[][4] __asm__("D_L00_001D7660");
extern float D_L00_001D76F0_2A62D0[][2] __asm__("D_L00_001D76F0");
extern float D_L00_001D76F4_2A62D0[][2] __asm__("D_L00_001D76F4");
extern int D_L00_001D7738_2A62D0[] __asm__("D_L00_001D7738");
extern int D_L00_001D7760_2A62D0[][4] __asm__("D_L00_001D7760");

extern void func_001FA1F8_2A62D0(void *, void *) __asm__("func_001FA1F8");
extern long func_001F4868_2A62D0(int) __asm__("func_001F4868");
extern void func_001F9EC0_2A62D0(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_001F9BD8_2A62D0(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FD1D8_2A62D0(void *, int, int) __asm__("func_L00_001FD1D8");
extern float func_001F9F90_2A62D0(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2A62D0(float) __asm__("func_001F9FA8");
extern void func_L00_00264690_2A62D0(void *, int, float, float) __asm__("func_L00_00264690");

void func_L00_002A62D0_m(char *m) __asm__("func_L00_002A62D0");
void func_L00_002A62D0_m(char *m) {
    char *p = *(char **)(m + 0x78);
    Quad_2A62D0 qd;
    long pk[4];
    float q[4][4];
    float d0[4];
    float mtx[3][4];
    V_2A62D0 M[4];
    V_2A62D0 T[4];
    float off;
    int k, i, i0, i1, i2, i3, col;

    d0[0] = 0.0f;
    d0[1] = 0.0f;
    d0[2] = *(float *)(D_L00_00166D80_2A62D0 + 0x158);
    func_001FA1F8_2A62D0(mtx, d0);

    D_L00_0016145C_2A62D0 = D_L00_0016145C_2A62D0 + 0.01f;
    if (1.0f < D_L00_0016145C_2A62D0) {
        D_L00_0016145C_2A62D0 = D_L00_0016145C_2A62D0 - 1.0f;
    }

    pk[2] = 1;
    pk[3] = 0x4000000044L;
    pk[0] = 0;
    pk[1] = func_001F4868_2A62D0(0x18);

    for (k = 0; k < 4; k++) {
        func_001F9EC0_2A62D0(q[0], D_L00_001D7660_2A62D0[D_L00_001D7760_2A62D0[k][0]], mtx);
        func_001F9EC0_2A62D0(q[1], D_L00_001D7660_2A62D0[D_L00_001D7760_2A62D0[k][1]], mtx);
        func_001F9EC0_2A62D0(q[2], D_L00_001D7660_2A62D0[D_L00_001D7760_2A62D0[k][2]], mtx);
        func_001F9EC0_2A62D0(q[3], D_L00_001D7660_2A62D0[D_L00_001D7760_2A62D0[k][3]], mtx);

        qd.v[0][0] = *(float *)(m + 0x10) + q[0][0] * *(float *)(p + 0x90);
        qd.v[0][1] = *(float *)(m + 0x14) + q[0][1] * *(float *)(p + 0x90);
        qd.v[0][2] = *(float *)(m + 0x18) + q[0][2];
        qd.v[1][0] = *(float *)(m + 0x10) + q[1][0] * *(float *)(p + 0x90);
        qd.v[1][1] = *(float *)(m + 0x14) + q[1][1] * *(float *)(p + 0x90);
        qd.v[1][2] = *(float *)(m + 0x18) + q[1][2];
        qd.v[2][0] = *(float *)(m + 0x10) + q[2][0] * *(float *)(p + 0x90);
        qd.v[2][1] = *(float *)(m + 0x14) + q[2][1] * *(float *)(p + 0x90);
        qd.v[2][2] = *(float *)(m + 0x18) + q[2][2];
        qd.v[3][0] = *(float *)(m + 0x10) + q[3][0] * *(float *)(p + 0x90);
        qd.v[3][1] = *(float *)(m + 0x14) + q[3][1] * *(float *)(p + 0x90);
        qd.v[3][2] = *(float *)(m + 0x18) + q[3][2];

        i0 = D_L00_001D7760_2A62D0[k][0];
        i1 = D_L00_001D7760_2A62D0[k][1];
        i2 = D_L00_001D7760_2A62D0[k][2];
        i3 = D_L00_001D7760_2A62D0[k][3];
        off = D_L00_0016145C_2A62D0;
        qd.col[0] = D_L00_001D7738_2A62D0[i0];
        qd.col[1] = D_L00_001D7738_2A62D0[i1];
        qd.col[2] = D_L00_001D7738_2A62D0[i2];
        qd.col[3] = D_L00_001D7738_2A62D0[i3];
        qd.uv[0][0] = D_L00_001D76F0_2A62D0[i0][0];
        qd.uv[0][1] = D_L00_001D76F4_2A62D0[i0][0] + off;
        qd.uv[1][0] = D_L00_001D76F0_2A62D0[i1][0];
        qd.uv[1][1] = D_L00_001D76F4_2A62D0[i1][0] + off;
        qd.uv[2][0] = D_L00_001D76F0_2A62D0[i2][0];
        qd.uv[2][1] = D_L00_001D76F4_2A62D0[i2][0] + off;
        qd.uv[3][0] = D_L00_001D76F0_2A62D0[i3][0];
        qd.uv[3][1] = D_L00_001D76F4_2A62D0[i3][0] + off;
        func_L00_001FD1D8_2A62D0(&qd, 0, 1);
    }

    pk[1] = func_001F4868_2A62D0(0x1B);

    D_L00_00161460_2A62D0 = D_L00_00161460_2A62D0 + 0.01f;
    if (1.0f < D_L00_00161460_2A62D0) {
        D_L00_00161460_2A62D0 = 0.0f;
    }

    {
        float g = D_L00_00161460_2A62D0;
        float n = -g * 1.2f;
        float pp = g * 1.2f;
        float z = g * 1.4f + 1.1f;
        float zz = z - 0.2f;

        T[0].q = 0;
        T[0].f[0] = n * *(float *)(p + 0x90);
        T[0].f[1] = n * *(float *)(p + 0x90);
        T[0].f[2] = z;
        T[1].q = 0;
        T[1].f[0] = pp * *(float *)(p + 0x90);
        T[1].f[1] = n * *(float *)(p + 0x90);
        T[1].f[2] = zz;
        T[2].q = 0;
        T[2].f[0] = n * *(float *)(p + 0x90);
        T[2].f[1] = pp * *(float *)(p + 0x90);
        T[2].f[2] = z;
        T[3].q = 0;
        T[3].f[0] = pp * *(float *)(p + 0x90);
        T[3].f[1] = pp * *(float *)(p + 0x90);
        T[3].f[2] = zz;
        M[0].q = T[0].q;
        M[1].q = T[1].q;
        M[2].q = T[2].q;
        M[3].q = T[3].q;
    }

    d0[0] = 0.0f;
    d0[1] = 0.0f;
    d0[2] = *(float *)(D_L00_00166D80_2A62D0 + 0x158);
    func_001FA1F8_2A62D0(mtx, d0);
    func_001F9EC0_2A62D0(q[0], M[0].f, mtx);
    func_001F9EC0_2A62D0(q[1], M[1].f, mtx);
    func_001F9EC0_2A62D0(q[2], M[2].f, mtx);
    func_001F9EC0_2A62D0(q[3], M[3].f, mtx);

    col = ((int)((1.0f - D_L00_00161460_2A62D0) * 128.0f) << 24) | 0x808080;
    qd.v[0][0] = *(float *)(m + 0x10) + q[0][0];
    qd.v[0][1] = *(float *)(m + 0x14) + q[0][1];
    qd.v[0][2] = *(float *)(m + 0x18) + q[0][2];
    qd.v[1][0] = *(float *)(m + 0x10) + q[1][0];
    qd.v[1][1] = *(float *)(m + 0x14) + q[1][1];
    qd.v[1][2] = *(float *)(m + 0x18) + q[1][2];
    qd.v[2][0] = *(float *)(m + 0x10) + q[2][0];
    qd.v[2][1] = *(float *)(m + 0x14) + q[2][1];
    qd.v[2][2] = *(float *)(m + 0x18) + q[2][2];
    qd.v[3][0] = *(float *)(m + 0x10) + q[3][0];
    qd.v[3][1] = *(float *)(m + 0x14) + q[3][1];
    qd.v[3][2] = *(float *)(m + 0x18) + q[3][2];
    qd.col[0] = col;
    qd.col[1] = col;
    qd.col[2] = col;
    qd.col[3] = col;
    qd.uv[0][0] = 0.0f;
    qd.uv[0][1] = 0.0f;
    qd.uv[1][0] = 1.0f;
    qd.uv[1][1] = 0.0f;
    qd.uv[2][0] = 0.0f;
    qd.uv[2][1] = 1.0f;
    qd.uv[3][0] = 1.0f;
    qd.uv[3][1] = 1.0f;
    func_L00_001FD1D8_2A62D0(&qd, 0, 1);

    /* the sparkle vector reuses the first matrix row's slot in retail; its w is that row's 0 */
    for (i = 0; i < 4; i++) {
        float ang = (float)i * 3.14159f * 0.5f - 3.14159f;
        T[0].f[0] = func_001F9F90_2A62D0(ang) * 1.1f;
        T[0].f[1] = func_001F9FA8_2A62D0(ang) * 1.1f;
        T[0].f[2] = 0.59f;
        func_001F9EC0_2A62D0(T[0].f, T[0].f, m + 0xC0);
        func_001F9BD8_2A62D0(T[0].f, T[0].f, m + 0x10);
        func_L00_00264690_2A62D0(T[0].f, *(int *)(m + 0x90) & 0xFFFF0000, 0.1333f, 0.0f);
    }
}
