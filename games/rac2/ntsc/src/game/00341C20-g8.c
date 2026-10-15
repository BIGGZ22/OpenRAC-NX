#include "include_asm.h"

extern float D_001AE180 __attribute__((sda));
extern float D_001AE17C;
extern float D_001AE178 __attribute__((sda));
extern void func_00335E38(void *, float, float, float, float);
void func_003423C8(char *self)
{
    int index = *(int *)(self + 0x16C);
    float *v = *(float **)(self + 0x180);
    func_00335E38(self + 0xE4, D_001AE178 + v[0],
            (D_001AE17C + D_001AE180 * (float)index) + v[1],
            0.0f, 0.0f);
}

extern float D_001AE188 __attribute__((sda));
extern float D_001AE18C;
extern float D_001AE190 __attribute__((sda));
extern float D_001AE194 __attribute__((sda));
extern void func_003480F0(void *, float, float);
extern void func_003480D8(void *, int, int);
extern void *func_00347F80(void *, int);
extern int func_00348128(void *);
void func_00342428(char *self, int arg)
{
    char *child = self + 0x188;
    float *position = *(float **)(self + 0x180);
    func_003480F0(child, position[0] + D_001AE188, position[1] + D_001AE18C);
    func_003480D8(child, (int)D_001AE190, (int)D_001AE194);
    func_00347F80(child, arg);
    *(int *)(self + 0x16C) = func_00348128(child);
}
extern float D_001AE198 __attribute__((sda));
extern float D_001AE19C;
extern float D_001AE1A0 __attribute__((sda));
extern float D_001AE1A4;
extern float D_001AE1A8[2] __attribute__((sda));
extern int func_00348130(void *);
extern void func_003363C0(void *, float);
extern void func_00342420(void *);
int func_003424C0(char *self, int arg)
{
    char *child = self + 0x4C;
    float *position = *(float **)(self + 0x180);
    float *scale;
    unsigned int index;
    func_00335E38(self, D_001AE198 + position[0], D_001AE19C + position[1], 0.0f, 0.0f);
    position = *(float **)(self + 0x180);
    func_00335E38(child, D_001AE1A0 + position[0], D_001AE1A4 + position[1], 0.0f, 0.0f);
    index = func_00348130(self + 0x188) - 1;
    scale = (float *)((char *)D_001AE1A8 + (index << 2));
    func_003363C0(self, *scale);
    func_003363C0(child, *scale);
    func_00342428(self, arg);
    func_003423C8(self);
    func_00342420(self);
    return *(int *)(self + 0x16C);
}
extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern char D_001ADC10[], D_001ADC18[];
extern char *D_001A8CFC[] __attribute__((sda));
extern float D_001AE12C __attribute__((sda));
extern void func_003361E0(void *, void *, void *);
extern int *func_00335E20(void *);
extern void func_00336390(void *, void *, int);
extern void func_00347EC8(void *, void *);
extern void func_00341EE8(void *, int);
void func_00341CE0(char *object, void *argument)
{
    int *allocation;
    char *first, *second, *widget;
    *(void **)(object + 4) = argument;
    if (argument) {
        allocation = func_00336DA8(16, func_00336E40(argument));
        *(int **)object = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    **(float **)object = 250.0f;
    (*(float **)object)[1] = 120.0f;
    first = object + 0xDC;
    second = object + 0x128;
    widget = object + 0x10;
    func_003361E0(first, D_001ADC10, argument);
    func_003361E0(second, D_001ADC18, argument);
    *func_00335E20(first) = 0x60442D00;
    *func_00335E20(second) = 0x55F0C070;
    func_00336390(first, D_001A8CFC[0] + 0x8710, 0x73);
    func_00336390(second, D_001A8CFC[0] + 0x8710, 0x74);
    func_003363C0(first, D_001AE12C);
    func_003363C0(second, D_001AE12C);
    func_00347EC8(widget, argument);
    func_003480D8(widget, 11, 60);
    func_00341EE8(object, 0);
}
