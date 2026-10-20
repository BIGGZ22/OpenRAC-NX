#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_967caba148c0b35c_u32;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_00;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_01;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_04;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_05;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_06;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_07;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_08;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_09;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_10;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_11;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_02;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_03;

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_12 __attribute__((sda));

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_13 __attribute__((sda));

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_14 __attribute__((sda));

extern Rac2Native_967caba148c0b35c_u32 ref_L00_002E56B0_15;

void func_0027B7E0(void)
{
    ref_L00_002E56B0_00 = 0;
    ref_L00_002E56B0_01 = 0;
    ref_L00_002E56B0_04 = 0;
    ref_L00_002E56B0_05 = 0;
    ref_L00_002E56B0_06 = 0;
    ref_L00_002E56B0_07 = 0;
    ref_L00_002E56B0_08 = 0;
    ref_L00_002E56B0_09 = 0;
    ref_L00_002E56B0_12 = 0;
    ref_L00_002E56B0_13 = 0;
    ref_L00_002E56B0_14 = 0;
    ref_L00_002E56B0_10 = 0;
    ref_L00_002E56B0_11 = 0;
    ref_L00_002E56B0_02 = 0;
    ref_L00_002E56B0_03 = 0;
    ref_L00_002E56B0_15 = 0;
}

extern char *D_001B22A8 __attribute__((sda));
extern char *ref_L00_0027B858_00;
extern char D_001391D0[];
extern char D_00139120[];
extern char D_001B9040[];
extern void func_002FC560(unsigned int, unsigned long);

void func_0027B858(void)
{
    unsigned long packed;
    *(int *)(ref_L00_0027B858_00 + 0) = 0x30000013;
    *(char **)(ref_L00_0027B858_00 + 4) = D_001391D0;
    *(int *)(ref_L00_0027B858_00 + 8) = 0;
    *(int *)(ref_L00_0027B858_00 + 12) = 0x50000013;
    {
        char *old = ref_L00_0027B858_00;
        ref_L00_0027B858_00 = old + 16;
        *(int *)(old + 16) = 0x3000000B;
    }
    *(char **)(ref_L00_0027B858_00 + 4) = D_00139120;
    *(int *)(ref_L00_0027B858_00 + 8) = 0;
    *(int *)(ref_L00_0027B858_00 + 12) = 0x5000000B;
    {
        char *base = D_001B9040;
        int w0 = *(int *)(base + 0x230);
        int w1 = *(int *)(base + 0x234);
        int w2 = *(int *)(base + 0x238);
        packed = (unsigned long)w0 | ((unsigned long)w1 << 8) | ((unsigned long)w2 << 16);
    }
    D_001B22A8 = ref_L00_0027B858_00 + 16;
    func_002FC560(0x3D, packed);
}
