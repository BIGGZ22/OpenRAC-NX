#include "include_asm.h"

typedef int s32;

extern s32 func_00133490(s32);

extern int func_00132C48(int, int, void *);

/* lot15 -- two twins of the first call-bearing body (same form, 32 bytes).
   The matched wrappers use a `nop` delay slot and return 1; the slot alone does not establish the callee parameter list. */
extern void func_001338C8();

extern s32 func_00133930(s32, s32);

int func_00133490(int value)
{
    int payload[4];
    payload[0] = value;
    return func_00132C48(0x36, 4, payload);
}

INCLUDE_ASM("asm/core", func_001334B8);

INCLUDE_ASM("asm/core", func_00133580);

INCLUDE_ASM("asm/core", func_00133640);

INCLUDE_ASM("asm/core", func_00133688);

INCLUDE_ASM("asm/core", func_001336C0);

INCLUDE_ASM("asm/core", func_00133700);

INCLUDE_ASM("asm/core", func_00133750);

INCLUDE_ASM("asm/core", func_00133788);

INCLUDE_ASM("asm/core", func_001337C8);

INCLUDE_ASM("asm/core", func_00133818);

INCLUDE_ASM("asm/core", func_00133848);

INCLUDE_ASM("asm/core", func_00133890);

INCLUDE_ASM("asm/core", func_001338B8);

void func_001338C8(void)
{
  func_00132C48(0x3c, 0, 0);
}

INCLUDE_ASM("asm/core", func_001338F0);

INCLUDE_ASM("asm/core", func_00133928);

int func_00133930(int first, int second)
{
    struct {
        unsigned int first;
        unsigned int second;
        unsigned int pad[2];
    } payload;
    payload.first = first;
    payload.second = second;
    return func_00132C48(0x5a, 8, &payload.first);
}

INCLUDE_ASM("asm/core", func_00133960);
