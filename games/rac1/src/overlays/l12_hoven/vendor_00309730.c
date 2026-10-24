/* Level 12 code: func_L12_00309730 in a file of its own, built with no extra flag.
 * vendor_002EDAA0.c needs -fno-sched-interblock (config/file_cflags.txt), and
 * this function only matches without it (docs/BUILD_FIDELITY.md, rule 2). */
#include "common.h"
/* functional: medium. Draw callback: builds a view matrix from the moby (+0xC0), sets up two quad packets (only the first is drawn) and draws it 5 x 6 times as a strip of jittering sparks (corner z grows by 1.0 per outer pass, y gets +-0.2 random jitter, u gets +-0.5 jitter per pass). Packet field meanings guessed from the layout. */
typedef struct Pkt {
    float corner[4][4];
    unsigned int color[4];
    float uv[4][2];
    long zero;
    long tex;
    long state;
    long flags;
} Pkt;
typedef struct Vq { float x, y, z, w; } __attribute__((aligned(16))) Vq;

extern long func_001F4868(int);
extern void func_001FA460(void *, void *);
extern float func_002140F8(float, float);
extern void func_L00_001FD1D8(void *, void *, int);
extern float D_L12_001FB990[][2];
extern Vq D_L12_001FB9B0[];
extern Vq D_L12_001FB9F0[];
extern short D_L12_00162010;
extern short D_L12_00162014;
extern short D_L12_0016203C;
extern short D_L12_00162040;
extern short D_L12_00162044;
extern short D_L12_00162048;
extern short D_L12_0016204C;
extern short D_L12_00162050;
extern short D_L12_00162054;
extern short D_L12_00162058;

void draw_hoven_jittered_quad_layers(char *m) {
    Pkt P[2] __attribute__((aligned(16)));
    float M[16] __attribute__((aligned(16)));
    long tex;
    int c0, c1;
    int a, b, j, k;
    float f20;

    func_001FA460(M, m + 0xC0);
    qcopy(&M[12], m + 0x10);
    M[15] = 1.0f;
    tex = GetEffectTex(0xE);
    P[0].tex = P[1].tex = tex;
    c0 = *(int *)&D_L12_00162010;
    c1 = *(int *)&D_L12_00162014;
    P[0].state = P[1].state = 0xFF9000000260L;
    P[0].flags = (long)*(int *)&D_L12_0016203C | ((long)*(int *)&D_L12_00162040 << 2) | ((long)*(int *)&D_L12_00162044 << 4)
                 | ((long)*(int *)&D_L12_00162048 << 6) | (0x8000L << 24);
    P[1].flags = (long)*(int *)&D_L12_0016204C | ((long)*(int *)&D_L12_00162050 << 2) | ((long)*(int *)&D_L12_00162054 << 4)
                 | ((long)*(int *)&D_L12_00162058 << 6) | (0x8000L << 24);
    P[0].color[0] = P[0].color[1] = P[0].color[2] = P[0].color[3] = c0;
    P[0].zero = P[1].zero = 0;
    P[1].color[0] = P[1].color[1] = P[1].color[2] = P[1].color[3] = c1;
    for (j = 0; j < 4; j++) {
        float u, v;
        qcopy(P[1].corner[j], &D_L12_001FB9B0[j]);
        qcopy(P[0].corner[j], &D_L12_001FB9F0[j]);
        u = D_L12_001FB990[j][0];
        v = D_L12_001FB990[j][1];
        P[0].uv[j][0] = u;
        P[0].uv[j][1] = v;
        P[1].uv[j][0] = u;
        P[1].uv[j][1] = v;
    }
    for (a = 0; a < 5; a++) {
        float f3 = random_float_between(-0.5f, 0.5f);
        for (k = 0; k < 4; k++) {
            float (*r0)[4] = P[0].corner;
            float (*r1)[4] = P[1].corner;
            r0[k][2] = r0[k][2] + 1.0f;
            r1[k][2] = r1[k][2] + 1.0f;
            P[0].uv[k][0] = D_L12_001FB990[k][0] + f3;
            P[0].uv[k][1] = D_L12_001FB990[k][1];
        }
        f20 = 0.0f;
        b = -3;
        do {
            int nb = b + 1;
            int more = b < 4;
            float f0 = (float)b + f20;
            f20 = 0.0f;
            P[0].corner[0][1] = f0;
            P[0].corner[1][1] = f0;
            if (more) f20 = random_float_between(-0.2f, 0.2f);
            f0 = (float)nb + f20;
            P[0].corner[2][1] = f0;
            P[0].corner[3][1] = f0;
            b = nb;
            func_L00_001FD1D8(&P[0], M, 0);
        } while (b < 3);
    }
}
