#include "include_asm.h"

extern char *D_001AF880 __attribute__((sda));

void func_00317390(void)
{
    char *p = D_001AF880;
    if (p != 0) {
        char *q = *(char **)(p + 0x68);
        *(int *)(q + 0x28) = 1;
    }
}
