#include "include_asm.h"

extern char *D_001B22A8 __attribute__((sda));
extern char D_001AC480[];

void func_002FC7A8(void)
{
    *(int *)(*(char * volatile *)&D_001B22A8 + 0) = 0x30000003;
    *(char **)(*(char * volatile *)&D_001B22A8 + 4) = D_001AC480;
    *(int *)(*(char * volatile *)&D_001B22A8 + 8) = 0;
    *(int *)(*(char * volatile *)&D_001B22A8 + 12) = 0x50000003;
    D_001B22A8 = *(char * volatile *)&D_001B22A8 + 16;
}
