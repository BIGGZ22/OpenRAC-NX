/* Carried from the PAL decompilation's nonmatching/functional/func_L14_00302C80.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* functional: medium. Builds the arc strand for the electric-arc moby (data block d): 32 jittered points (P) and offsets (Q) along the table entry's span, smoothed with a 3-point average into d+0x20/d+0x220, then 4 branches (byte count at d+0x18+i, 3 segments each at d+0x420..) and, when the entry is at least 4.0 from the camera point, spawns 4+2 bursts of particles (func_00218888). Meanings of the gp-relative tuning values are unknown; the operations follow the assembly. */
typedef struct Vq_02C80_c3017f8 { float x, y, z, w; } __attribute__((aligned(16))) Vq_02C80_c3017f8;
extern float jit_02C80(int) __asm__("func_001FA6C0");
extern int frames_02C80(int) __asm__("func_001F9688");
extern int pick_02C80(int, int) __asm__("func_L00_00257B90");
extern float len_02C80(void *) __asm__("func_001F9AF0");
extern void zero_02C80(void *) __asm__("func_001F99F8");
extern void rvec_02C80(void *, float, float) __asm__("func_L00_00257D78");
extern void add_02C80(void *, void *, void *) __asm__("func_001F9A10");
extern void scale_02C80(void *, void *, float) __asm__("func_001F9A68");
extern float rnd_02C80(float, float) __asm__("func_002132A8");
extern int ftoi_02C80(float) __asm__("func_001FA6D0");
extern float dist_02C80(void *, void *) __asm__("func_001F9B80");
extern void mat_02C80(void *, void *) __asm__("func_001FA050");
extern void xform_02C80(void *, void *, void *) __asm__("func_001F9D20");
extern int rndi_02C80(int) __asm__("func_00213260");
extern void burst_02C80(void *, void *, void *, int, int, int, int, int, int) __asm__("func_00218888");
extern char *D_L14_001601AC_02C80 __asm__("D_L14_001600EC");
extern float D_L14_001675C0_02C80[] __asm__("D_L14_001674C0");
extern float D_0015EE6C_02C80 __asm__("D_0015ED6C");
extern int D_L14_0016218C_02C80 __asm__("D_L14_001620BC");
extern int D_L14_00162190_02C80 __asm__("D_L14_001620C0");
extern float D_L14_001621CC_02C80 __asm__("D_L14_001620FC");
extern float D_L14_001621D0_02C80 __asm__("D_L14_00162100");
extern int D_L14_001621D8_02C80 __asm__("D_L14_00162108");
extern int D_L14_001621DC_02C80 __asm__("D_L14_0016210C");
extern float D_L14_001621E4_02C80 __asm__("D_L14_00162114");
extern int D_L14_001621EC_02C80 __asm__("D_L14_0016211C");
extern int D_L14_001621F0_02C80 __asm__("D_L14_00162120");

void func_L14_00302C80_impl(char *m) __asm__("FUN_L14_003017f8");


void func_L14_00302C80_impl(char *m) {
    char *d = *(char **)(m + 0x78);
    Vq_02C80_c3017f8 P[32];
    Vq_02C80_c3017f8 Q[32];
    Vq_02C80_c3017f8 A, B;
    Vq_02C80_c3017f8 M[4];
    Vq_02C80_c3017f8 vc;
    char *ent;
    float f22;
    int a, b, r;
    int i, j, k;

    ent = D_L14_001601AC_02C80 + (*(int *)d << 7);
    *(float *)(ent + 0x3C) = 1.0f;
    a = frames_02C80(D_L14_0016218C_02C80);
    b = frames_02C80(D_L14_00162190_02C80);
    r = pick_02C80(a, b);
    *(int *)(d + 0x620) = (short)r;
    *(short *)(d + 0x12) = (short)r;
    f22 = len_02C80(D_L14_001601AC_02C80 + (*(int *)d << 7) + 0x10);
    f22 = f22 + f22;

    for (i = 0; i < 32; i++) {
        float f20 = jit_02C80(i);
        float t = jit_02C80(0x1F);
        float y;
        f20 = f20 / t;
        f20 = (0.5f - f20) * f22;
        if ((unsigned)(i - 3) < 0x1A) {
            rvec_02C80(&P[i], 0.0f, D_L14_001621CC_02C80);
            rvec_02C80(&Q[i], 0.0f, D_L14_001621D0_02C80 * D_0015EE6C_02C80);
        } else {
            zero_02C80(&P[i]);
            zero_02C80(&Q[i]);
        }
        y = P[i].y;
        P[i].w = 1.0f;
        P[i].y = y + f20;
    }

    for (j = 0; j < 32; j++) {
        if (j == 0 || j == 31) {
            *(Vq_02C80_c3017f8 *)(d + 0x20 + (j << 4)) = P[j];
            *(Vq_02C80_c3017f8 *)(d + 0x220 + (j << 4)) = Q[j];
        } else {
            add_02C80(&A, &P[j - 1], &P[j]);
            add_02C80(&A, &A, &P[j + 1]);
            scale_02C80(d + 0x20 + (j << 4), &A, 0.333f);
            add_02C80(&A, &Q[j - 1], &Q[j]);
            add_02C80(&A, &A, &Q[j + 1]);
            scale_02C80(d + 0x220 + (j << 4), &A, 0.333f);
        }
    }

    for (i = 0; i < 4; i++) {
        int n;
        float c = D_L14_001621CC_02C80;
        rvec_02C80(&A, c, c);
        n = ftoi_02C80(rnd_02C80((float)i * 6.0f, (float)(i + 1) * 6.0f)) + 4;
        *(unsigned char *)(d + 0x18 + i) = (unsigned char)n;
        *(Vq_02C80_c3017f8 *)(d + 0x420 + (i << 4)) = *(Vq_02C80_c3017f8 *)(d + 0x20 + (n << 4));
        *(Vq_02C80_c3017f8 *)(d + 0x520 + (i << 4)) = *(Vq_02C80_c3017f8 *)(d + 0x220 + (n << 4));
        for (k = 0; k < 3; k++) {
            rvec_02C80(&B, 0.1f, 0.1f);
            rvec_02C80(d + (i << 4) + 0x560 + (k << 6), 0.0f, D_L14_001621D0_02C80 * D_0015EE6C_02C80);
            add_02C80(d + (i << 4) + 0x460 + (k << 6), d + (i << 4) + 0x420 + (k << 6), &A);
            add_02C80(d + (i << 4) + 0x460 + (k << 6), d + (i << 4) + 0x460 + (k << 6), &B);
        }
    }

    if (dist_02C80(D_L14_001601AC_02C80 + (*(int *)d << 7) + 0x30, D_L14_001675C0_02C80) >= 4.0f) {
        int s17;
        mat_02C80(M, D_L14_001601AC_02C80 + (*(int *)d << 7) + 0x70);
        vc = *(Vq_02C80_c3017f8 *)(D_L14_001601AC_02C80 + (*(int *)d << 7) + 0x30);
        for (i = 0; i < 32; i += 8) {
            int n;
            char *p = d + 0x20 + (i << 4);
            xform_02C80(&A, p, M);
            B = *(Vq_02C80_c3017f8 *)(p + 0x200);
            B.w = D_L14_001621E4_02C80;
            s17 = rndi_02C80(2) ? D_L14_001621EC_02C80 : D_L14_001621F0_02C80;
            n = frames_02C80(rndi_02C80(10) + 20);
            burst_02C80(&A, &B, &B, D_L14_001621D8_02C80, D_L14_001621DC_02C80, i >> 1, 15, n, s17);
        }
        for (k = 0; k < 2; k++) {
            int n;
            xform_02C80(&A, d + 0x4E0 + (k << 4), M);
            B = *(Vq_02C80_c3017f8 *)(d + 0x5E0 + (k << 4));
            B.w = D_L14_001621E4_02C80;
            rndi_02C80(2);
            n = frames_02C80(rndi_02C80(10) + 20);
            burst_02C80(&A, &B, &B, D_L14_001621D8_02C80, D_L14_001621DC_02C80,
                        *(unsigned char *)(d + 0x18 + k) >> 1, 15, n, D_L14_001621F0_02C80);
        }
    }
}
