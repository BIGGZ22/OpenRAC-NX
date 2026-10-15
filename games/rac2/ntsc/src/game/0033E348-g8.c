#include "include_asm.h"
extern void func_0033AF48(char *);
extern void func_00348120(char *, int);
extern void func_00348138(char *);
extern unsigned char D_001A7BB4;
extern int D_001ADED8[2] __attribute__((sda));

void func_0033E428(char *object)
{
    char *p;
    func_0033AF48(object + 8);
    p = object + 0x2B0;
    func_00348120(p, D_001ADED8[D_001A7BB4]);
    func_00348138(p);
}

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern void func_0033ABD8(void *, void *, void *);
extern int func_00288FB8(int);
extern void func_0033B050(void *, int, int, int);
extern void func_0033B030(void *, float, float, float, float, float, float);
extern void func_0033E5C0(void *, int);
extern int D_001ADEF0[2] __attribute__((sda));
void func_0033E4C0(char *object, void *argument)
{
    int *allocation;
    char *child;
    int text;
    *(void **)object = argument;
    if (argument) {
        allocation = func_00336DA8(16, func_00336E40(argument));
        *(int **)(object + 0x2A8) = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    **(float **)(object + 0x2A8) = 255.0f;
    (*(float **)(object + 0x2A8))[1] = 208.0f;
    child = object + 8;
    func_0033ABD8(child, argument, D_001ADEF0);
    text = func_00288FB8(0x2E21);
    func_0033B050(child, text, 0, func_00288FB8(0x2D4F));
    func_0033B030(child, -128.0f, -183.0f, 112.0f, 110.0f, 102.0f, 110.0f);
    func_0033E5C0(object, 0);
}
