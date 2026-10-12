#include "include_asm.h"

struct IndirectWord
{
    unsigned char reserved[64];
    unsigned int *value;
};

/* Returns the constant integer 1. Matches func_0012F940 disassembly and decompilation. */
int func_0012F940(void) {
    return 1;
}

INCLUDE_ASM("asm/core", func_0012F948);

INCLUDE_ASM("asm/core", func_0012F998);

unsigned int func_0012F9A8(struct IndirectWord *resource)
{
    return *resource->value;
}
