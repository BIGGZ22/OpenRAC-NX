#include "include_asm.h"

void func_0029B658(void *object, int value)
{
    extern char *D_001A8CFC[] __attribute__((sda));
    extern void func_003384C0(void *, void *, int);
    if (D_001A8CFC[0] == 0) return;
    func_003384C0(D_001A8CFC[0] + 0x36F28, object, value);
}