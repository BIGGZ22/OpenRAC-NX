#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern int func_2DB350(TI128 *dst, float f);

extern void func_37B508(int a, int b);

extern void func_41B1B8(char *a0, int a1, int a2, float f12);

extern void func_41BCC8(char *a0);

extern void func_L00_0041AFD8(char *a0, char *a1, int a2, float f12);

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

void func_L00_0041AD40(void) { }
