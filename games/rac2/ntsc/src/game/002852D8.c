#include "include_asm.h"

extern char *D_001B22A8 __attribute__((sda));
extern char D_001A6600[];
extern char D_001A6860[];

void func_002852D8(void)
{
    *(int *)(*(char * volatile *)&D_001B22A8 + 0) = 0x30000026;
    *(char **)(*(char * volatile *)&D_001B22A8 + 4) = D_001A6600;
    *(int *)(*(char * volatile *)&D_001B22A8 + 8) = 0;
    *(int *)(*(char * volatile *)&D_001B22A8 + 12) = 0x50000026;
    D_001B22A8 = *(char * volatile *)&D_001B22A8 + 16;
}


void func_00285338(void)
{
    *(int *)(*(char * volatile *)&D_001B22A8 + 0) = 0x30000029;
    *(char **)(*(char * volatile *)&D_001B22A8 + 4) = D_001A6860;
    *(int *)(*(char * volatile *)&D_001B22A8 + 8) = 0;
    *(int *)(*(char * volatile *)&D_001B22A8 + 12) = 0x50000029;
    D_001B22A8 = *(char * volatile *)&D_001B22A8 + 16;
}
