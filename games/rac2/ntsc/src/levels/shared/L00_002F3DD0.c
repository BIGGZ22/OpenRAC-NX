#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

typedef struct { u32 items[5]; } NativeTable20;

extern const NativeTable20 D_L00_001A8EB0;

extern void func_002889B8(unsigned char *);

/* indexed copy of overlay-owned five-entry table */
u32 func_L00_002F3DD0(s32 index) {
    NativeTable20 values=D_L00_001A8EB0;
    return values.items[index];
}

INCLUDE_ASM("asm/levels", func_L00_002F4018);

/* Pass the supplied owner pointer to the measured owner initializer. */
void func_L00_002F4180(void *owner)
{
    func_002889B8(owner);
}

/* Resident byte, below the level's own region so the same address holds in every program.
   The literal address is what the retail shape needs: a named symbol makes this cc1 form the
   address in $v1 and load through it, the constant folds into lui $v0 + lbu $v0,lo($v0). */
unsigned char func_L00_002F43D0(void)
{
    return *(unsigned char *)0x001A7A0A;
}
