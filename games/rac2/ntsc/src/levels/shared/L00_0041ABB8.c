#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern int func_2DB350(TI128 *dst, float f);

extern void func_L00_0041AEF8_1(void *dst);

extern long long func_0027C878(int id);

extern float func_00283248(float x, float y);

extern float func_00282E20(void *vec);

extern void func_00283410(void *dst, void *src);

extern void func_00282D30(void *dst, void *src, float scale);

extern void func_00280B90(void *dst, void *src, int flag);

#ifndef RAC2_T_BLOCK_F043C77A5
#define RAC2_T_BLOCK_F043C77A5
typedef struct { int q[4]; } BLOCK_F043c77a5;
#endif

#ifndef RAC2_T_LOC_F043C77A5
#define RAC2_T_LOC_F043C77A5
typedef struct {
    BLOCK_F043c77a5 b0;
    BLOCK_F043c77a5 b1;
    BLOCK_F043c77a5 b2;
    BLOCK_F043c77a5 b3;
    int   k0, k1, k2, k3;
    int   z80, z84, z88;
    float f92, f96;
    int   z100;
    float f104, f108;
    long long v112;
    long long v120;
    long long v128;
    long long v136;
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float v192, v196, v200, v204;
    float v208, f212, f216, v220;
} LOC_F043c77a5;
#endif

extern int func_002ABB78(char *object);

extern float *func_002AB1F0(char *object);

extern float func_002702C8_1(int *dst, float a, float b, float c, float d, float e);

INCLUDE_ASM("asm/levels", func_L00_0041ABB8);

INCLUDE_ASM("asm/levels", func_L00_0041AD40);

INCLUDE_ASM("asm/levels", func_L00_0041AE28);

INCLUDE_ASM("asm/levels", func_L00_0041AEF0);

/* shared native body of family 5a97ab89858c */
void func_L00_0041AEF8(char *p)
{
    *(long long *)(p + 112) = 0;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

/* shared native body of family 9a8a2404 */
void func_L00_0041AF60(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 8) = 0;
    *(float *)(p + 24) = 2.0f;
    *(int *)(p + 40) = 0;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_0041AFD0);

INCLUDE_ASM("asm/levels", func_L00_0041AFD8);

INCLUDE_ASM("asm/levels", func_L00_0041B1B8);

INCLUDE_ASM("asm/levels", func_L00_0041B840);

INCLUDE_ASM("asm/levels", func_L00_0041BAE8);

INCLUDE_ASM("asm/levels", func_L00_0041BCC8);

/* Builds a 224-byte local record, writes a looked-up handle and fixed fields, calls bound float helpers, invokes one bound helper on each of four blocks, and passes the record to a final bound helper. */
void func_L00_0041C160(void *unused, float *b, float *a, int k, float f)
{
    LOC_F043c77a5 local;
    float x;

    func_L00_0041AEF8_1(&local);
    local.z80 = 0;
    local.f92 = 1.0f;
    local.f96 = 1.0f;
    local.f104 = 1.0f;
    local.f108 = 1.0f;
    local.z84 = 0;
    local.z88 = 0;
    local.z100 = 0;
    local.v120 = func_0027C878(11);
    local.v128 = 0xFF9000000260LL;
    local.v136 = 0x0000008000000048LL;
    local.k3 = k;
    local.k2 = k;
    local.k1 = k;
    local.k0 = k;
    local.v112 = 0;
    local.v220 = 0.0f;
    local.f216 = func_00283248(a[0], a[1]);
    x = func_00283248(func_00282E20(a), a[2]);
    local.f212 = -x;
    local.v208 = 0.0f;
    func_00283410(&local.m00, &local.v208);
    local.v192 = b[0];
    local.v196 = b[1];
    local.v200 = b[2];
    local.v204 = 1.0f;
    func_00282D30(&local.b0, &local.b0, f);
    func_00282D30(&local.b1, &local.b1, f);
    func_00282D30(&local.b2, &local.b2, f);
    func_00282D30(&local.b3, &local.b3, f);
    func_00280B90(&local, &local.m00, 0);
}

INCLUDE_ASM("asm/levels", func_L00_0041C2D0);

INCLUDE_ASM("asm/levels", func_L00_0041CB80);

INCLUDE_ASM("asm/levels", func_L00_0041DEF0);

INCLUDE_ASM("asm/levels", func_L00_0041E168);

INCLUDE_ASM("asm/levels", func_L00_0041E4A8);

INCLUDE_ASM("asm/levels", func_L00_0041E648);

INCLUDE_ASM("asm/levels", func_L00_0041E958);

INCLUDE_ASM("asm/levels", func_L00_0041EB38);

INCLUDE_ASM("asm/levels", func_L00_0041EE58);

INCLUDE_ASM("asm/levels", func_L00_0041F570);

INCLUDE_ASM("asm/levels", func_L00_0041F820);

INCLUDE_ASM("asm/levels", func_L00_0041FC20);

INCLUDE_ASM("asm/levels", func_L00_004200F0);

/* calls two guards and returns 1 only when both succeed and the object's float field is not positive */
int func_L00_00420310(char *object)
{
    float *p;

    if (func_002ABB78(object) != 0)
        return 0;
    p = func_002AB1F0(object);
    if (p == 0)
        return 0;
    if (0.0f < *p)
        return 0;
    return 1;
}

INCLUDE_ASM("asm/levels", func_L00_00420388);

int func_L00_004204A0(TI128 *src)
{
    TI128 local;

    local = *src;
    return func_2DB350(&local, 0.0f) != 0;
}

INCLUDE_ASM("asm/levels", func_L00_00420638);

INCLUDE_ASM("asm/levels", func_L00_00420780);

INCLUDE_ASM("asm/levels", func_L00_004208C8);

INCLUDE_ASM("asm/levels", func_L00_00420C38);

INCLUDE_ASM("asm/levels", func_L00_004212B8);

/* reads a base pointer from a0+112 and pushes three consecutive tweak slots through the same five-float interpolation helper, writing each result back over the first float vector while forwarding three trailing floats unchanged. */
void func_L00_004219D8(char *a0, float *a1, float *a2,
                                        float f12, float f13, float f14)
{
    int *p = *(int **)(a0 + 112);

    a1[0] = func_002702C8_1(p + 64, a1[0], a2[0], f12, f13, f14);
    a1[1] = func_002702C8_1(p + 65, a1[1], a2[1], f12, f13, f14);
    a1[2] = func_002702C8_1(p + 66, a1[2], a2[2], f12, f13, f14);
}

INCLUDE_ASM("asm/levels", func_L00_00421A98);

INCLUDE_ASM("asm/levels", func_L00_00421B50);

INCLUDE_ASM("asm/levels", func_L00_00421C50);

INCLUDE_ASM("asm/levels", func_L00_00422268);
