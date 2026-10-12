#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_ff971bf89acc2217_u32;

typedef unsigned char Rac2Native_ff971bf89acc2217_u8;

extern Rac2Native_ff971bf89acc2217_u32 ref_L00_0037B6F0_00;

extern Rac2Native_ff971bf89acc2217_u8 ref_L00_0037B6F0_01[];

extern Rac2Native_ff971bf89acc2217_u32 ref_L00_0037B750_00;

extern Rac2Native_ff971bf89acc2217_u8 ref_L00_0037B750_01[];

extern Rac2Native_ff971bf89acc2217_u32 ref_L00_0037B7B0_00;

extern Rac2Native_ff971bf89acc2217_u8 ref_L00_0037B7B0_01[];

void func_L00_0037B6F0(void)
{
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B6F0_00 + 0) = 0x30000003u;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B6F0_00 + 4) = (Rac2Native_ff971bf89acc2217_u32)ref_L00_0037B6F0_01;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B6F0_00 + 8) = 0;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B6F0_00 + 12) = 0x50000003u;
    ref_L00_0037B6F0_00 += 16;
}

void func_L00_0037B750(void)
{
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B750_00 + 0) = 0x30000003u;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B750_00 + 4) = (Rac2Native_ff971bf89acc2217_u32)ref_L00_0037B750_01;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B750_00 + 8) = 0;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B750_00 + 12) = 0x50000003u;
    ref_L00_0037B750_00 += 16;
}

void func_L00_0037B7B0(void)
{
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B7B0_00 + 0) = 0x30000003u;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B7B0_00 + 4) = (Rac2Native_ff971bf89acc2217_u32)ref_L00_0037B7B0_01;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B7B0_00 + 8) = 0;
    *(Rac2Native_ff971bf89acc2217_u32 *)(ref_L00_0037B7B0_00 + 12) = 0x50000003u;
    ref_L00_0037B7B0_00 += 16;
}

INCLUDE_ASM("asm/levels", func_L00_0037B810);

INCLUDE_ASM("asm/levels", func_L00_0037DD18);
