/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002DACC0.c (games/rac1) through the version
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
typedef struct { float x, y, z, w; } Vq2_c2d9810 __attribute__((aligned(16)));
extern int func_001F4868_2DACC0(int) __asm__("func_001F44B8");
extern void func_00234C98_2DACC0(int, long) __asm__("func_00233980");
extern void func_001F9BD8_2DACC0(void *, void *, void *) __asm__("func_001F9A10");
extern void func_001F9BF0_2DACC0(void *, void *, void *) __asm__("func_001F9A28");
extern void func_L00_001FF4B0_2DACC0(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_001F9CA0_2DACC0(void *, void *, void *) __asm__("func_001F9AD8");
extern void func_001F7868_2DACC0(void) __asm__("func_001F76A0");
extern void func_001F9EE8_2DACC0(void *, void *, void *) __asm__("func_001F9D20");
extern void func_L00_001FDE48_2DACC0(int, int, int, void *, int) __asm__("func_L00_001FDE98");
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_001F9C30_2DACC0(void *, void *, float) __asm__("func_001F9A68");
extern void func_L00_001FD1D8_2DACC0(void *, void *, int) __asm__("func_L00_001FD228");
extern int func_L00_002D95D8_2DACC0(unsigned char *m) __asm__("func_L00_002D8128");
extern void func_L00_001FF500_2DACC0(float *, float *, float) __asm__("func_L00_001FF550");
extern float func_L00_00200210_2DACC0(float, float) __asm__("func_L00_00200260");
extern char D_L00_001E40F0_2DACC0[] __asm__("D_L00_001E3DA0");
extern char D_L00_00166EC0_2DACC0[] __asm__("D_L00_00166DC0") NOT_SDA;
extern char D_L00_00161BC0_2DACC0[] __asm__("D_L00_00161AF0") NOT_SDA;
extern char D_L00_001E1810_2DACC0[] __asm__("D_L00_001E14C0");
extern char D_L00_001E3340_2DACC0[] __asm__("D_L00_001E2FF0");
extern char D_L00_001E2A30_2DACC0[] __asm__("D_L00_001E26E0");
extern int D_L00_00161BE0_2DACC0 __asm__("D_L00_00161B10") MACRO_ADDR;
typedef struct { short v[4]; } Row_2DACC0_c2d9810;
extern Row_2DACC0_c2d9810 D_L00_001E3DD0_r[] __asm__("D_L00_001E3A80");
extern char D_L00_001E37D0_2DACC0[] __asm__("D_L00_001E3480");
extern char D_L00_001E3ED0_2DACC0[] __asm__("D_L00_001E3B80");
extern Row_2DACC0_c2d9810 D_L00_001E3FD0_r[] __asm__("D_L00_001E3C80");
extern short D_L00_00161AE0_2DACC0 __asm__("D_L00_00161A10");
extern short D_L00_00161BA4_2DACC0 __asm__("D_L00_00161AD4");
extern short D_L00_00161B94_2DACC0 __asm__("D_L00_00161AC4");
extern short D_L00_00161B90_2DACC0 __asm__("D_L00_00161AC0");
extern short D_L00_00161B98_2DACC0 __asm__("D_L00_00161AC8");
extern short D_L00_00161B9C_2DACC0 __asm__("D_L00_00161ACC");
extern short D_L00_00161BA0_2DACC0 __asm__("D_L00_00161AD0");
extern short D_L00_00161BA8_2DACC0 __asm__("D_L00_00161AD8");
extern short D_L00_00161BB0_2DACC0 __asm__("D_L00_00161AE0");
extern short D_L00_00161BAC_2DACC0 __asm__("D_L00_00161ADC");
extern short D_L00_00161BBC_2DACC0 __asm__("D_L00_00161AEC");
extern short D_L00_00161BB4_2DACC0 __asm__("D_L00_00161AE4");
extern short D_L00_00161BB8_2DACC0 __asm__("D_L00_00161AE8");

/* FUN_L00_002d9810 -- src/overlays/shared/vendor_002D9438.c (functional C for the port, not a match)
 * Draw callback of a glowing orb: builds a camera-facing basis at the moby (lifted by its size),
 * transforms the 0x122-point mesh (D_L00_001E14C0) and draws its three strips (func_L00_001FDE98),
 * then 32 glow quads from the D_L00_001E3A80 rows (alpha by size), and when func_L00_002D8128 says
 * so, four reflection quads with uvs from the camera direction.
 * From the staged near miss (nonmatching/shared/FUN_L00_002d9810.c); fixed: the glow quads' first
 * two corners use the plain scale D_L00_00161ADC and the last two the scale times D_L00_00161AEC
 * (the near miss had them the other way round, which the equivalence tool cannot see).
 * equiv: DIFFERENT by shape only: retail keeps the row tables' full addresses in registers and
 * indexes them (sll 3, addu base); this adds the index to the high half and the low half after. */
void FUN_L00_002d9810(char *m) {
    struct { Vq2_c2d9810 pts[4]; int col[4]; float uv[8]; long w[4]; } q;
    Vq2_c2d9810 v90;
    Vq2_c2d9810 va0;
    Vq2_c2d9810 vb0;
    Vq2_c2d9810 vc0;
    Vq2_c2d9810 vd0;
    Vq2_c2d9810 ve0;
    Vq2_c2d9810 vf0;
    char *data = *(char **)(m + 0x78);
    float f22 = *(float *)data / *(float *)&D_L00_00161AE0_2DACC0;
    int i, j, k, n2;
    float *out;
    char *in;

    q.w[1] = func_001F4868_2DACC0(*(int *)&D_L00_00161BA4_2DACC0);
    q.w[0] = 0;
    q.w[2] = 0xFF9000000260L;
    q.w[3] = (long)*(int *)&D_L00_00161B90_2DACC0 | ((long)*(int *)&D_L00_00161B94_2DACC0 << 2)
           | ((long)*(int *)&D_L00_00161B98_2DACC0 << 4) | ((long)*(int *)&D_L00_00161B9C_2DACC0 << 6)
           | ((long)*(int *)&D_L00_00161BA0_2DACC0 << 32);
    func_00234C98_2DACC0(0x47, 0x53001);
    vc0.x = *(float *)(m + 0x10);
    vc0.y = *(float *)(m + 0x14);
    vc0.z = *(float *)(m + 0x18) + 0.5f;
    vc0.w = 1.0f;
    vc0.z += *(float *)data;
    func_001F9BD8_2DACC0(&vc0, &vc0, D_L00_00161BC0_2DACC0);
    func_001F9BF0_2DACC0(&ve0, D_L00_00166EC0_2DACC0, &vc0);
    func_L00_001FF4B0_2DACC0(&v90, &ve0, 1.0f);
    func_001F9CA0_2DACC0(&va0, &v90, *(char **)(data + 0xC) + 0xE0);
    func_L00_001FF4B0_2DACC0(&va0, &va0, 1.0f);
    func_001F9CA0_2DACC0(&vb0, &va0, &v90);
    q.col[0] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[3] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[2] = *(int *)&D_L00_00161BA8_2DACC0;
    q.col[1] = *(int *)&D_L00_00161BA8_2DACC0;
    func_00234C98_2DACC0(6, func_001F4868_2DACC0(*(int *)&D_L00_00161BA4_2DACC0));
    func_00234C98_2DACC0(0x42, 0x8000000044L);
    func_00234C98_2DACC0(8, 0);
    func_00234C98_2DACC0(0x14, 0xFF9000000260L);
    func_00234C98_2DACC0(0x4A, 0);
    func_001F7868_2DACC0();
    out = (float *)D_L00_001E40F0_2DACC0;
    in = D_L00_001E1810_2DACC0;
    for (i = 0; i < 0x122; i++) {
        func_001F9EE8_2DACC0(&vf0, in, &v90);
        in += 0x10;
        out[0] = vf0.x;
        out[1] = vf0.y;
        out[2] = vf0.z;
        out += 3;
    }
    func_L00_001FDE48_2DACC0(0x94, (int)D_L00_001E40F0_2DACC0, (int)D_L00_001E3340_2DACC0, D_L00_001E2A30_2DACC0, 1);
    func_L00_001FDE48_2DACC0(0x6D, (int)(D_L00_001E40F0_2DACC0 + 0x6D8), (int)(D_L00_001E3340_2DACC0 + 0x248), D_L00_001E2A30_2DACC0 + 0x490, 1);
    func_L00_001FDE48_2DACC0(0x24, (int)(D_L00_001E40F0_2DACC0 + 0xBE8), (int)(D_L00_001E3340_2DACC0 + 0x3F8), D_L00_001E2A30_2DACC0 + 0x7F0, 1);
    q.w[1] = func_001F4868_2DACC0(0xB);
    q.w[3] = 0x8000000048L;
    {
        int n = func_001FA898_r(f22 * 64.0f + 128.0f);
        int c = ((n + D_L00_00161BE0_2DACC0) << 24) | *(int *)&D_L00_00161BB0_2DACC0;
        q.col[3] = 0;
        q.col[2] = 0;
        q.col[0] = c;
        q.col[1] = c;
    }
    i = 0;
    do {
        short *row;
        n2 = i + 1;
        row = D_L00_001E3DD0_r[i].v;
        for (j = 0; j < 4; j++) {
            if (!(j < 2)) {
                func_001F9C30_2DACC0(&q.pts[j], D_L00_001E37D0_2DACC0 + row[j] * 16, *(float *)&D_L00_00161BAC_2DACC0 * *(float *)&D_L00_00161BBC_2DACC0);
            } else {
                func_001F9C30_2DACC0(&q.pts[j], D_L00_001E37D0_2DACC0 + row[j] * 16, *(float *)&D_L00_00161BAC_2DACC0);
            }
        }
        q.uv[0] = 0.5f;
        q.uv[1] = 0.5f;
        q.uv[2] = 0.5f;
        q.uv[3] = 0.5f;
        q.uv[4] = 0.5f;
        q.uv[5] = 1.0f;
        q.uv[6] = 0.5f;
        q.uv[7] = 1.0f;
        func_L00_001FD1D8_2DACC0(&q, &v90, 0);
        i = n2;
    } while (i < 0x20);
    if (func_L00_002D95D8_2DACC0((unsigned char *)m)) {
        q.w[1] = func_001F4868_2DACC0(*(int *)&D_L00_00161BB4_2DACC0);
        q.col[0] = 0x107F7F7F;
        q.col[3] = 0x107F7F7F;
        q.col[2] = 0x107F7F7F;
        q.col[1] = 0x107F7F7F;
        q.w[3] = 0x8000000044L;
        {
            char *p = *(char **)(data + 0xC);
            qcopy(&v90, p + 0xC0);
            qcopy(&va0, p + 0xD0);
            qcopy(&vb0, p + 0xE0);
        }
        vc0.z = *(float *)(m + 0x18);
        i = 0;
        do {
            short *row;
            float *u0 = &q.uv[0];
            float *u1 = &q.uv[1];
            n2 = i + 1;
            row = D_L00_001E3FD0_r[i].v;
            for (k = 0; k < 4; k++) {
                func_001F9EE8_2DACC0(&q.pts[k], D_L00_001E3ED0_2DACC0 + row[k] * 16, &v90);
                func_001F9BF0_2DACC0(&vd0, &q.pts[k], D_L00_00166EC0_2DACC0);
                vd0.y = vd0.z;
                func_L00_001FF500_2DACC0((float *)&vd0, (float *)&vd0, *(float *)&D_L00_00161BB8_2DACC0);
                *u0 = vd0.x * 0.5f + 0.5f;
                *u0 = func_L00_00200210_2DACC0(*u0, 8.0f);
                *u1 = vd0.y * 0.5f + 0.5f;
                *u1 = func_L00_00200210_2DACC0(*u1, 8.0f);
                u0 += 2;
                u1 += 2;
            }
            func_L00_001FD1D8_2DACC0(&q, 0, 0);
            i = n2;
        } while (i < 4);
    }
    func_00234C98_2DACC0(0x47, 0x5360B);
}
