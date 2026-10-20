#include "include_asm.h"

#ifndef RAC2_T_F32_F7036EF03
#define RAC2_T_F32_F7036EF03
typedef float f32_F7036ef03;
#endif

extern f32_F7036ef03 D_L04_001AE4A8 __attribute__((sda));

extern f32_F7036ef03 D_L04_001AE4B0 __attribute__((sda));

extern f32_F7036ef03 D_L04_001AE4B8 __attribute__((sda));

extern f32_F7036ef03 D_L04_001AE4C0 __attribute__((sda));

extern f32_F7036ef03 D_L04_001AE4C8 __attribute__((sda));

extern f32_F7036ef03 D_L04_001AE4AC;

extern f32_F7036ef03 D_L04_001AE4B4;

extern f32_F7036ef03 D_L04_001AE4BC;

extern f32_F7036ef03 D_L04_001AE4C4;

extern f32_F7036ef03 D_L04_001AE4CC;

extern void func_00335E38_2(void *a0, f32_F7036ef03 a1, f32_F7036ef03 a2, f32_F7036ef03 a3, f32_F7036ef03 a4);

extern void func_L00_00438970(void *a0, f32_F7036ef03 a1, f32_F7036ef03 a2);

extern void func_00343638(void *a0);

extern void func_003435C8(void *a0);

extern int func_00342978(void *a0, int a1);

void func_00343798(char *self, int arg)
{
    f32_F7036ef03 *v;
    char *sub = self + 664;

    v = *(f32_F7036ef03 **)(self + 648);
    func_00335E38_2(self, D_L04_001AE4A8 + v[0], D_L04_001AE4AC + v[1], 0.0f, 0.0f);
    v = *(f32_F7036ef03 **)(self + 648);
    func_00335E38_2(self + 76, D_L04_001AE4B0 + v[0], D_L04_001AE4B4 + v[1], 0.0f, 0.0f);
    v = *(f32_F7036ef03 **)(self + 648);
    func_00335E38_2(self + 152, D_L04_001AE4B8 + v[0], D_L04_001AE4BC + v[1], 0.0f, 0.0f);
    v = *(f32_F7036ef03 **)(self + 648);
    func_00335E38_2(self + 576, D_L04_001AE4C0 + v[0], D_L04_001AE4C4 + v[1], 0.0f, 0.0f);
    v = *(f32_F7036ef03 **)(self + 648);
    func_L00_00438970(sub, D_L04_001AE4C8 + v[0], D_L04_001AE4CC + v[1]);
    func_00343638(self);
    func_003435C8(self);
    *(int *)(self + 416) = func_00342978(sub, arg);
}

INCLUDE_ASM("asm/game", func_003438E0);

extern void func_00336320(void *a0);
extern void func_00336A78(void *a0);
extern void func_00342BE8(void *a0);
extern int func_00342BC0(void *a0);
extern int func_00288FB8(int);
extern void func_00336A38(void *a0, int a1);
extern unsigned char D_00139568[];
extern char D_00239BA0[];

void func_003438E8(char *self)
{
    int r;
    char *rec;

    if (*(int *)(self + 0x290) != 0) {
        func_00336320(self);
        func_00336320(self + 0x4C);
        func_00336320(self + 0x98);
        func_00336A78(self + 0x1B0);
        func_00342BE8(self + 0x298);
        r = func_00342BC0(self + 0x298);
        rec = D_00239BA0 + D_00139568[r] * 224;
        func_00336A38(self + 0x1F8, func_00288FB8(*(int *)(rec + 8)));
        func_00336A78(self + 0x1F8);
        func_00336A78(self + 0x240);
    }
}
