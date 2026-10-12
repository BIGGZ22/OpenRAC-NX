#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L16_002B5EE0);

INCLUDE_ASM("asm/levels", func_L16_002E5440);

INCLUDE_ASM("asm/levels", func_L16_00395250);

INCLUDE_ASM("asm/levels", func_L16_00395858);

INCLUDE_ASM("asm/levels", func_L16_003959B8);

INCLUDE_ASM("asm/levels", func_L16_00395C00);

INCLUDE_ASM("asm/levels", func_L16_00395D30);

INCLUDE_ASM("asm/levels", func_L16_00395ED0);

/* Read and return the float at linked offset 0x 34 through the pointer stored at owner+0x 68. */
float func_L16_003960E8(const unsigned char *owner)
{
    const unsigned char *linked = *(const unsigned char * const *)(owner + 0x68);
    return *(const float *)(linked + 0x34);
}

INCLUDE_ASM("asm/levels", func_L16_003960F8);

INCLUDE_ASM("asm/levels", func_L16_003A8D58);

INCLUDE_ASM("asm/levels", func_L16_003A90E0);

INCLUDE_ASM("asm/levels", func_L16_003C2C08);

INCLUDE_ASM("asm/levels", func_L16_003C32F8);

INCLUDE_ASM("asm/levels", func_L16_003C3870);

INCLUDE_ASM("asm/levels", func_L16_003C3A10);

INCLUDE_ASM("asm/levels", func_L16_004051A8);

INCLUDE_ASM("asm/levels", func_L16_00405818);

INCLUDE_ASM("asm/levels", func_L16_00406080);

INCLUDE_ASM("asm/levels", func_L16_00406550);

INCLUDE_ASM("asm/levels", func_L16_0040BED0);

INCLUDE_ASM("asm/levels", func_L16_0040C5F0);

INCLUDE_ASM("asm/levels", func_L16_0040C7E8);

INCLUDE_ASM("asm/levels", func_L16_00413850);

INCLUDE_ASM("asm/levels", func_L16_00414038);

INCLUDE_ASM("asm/levels", func_L16_0041F950);

INCLUDE_ASM("asm/levels", func_L16_0042FB58);

INCLUDE_ASM("asm/levels", func_L16_004364B0);
