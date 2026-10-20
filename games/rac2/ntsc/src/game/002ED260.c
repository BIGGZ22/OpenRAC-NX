#include "include_asm.h"

#ifndef RAC2_T_U32_FE8672EA5
#define RAC2_T_U32_FE8672EA5
typedef unsigned int u32_Fe8672ea5;
#endif

extern u32_Fe8672ea5 D_L04_001B2AF0[];

extern u32_Fe8672ea5 D_L04_001AC18C __attribute__((sda));

extern float D_L04_001AC190 __attribute__((sda));

extern void func_00282C88(void *target);

void func_002ED260(u32_Fe8672ea5 object)
{
    u32_Fe8672ea5 cursor = object + 40952;
    int i;

    D_L04_001AC18C = object;
    for (i = 511; i >= 0; i--) {
        *(u32_Fe8672ea5 *)cursor = 0;
        cursor -= 80;
    }
    D_L04_001AC190 = 1.0f;
    func_00282C88(D_L04_001B2AF0);
}

INCLUDE_ASM("asm/game", func_002ED2B8);

extern float func_00283CE0(int value);
extern int func_00283CF0(float value);
extern int func_00283D38(int a, int b, float c);
extern float func_00283B98(float a, float b);
extern float func_002A7878(float a, float b);
extern void func_00282CC0(void *a0, void *a1, void *a2);
extern void func_00282D30(void *a0, void *a1, float scale);
extern int func_00282948(void *a0);

void func_002ED368(u32_Fe8672ea5 owner)
{
    int i;
    u32_Fe8672ea5 offset;
    u32_Fe8672ea5 rec;
    float f20;

    if (D_L04_001AC18C == 0) {
        return;
    }
    i = 0;
    offset = 0;
    while (1) {
        rec = offset + D_L04_001AC18C;
        if (*(u32_Fe8672ea5 *)(rec + 0x48) == owner) {
            float ratio = func_00283CE0(*(int *)(rec + 0x40));
            float delta;
            f20 = ratio / func_00283CE0(*(int *)(rec + 0x44));
            *(int *)(rec + 0x38) = func_00283CF0((float)*(int *)(rec + 0x34) * f20);
            *(u32_Fe8672ea5 *)(rec + 0x30) = *(u32_Fe8672ea5 *)(rec + 0x2C);
            *(int *)(rec + 0x30) = func_00283D38(0x003F3F3F, *(int *)(rec + 0x2C), (f20 * f20) * f20);
            *(float *)(rec + 0x24) = func_00283B98(*(float *)(rec + 0x24), *(float *)(rec + 0x20));
            if (*(int *)(rec + 0x3C) != 0) {
                delta = func_002A7878(0.001f, 0.003f);
                *(float *)(rec + 0x28) = *(float *)(rec + 0x28) + delta;
            }
            func_00282CC0((void *)rec, (void *)rec, (void *)(rec + 0x10));
            func_00282D30((void *)(rec + 0x10), (void *)(rec + 0x10), 0.99f);
            if (func_00282948((void *)(rec + 0x40)) != 0) {
                *(u32_Fe8672ea5 *)(rec + 0x48) = 0;
            }
        }
        i++;
        offset += 80;
        if (i >= 512)
            break;
    }
}

#include "include_asm.h"

#ifndef RAC2_T_U32_FE8672EA5
#define RAC2_T_U32_FE8672EA5
typedef unsigned int u32_Fe8672ea5;
#endif

extern u32_Fe8672ea5 D_L04_001AC18C __attribute__((sda));
extern float D_L04_001AC190 __attribute__((sda));

extern void func_00281EB8(void *a0, u32_Fe8672ea5 a1, u32_Fe8672ea5 a2, int a3, float f12, float f13, float f14);

void func_002ED4B8(void)
{
    u32_Fe8672ea5 offset;
    u32_Fe8672ea5 cursor;
    int i;

    if (D_L04_001AC18C == 0) {
        return;
    }
    offset = 0;
    for (i = 511; i >= 0; i--) {
        cursor = D_L04_001AC18C + offset;
        if (*(u32_Fe8672ea5 *)(cursor + 0x48) != 0) {
            float scaled;
            int flag;
            int x;
            flag = 1;
            x = *(int *)(cursor + 0x3C) >> 1;
            if (x != 0) {
                flag = x != 1;
            }
            scaled = (float)*(int *)(cursor + 0x38) * D_L04_001AC190;
            func_00281EB8((void *)cursor, *(u32_Fe8672ea5 *)(cursor + 0x4C),
                          *(u32_Fe8672ea5 *)(cursor + 0x30) | ((u32_Fe8672ea5)(int)scaled << 24),
                          flag,
                          *(float *)(cursor + 0x28), 0.001f, *(float *)(cursor + 0x24));
        }
        offset += 80;
    }
}

INCLUDE_ASM("asm/game", func_002ED570);
