#include "include_asm.h"

extern void Ffb202470_FUN_0033AB50(char *p);

char *func_0033C9B8(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033C9E8);
