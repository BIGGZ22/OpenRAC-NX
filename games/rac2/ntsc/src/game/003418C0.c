#include "include_asm.h"

#ifndef RAC2_T_F32_FD368ADD8
#define RAC2_T_F32_FD368ADD8
typedef float f32_Fd368add8;
#endif

extern f32_Fd368add8 D_L01_001AE3F8 __attribute__((sda));

extern f32_Fd368add8 D_L01_001AE408 __attribute__((sda));

extern f32_Fd368add8 D_L01_001AE400 __attribute__((sda));

extern f32_Fd368add8 D_L01_001AE410 __attribute__((sda));

extern f32_Fd368add8 D_L01_001AE418 __attribute__((sda));

extern f32_Fd368add8 D_L01_001AE3FC;

extern f32_Fd368add8 D_L01_001AE40C;

extern f32_Fd368add8 D_L01_001AE404;

extern f32_Fd368add8 D_L01_001AE414;

extern f32_Fd368add8 D_L01_001AE41C;

extern void func_00335E38_1(void *a0, f32_Fd368add8 a1, f32_Fd368add8 a2, f32_Fd368add8 a3, f32_Fd368add8 a4);

extern void *func_00335E20_1(void *a0);

extern int func_002A9588(int a0, int a1, int a2, int a3, int a4);

extern void func_003417D0(void *a0, int a1);

extern void func_00341760(void *a0);

/* This call target is not the unrelated function at Aranos 0x004370F8. */
extern void ref_L00_003418C0_00(void *a0);

int func_003418C0(char *self, int arg1)
{
    f32_Fd368add8 *v;

    v = *(f32_Fd368add8 **)(self + 536);
    func_00335E38_1(self, D_L01_001AE3F8 + v[0], D_L01_001AE3FC + v[1], 0.0f, 0.0f);
    v = *(f32_Fd368add8 **)(self + 536);
    func_00335E38_1(self + 152, D_L01_001AE408 + v[0], D_L01_001AE40C + v[1], 0.0f, 0.0f);
    v = *(f32_Fd368add8 **)(self + 536);
    func_00335E38_1(self + 76, D_L01_001AE400 + v[0], D_L01_001AE404 + v[1], 0.0f, 0.0f);
    v = *(f32_Fd368add8 **)(self + 536);
    func_00335E38_1(self + 228, D_L01_001AE410 + v[0], D_L01_001AE414 + v[1], 0.0f, 0.0f);
    v = *(f32_Fd368add8 **)(self + 536);
    func_00335E38_1(self + 464, D_L01_001AE418 + v[0], D_L01_001AE41C + v[1], 0.0f, 0.0f);

    *(int *)func_00335E20_1(self + 304) = func_002A9588(0x60442D00, 0x70FFFEED, 20, 0, 0);
    func_003417D0(self, arg1);
    func_00341760(self);
    ref_L00_003418C0_00(self);
    return *(int *)(self + (*(int *)(self + 764) << 2) + 440);
}

extern void func_00336320(void *);

void func_00341A48(char *object) {
    func_00336320(object + 0x130);
}
