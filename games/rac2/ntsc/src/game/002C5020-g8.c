#include "include_asm.h"

extern int D_001AA3B4 __attribute__((sda));
extern void func_002A8860(void *, int);
extern int func_00282978(void *);
extern void func_002B9180(void *);

void func_002C5020(char *object)
{
    func_002A8860(object + 4, D_001AA3B4);
    if (func_00282978(object + 0xA))
        func_002B9180(object);
}
