#include "include_asm.h"

extern void func_002889B8(unsigned char *);

void func_00288B18(void*);

void func_00288B18(void *owner)
{
    func_002889B8(owner);
}

INCLUDE_ASM("asm/game", func_00288B38);
