#include "include_asm.h"

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned long QwenRecovery_339fe89f21bc_WrapperWaitMaskU64;

extern int ref_L00_0037B368_01 __attribute__((sda));

extern void ref_L00_0037B368_00(int ticks);

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_4e44694cc39e7084_u32;

extern Rac2Native_4e44694cc39e7084_u32 ref_L00_0037B3B8_00;

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_ae24050903bcaa08_u32;

extern Rac2Native_ae24050903bcaa08_u32 ref_L00_0037B4C0_00;

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_9da4b6b15b3cf4a0_RecordWord180;

typedef unsigned long long Rac2Native_9da4b6b15b3cf4a0_RecordValue180;

extern Rac2Native_9da4b6b15b3cf4a0_RecordWord180 ref_L00_0037B508_00;

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_3646a0ca5397f72d_u32;

typedef unsigned char Rac2Native_3646a0ca5397f72d_u8;

extern Rac2Native_3646a0ca5397f72d_u32 ref_L00_0037B818_00;

extern Rac2Native_3646a0ca5397f72d_u8 ref_L00_0037B818_01[];

void func_002FC3C0(QwenRecovery_339fe89f21bc_WrapperWaitMaskU64 mask)
{
    while (ref_L00_0037B368_01 & mask)
        ref_L00_0037B368_00(0x400);
}

void func_002FC410(Rac2Native_4e44694cc39e7084_u32 data_address, Rac2Native_4e44694cc39e7084_u32 tag_bits)
{
    *(Rac2Native_4e44694cc39e7084_u32 *)(ref_L00_0037B3B8_00 + 0) = tag_bits | 0x30000000u;
    *(Rac2Native_4e44694cc39e7084_u32 *)(ref_L00_0037B3B8_00 + 4) = data_address;
    *(Rac2Native_4e44694cc39e7084_u32 *)(ref_L00_0037B3B8_00 + 8) = 0;
    *(Rac2Native_4e44694cc39e7084_u32 *)(ref_L00_0037B3B8_00 + 12) = 0;
    ref_L00_0037B3B8_00 += 16;
}

INCLUDE_ASM("asm/game", func_002FC460);

INCLUDE_ASM("asm/game", func_002FC468);

INCLUDE_ASM("asm/game", func_002FC510);

void func_002FC518(Rac2Native_ae24050903bcaa08_u32 final_word)
{
    *(Rac2Native_ae24050903bcaa08_u32 *)(ref_L00_0037B4C0_00 + 0) = 0x10000000u;
    *(Rac2Native_ae24050903bcaa08_u32 *)(ref_L00_0037B4C0_00 + 4) = 0;
    *(Rac2Native_ae24050903bcaa08_u32 *)(ref_L00_0037B4C0_00 + 8) = 0;
    *(Rac2Native_ae24050903bcaa08_u32 *)(ref_L00_0037B4C0_00 + 12) = final_word;
    ref_L00_0037B4C0_00 += 16;
}

void func_002FC560(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 final_word, Rac2Native_9da4b6b15b3cf4a0_RecordValue180 value)
{
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 0) = 0x10000002u;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 4) = 0;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 8) = 0;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 12) = 0x50000002u;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 16) = 0x8001u;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 20) = 0x10000000u;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 24) = 0x0eu;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 28) = 0;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordValue180 *)(ref_L00_0037B508_00 + 32) = value;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 40) = final_word;
    *(Rac2Native_9da4b6b15b3cf4a0_RecordWord180 *)(ref_L00_0037B508_00 + 44) = 0;
    ref_L00_0037B508_00 += 48;
}

INCLUDE_ASM("asm/game", func_002FC618);

INCLUDE_ASM("asm/game", func_002FC620);

INCLUDE_ASM("asm/game", func_002FC740);




INCLUDE_ASM("asm/game", func_002FC868);

void func_002FC870(void)
{
    *(Rac2Native_3646a0ca5397f72d_u32 *)(ref_L00_0037B818_00 + 0) = 0x3000000bu;
    *(Rac2Native_3646a0ca5397f72d_u32 *)(ref_L00_0037B818_00 + 4) = (Rac2Native_3646a0ca5397f72d_u32)ref_L00_0037B818_01;
    *(Rac2Native_3646a0ca5397f72d_u32 *)(ref_L00_0037B818_00 + 8) = 0;
    *(Rac2Native_3646a0ca5397f72d_u32 *)(ref_L00_0037B818_00 + 12) = 0x5000000bu;
    ref_L00_0037B818_00 += 16;
}
