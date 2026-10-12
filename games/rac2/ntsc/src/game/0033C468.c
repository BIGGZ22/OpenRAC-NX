#include "include_asm.h"

#ifndef RAC2_T_U8_FF4EB83C9
#define RAC2_T_U8_FF4EB83C9
typedef unsigned char u8_Ff4eb83c9;
#endif

#ifndef RAC2_T_S32_FF4EB83C9
#define RAC2_T_S32_FF4EB83C9
typedef int s32_Ff4eb83c9;
#endif

#ifndef RAC2_T_F32_FF4EB83C9
#define RAC2_T_F32_FF4EB83C9
typedef float f32_Ff4eb83c9;
#endif

/* Resident shared table reached through $gp (small-data symbol). */
extern f32_Ff4eb83c9 D_L04_001AE070[2] __attribute__((sda));

extern void *func_00336E40_3(void *);

extern void *func_00336DA8_3(s32_Ff4eb83c9, void *);

extern void func_0033ABD8_3(u8_Ff4eb83c9 *, void *, f32_Ff4eb83c9 *);

extern void *func_00288FB8_2(s32_Ff4eb83c9);

extern void func_0033B050_2(u8_Ff4eb83c9 *, void *, void *, void *);

extern void func_0033B030_3(u8_Ff4eb83c9 *, f32_Ff4eb83c9, f32_Ff4eb83c9, f32_Ff4eb83c9, f32_Ff4eb83c9, f32_Ff4eb83c9, f32_Ff4eb83c9);

extern void func_0033B0A8_2(u8_Ff4eb83c9 *, f32_Ff4eb83c9);

extern void func_0033C590(u8_Ff4eb83c9 *, s32_Ff4eb83c9);

void func_0033C468(u8_Ff4eb83c9 *self, void *arg) {
    u8_Ff4eb83c9 *sub;
    void *first, *second, *third;
    *(void **)self = arg;
    if (arg != 0) {
        void *t = func_00336E40_3(arg);
        void *p = func_00336DA8_3(16, t);
        *(void **)(self + 684) = p;
        ((s32_Ff4eb83c9 *)p)[1] = 0;
        ((s32_Ff4eb83c9 *)p)[2] = 0;
        ((s32_Ff4eb83c9 *)p)[3] = 0;
        ((s32_Ff4eb83c9 *)p)[0] = 0;
    }
    sub = self + 8;
    ((f32_Ff4eb83c9 *)*(void **)(self + 684))[0] = 255.0f;
    ((f32_Ff4eb83c9 *)*(void **)(self + 684))[1] = 195.0f;
    func_0033ABD8_3(sub, arg, D_L04_001AE070);
    first = func_00288FB8_2(11672);
    second = func_00288FB8_2(11598);
    third = func_00288FB8_2(11599);
    func_0033B050_2(sub, first, second, third);
    func_0033B030_3(sub, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    func_0033B0A8_2(sub, 0.65f);
    *(s32_Ff4eb83c9 *)(self + 680) = 0;
    func_0033C590(self, 0);
}

INCLUDE_ASM("asm/game", func_0033C590);

INCLUDE_ASM("asm/game", func_0033C738);
