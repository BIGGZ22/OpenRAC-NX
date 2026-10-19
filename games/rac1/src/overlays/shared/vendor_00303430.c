/* Level 14 code: func_L14_00303430 in a file of its own, built with -fno-sched-spec
 * (config/file_cflags.txt). The functions of vendor_002B2A28.c do not build
 * with it (func_L14_00300130 changes), so it was another object in retail
 * (docs/BUILD_FIDELITY.md, rule 2). */
#include "common.h"

typedef struct Pkt_03430 {
    float corner[4][4];
    unsigned int color[4];
    float uv[4][2];
    long zero;
    long tex;
    long state;
    long flags;
} Pkt_03430;

extern void func_001FA218(void *, void *);
extern void func_001FA4A0(void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9BC0(void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_001FA888(int);
u64 func_001F4868(s32);
extern void func_L00_001FD1D8(void *, void *, int);
typedef struct { float v[8][4]; } Frame_03430;
extern char *D_L14_001601AC MACRO_ADDR;
extern char D_L14_001675C0[];
extern float D_L14_001E6320[][2];
extern short D_L14_001621AC;
extern short D_L14_001621B0;
extern short D_L14_001621B4;
extern short D_L14_001621B8;
extern short D_L14_001621BC;

typedef struct {
    int idx;
    char pad4[0x10];
    int color;
    unsigned char cnt[4];
    char pad1c[4];
    float pts[64][4];
    float seg[4][4][4];
} D_03430;

void func_L14_00303430(char *m) {
    D_03430 *d = *(D_03430 **)(m + 0x78);
    Pkt_03430 P[3];
    float M1[16] __attribute__((aligned(16)));
    float M2[16] __attribute__((aligned(16)));
    float v230[4] __attribute__((aligned(16)));
    float v240[4] __attribute__((aligned(16)));
    float v250[4] __attribute__((aligned(16)));
    float v260[4] __attribute__((aligned(16)));
    float v270[4] __attribute__((aligned(16)));
    long tex;
    long flags;
    int j, n, k;
    int c;
    int t;

    func_001FA218(M1, ((Frame_03430 *)D_L14_001601AC)[d->idx].v[7]);
    func_001FA4A0(M2, M1);
    qcopy_nc(&M1[12], ((Frame_03430 *)D_L14_001601AC)[d->idx].v[3]);
    func_001F9BF0(v230, D_L14_001675C0, ((Frame_03430 *)D_L14_001601AC)[d->idx].v[3]);
    func_001F9EC0(v230, v230, M2);
    tex = func_001F4868(*(int *)&D_L14_001621AC);
    flags = (long)*(int *)&D_L14_001621B0 | ((long)*(int *)&D_L14_001621B4 << 2) | ((long)*(int *)&D_L14_001621B8 << 4)
            | ((long)*(int *)&D_L14_001621BC << 6) | (0x8000L << 24);
    P[2].state = 0xFF9000000260L;
    P[1].state = 0xFF9000000260L;
    P[0].state = 0xFF9000000260L;
    P[2].tex = tex;
    P[1].tex = tex;
    P[0].tex = tex;
    P[2].zero = 0;
    P[1].zero = 0;
    P[0].zero = 0;
    P[2].flags = flags;
    P[1].flags = flags;
    P[0].flags = flags;
    for (t = 0; t < 4; t++) {
        float v = D_L14_001E6320[t][1];
        int col;
        P[2].uv[t][1] = v;
        P[1].uv[t][1] = v;
        P[0].uv[t][1] = v;
        col = d->color;
        P[2].color[t] = col;
        P[1].color[t] = col;
        P[0].color[t] = col;
    }
    func_001F9BC0(v250);
    {
        int i;
        for (i = 0; i < 31; i++) {
            qcopy(v240, v250);
            if (i < 30) {
                func_001F9BF0(v270, d->pts[i], d->pts[i + 2]);
                func_001F9BF0(v260, d->pts[i + 1], v230);
                func_001F9CA0(v250, v260, v270);
                func_L00_001FF4B0(v250, v250, 0.1f);
            } else {
                func_001F9BC0(v250);
            }
            for (k = 0; k < 4; k++) {
                if (k < 2) {
                    qcopy(P[0].corner[k], d->pts[i]);
                    if (k & 1) func_001F9BD8(P[0].corner[k], P[0].corner[k], v240);
                    else func_001F9BF0(P[0].corner[k], P[0].corner[k], v240);
                } else {
                    qcopy(P[0].corner[k], d->pts[i + 1]);
                    if (k & 1) func_001F9BD8(P[0].corner[k], P[0].corner[k], v250);
                    else func_001F9BF0(P[0].corner[k], P[0].corner[k], v250);
                }
                P[0].uv[k][0] = D_L14_001E6320[k][0] + func_001FA888(i - i / 4 * 4) * 0.25f;
            }
            func_L00_001FD1D8(&P[0], M1, 0);
        }
    }
    for (j = 0; j < 4; j++) {
        func_001F9BC0(v250);
        c = d->cnt[j];
        if (c >= 0x1F) continue;
        if (c <= -1) continue;
        for (n = 0; n < 3; n++) {
            qcopy(v240, v250);
            if (n < 2) {
                func_001F9BF0(v270, d->seg[n][j], d->seg[n + 2][j]);
                func_001F9BF0(v260, d->seg[n + 1][j], v230);
                func_001F9CA0(v250, v260, v270);
                func_L00_001FF4B0(v250, v250, 0.1f);
            } else {
                func_001F9BC0(v250);
            }
            if (n == 0) qcopy(v240, v250);
            for (k = 0; k < 4; k++) {
                if (k < 2) {
                    qcopy(P[0].corner[k], d->seg[n][j]);
                    if (k & 1) func_001F9BD8(P[0].corner[k], P[0].corner[k], v240);
                    else func_001F9BF0(P[0].corner[k], P[0].corner[k], v240);
                } else {
                    qcopy(P[0].corner[k], d->seg[n + 1][j]);
                    if (k & 1) func_001F9BD8(P[0].corner[k], P[0].corner[k], v250);
                    else func_001F9BF0(P[0].corner[k], P[0].corner[k], v250);
                }
                P[0].uv[k][0] = D_L14_001E6320[k][0] + func_001FA888(n - n / 4 * 4) * 0.25f;
            }
            func_L00_001FD1D8(&P[0], M1, 0);
        }
    }
}
