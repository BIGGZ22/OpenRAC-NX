#include "common.h"
#include "structs.h"

extern int func_0011EB00(int arg0, int arg1, int arg2, void *arg3);

/* rac1-decomp src/core/0011CCE0.c: func_0011D078 */
int func_0011ED08(int arg0, int arg1, int arg2) {
    char buf[0x10];
    return func_0011EB00(arg0, arg1, arg2, buf);
}

INCLUDE_ASM("asm/core", func_0011ED28);
