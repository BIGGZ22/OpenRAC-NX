#include "include_asm.h"

#ifndef RAC2_T_VEC_F6BF9CE42
#define RAC2_T_VEC_F6BF9CE42
typedef struct { float x; float y; } VEC_F6bf9ce42;
#endif

extern int D_L04_001AE478 __attribute__((sda));

extern int D_L04_001AE47C;

extern int D_L04_001AE480;

extern int D_L04_001AE484;

extern int D_L04_001AE488 __attribute__((sda));

extern int D_L04_001AE48C;

extern int D_001A7340;

extern int D_001A7344;

extern int func_002A9588(int a, int b, int c, int d, int e);

extern int func_00285548(int a, int b, int c, int d, int e, int f, int g);

void func_00343068(VEC_F6bf9ce42 **pobj)
{
    int s0, s1, s2, s3;
    int r;


    VEC_F6bf9ce42 *v;
    v = *pobj;
    s0 = (int)((float)D_L04_001AE478 + v->x - (float)D_L04_001AE488);
    s1 = (int)((float)D_L04_001AE47C + v->y - (float)D_L04_001AE48C);
    s3 = (int)((float)D_L04_001AE480 + v->x + (float)D_L04_001AE488);
    s2 = (int)((float)D_L04_001AE484 + v->y + (float)D_L04_001AE48C);

    r = func_002A9588(0x60241700, 0x55F0C070, 20, 0, 0);
    func_00285548(s0, s1, s3, s2, D_001A7340, D_001A7344, r);

    v = *pobj;
    func_00285548((int)((float)D_L04_001AE478 + v->x), (int)((float)D_L04_001AE47C + v->y),
            (int)((float)D_L04_001AE480 + v->x), (int)((float)D_L04_001AE484 + v->y),
            D_001A7340, D_001A7344, 0x60241700);

}
