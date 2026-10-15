#include "include_asm.h"

extern float D_001AE0E0 __attribute__((sda));
extern float D_001AE0DC;
extern float D_001AE0D8 __attribute__((sda));
extern void func_00335E38(void *, float, float, float, float);

void func_00341760(char *self)
{
    int index = *(int *)(self + 0x2FC);
    float *v = *(float **)(self + 0x218);
    int *base = (int *)(self + (index << 2));

    func_00335E38(self + 0x130, D_001AE0D8 + v[0],
            (D_001AE0DC + D_001AE0E0 * (float)base[0x6E]) + v[1],
            0.0f, 0.0f);
}

extern float D_001AE0E8 __attribute__((sda));
extern float D_001AE0EC;
extern float D_001AE0F0 __attribute__((sda));
extern float D_001AE0F4 __attribute__((sda));
extern void func_003480F0(void *, float, float);
extern void func_003480D8(void *, int, int);
extern void *func_00347F80(void *, int);
extern int func_00348128(void *);
extern int func_002A9588(int, int, int, int, int);
void func_003417D0(char *self, int arg)
{
    char *indices = self + 0x1B8;
    char *child = self + 0x228;
    int old;
    float *position = *(float **)(self + 0x218);
    func_003480F0(child, position[0] + D_001AE0E8, position[1] + D_001AE0EC);
    func_003480D8(child, (int)D_001AE0F0, (int)D_001AE0F4);
    func_00347F80(child, arg);
    old = *(int *)(indices + (*(int *)(self + 0x2FC) << 2));
    *(int *)(indices + (*(int *)(self + 0x2FC) << 2)) = func_00348128(child);
    indices += *(int *)(self + 0x2FC) << 2;
    if (old != *(int *)indices)
        func_002A9588(0, 0, 1, 0, 1);
}
