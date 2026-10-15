#include "include_asm.h"

extern int D_001A79F0 __attribute__((sda));
extern void func_00288CB0(int, int);

void func_0029CE80(void)
{
    int value = 0;
    switch (D_001A79F0) {
    case 22: value = 9; break;
    case 23: value = 18; break;
    }
    func_00288CB0(value, 1);
}
