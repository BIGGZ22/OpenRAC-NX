#include "include_asm.h"
extern void func_00282CF0(void *, void *, void *);
extern unsigned int func_0031AB08(unsigned int);
extern void func_0031AB18(void *, unsigned int, void *, int);

void func_0031AAB0(void *self)
{
    float local[4];
    void *node = *(void **)(*(char **)((char *)self + 0x68) + 0x30);
    if (node != 0) {
        func_00282CF0(local, (char *)node + 0x10, (char *)self + 0x10);
        func_0031AB18(self, func_0031AB08((unsigned int)self), local, 0);
    }
}
