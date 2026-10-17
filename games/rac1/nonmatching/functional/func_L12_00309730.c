/* functional: medium. Corrected in place: the translation row M[12..14] = moby +0x10 (lq) with M[15] = 1.0 was missing after the matrix build. Draw callback: builds a view matrix from the moby (+0xC0), sets up two quad packets (only the first is drawn) and draws it 5 x 6 times as a strip of jittering sparks (corner z grows by 1.0 per outer pass, y gets +-0.2 random jitter, u gets +-0.5 jitter per pass). Packet field meanings guessed from the layout. */
typedef struct Pkt_09730 {
    float corner[4][4];
    unsigned int color[4];
    float uv[4][2];
    long zero;
    long tex;
    long state;
    long flags;
} Pkt_09730;
typedef struct Vq_09730 { float x, y, z, w; } __attribute__((aligned(16))) Vq_09730;

extern void mat_09730(void *, void *) __asm__("func_001FA460");
extern long tex_09730(int) __asm__("func_001F4868");
extern float rnd_09730(float, float) __asm__("func_002140F8");
extern void draw_09730(void *, void *, int) __asm__("func_L00_001FD1D8");
extern float D_L12_001FB990_09730[][2] __asm__("D_L12_001FB990");
extern Vq_09730 D_L12_001FB9B0_09730[] __asm__("D_L12_001FB9B0");
extern Vq_09730 D_L12_001FB9F0_09730[] __asm__("D_L12_001FB9F0");
extern int D_L12_00162010_09730 __asm__("D_L12_00162010");
extern int D_L12_00162014_09730 __asm__("D_L12_00162014");
extern int D_L12_0016203C_09730 __asm__("D_L12_0016203C");
extern int D_L12_00162040_09730 __asm__("D_L12_00162040");
extern int D_L12_00162044_09730 __asm__("D_L12_00162044");
extern int D_L12_00162048_09730 __asm__("D_L12_00162048");
extern int D_L12_0016204C_09730 __asm__("D_L12_0016204C");
extern int D_L12_00162050_09730 __asm__("D_L12_00162050");
extern int D_L12_00162054_09730 __asm__("D_L12_00162054");
extern int D_L12_00162058_09730 __asm__("D_L12_00162058");

void func_L12_00309730_impl(char *m) __asm__("func_L12_00309730");
void func_L12_00309730_impl(char *m) {
    Pkt_09730 P[2] __attribute__((aligned(16)));
    float M[16] __attribute__((aligned(16)));
    long tex;
    int a, b, j, k;
    float f20;

    mat_09730(M, m + 0xC0);
    {
        float *pos = (float *)(m + 0x10);
        M[12] = pos[0];
        M[13] = pos[1];
        M[14] = pos[2];
        M[15] = 1.0f;
    }
    tex = tex_09730(0xE);
    P[0].flags = (long)D_L12_0016203C_09730 | ((long)D_L12_00162040_09730 << 2) | ((long)D_L12_00162044_09730 << 4)
                 | ((long)D_L12_00162048_09730 << 6) | (0x8000L << 24);
    P[1].flags = (long)D_L12_0016204C_09730 | ((long)D_L12_00162050_09730 << 2) | ((long)D_L12_00162054_09730 << 4)
                 | ((long)D_L12_00162058_09730 << 6) | (0x8000L << 24);
    P[0].state = 0xFF9000000260L;
    P[1].state = 0xFF9000000260L;
    P[0].color[0] = D_L12_00162010_09730;
    P[0].color[3] = D_L12_00162010_09730;
    P[0].color[2] = D_L12_00162010_09730;
    P[0].color[1] = D_L12_00162010_09730;
    P[0].tex = tex;
    P[1].color[0] = D_L12_00162014_09730;
    P[1].tex = tex;
    P[1].zero = 0;
    P[0].zero = 0;
    P[1].color[3] = D_L12_00162014_09730;
    P[1].color[2] = D_L12_00162014_09730;
    P[1].color[1] = D_L12_00162014_09730;
    for (j = 0; j < 4; j++) {
        float u = D_L12_001FB990_09730[j][0];
        float v = D_L12_001FB990_09730[j][1];
        *(Vq_09730 *)P[1].corner[j] = D_L12_001FB9B0_09730[j];
        *(Vq_09730 *)P[0].corner[j] = D_L12_001FB9F0_09730[j];
        P[0].uv[j][0] = u;
        P[0].uv[j][1] = v;
        P[1].uv[j][0] = u;
        P[1].uv[j][1] = v;
    }
    for (a = 0; a < 5; a++) {
        float f3 = rnd_09730(-0.5f, 0.5f);
        for (k = 0; k < 4; k++) {
            float u = D_L12_001FB990_09730[k][0];
            float v = D_L12_001FB990_09730[k][1];
            P[0].corner[k][2] = P[0].corner[k][2] + 1.0f;
            P[1].corner[k][2] = P[1].corner[k][2] + 1.0f;
            P[0].uv[k][0] = u + f3;
            P[0].uv[k][1] = v;
        }
        f20 = 0.0f;
        b = -3;
        do {
            float f0 = (float)b + f20;
            int cur = b;
            b = b + 1;
            f20 = 0.0f;
            P[0].corner[1][1] = f0;
            P[0].corner[0][1] = f0;
            if (cur < 4) f20 = rnd_09730(-0.2f, 0.2f);
            f0 = (float)b + f20;
            P[0].corner[3][1] = f0;
            P[0].corner[2][1] = f0;
            draw_09730(&P[0], M, 0);
        } while (b < 3);
    }
}
