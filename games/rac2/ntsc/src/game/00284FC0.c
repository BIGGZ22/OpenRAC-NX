#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_a33fc77309a5e461_u32;

extern Rac2Native_a33fc77309a5e461_u32 ref_L00_002F05B8_01;

extern Rac2Native_a33fc77309a5e461_u32 ref_L00_002F05B8_00;

void func_00284FC0(void)
{
    *(Rac2Native_a33fc77309a5e461_u32 *)(ref_L00_002F05B8_01 + 0) = 0x30000009u;
    *(Rac2Native_a33fc77309a5e461_u32 *)(ref_L00_002F05B8_01 + 4) =
        (ref_L00_002F05B8_00 + 0xc0u) & 0x0fffffffu;
    *(Rac2Native_a33fc77309a5e461_u32 *)(ref_L00_002F05B8_01 + 8) = 0;
    *(Rac2Native_a33fc77309a5e461_u32 *)(ref_L00_002F05B8_01 + 12) = 0x50000009u;
    ref_L00_002F05B8_01 += 16;
}

INCLUDE_ASM("asm/game", func_00285030);




INCLUDE_ASM("asm/game", func_00285398);

INCLUDE_ASM("asm/game", func_00285548);
