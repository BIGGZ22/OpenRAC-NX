#include "include_asm.h"

extern unsigned int D_001A7424;
extern int D_001A7420_1;
extern unsigned int D_001A7424_1;
extern int D_001A7420_2;
extern unsigned int D_001A7424_2;
extern int D_001A7420_3;
extern unsigned int D_001A7424_3;
extern int D_001A7420_4;

/* Write state value 3 when bit 0x40 is clear in the absolute flag word; preserve the existing value otherwise. */
void func_L00_00309AE8(void)
{
    if ((D_001A7424 & 0x40u) == 0)
        D_001A7420_1 = 3;
}

/* Write state value 3 when bit 0x40 is clear in the absolute flag word; preserve the existing value otherwise. */
void func_L00_00309B10(void)
{
    if ((D_001A7424_1 & 0x40u) == 0)
        D_001A7420_2 = 3;
}

/* Write state value 3 when bit 0x40 is clear in the absolute flag word; preserve the existing value otherwise. */
void func_L00_00309B58(void)
{
    if ((D_001A7424_2 & 0x40u) == 0)
        D_001A7420_3 = 3;
}

/* Write state value 3 when bit 0x40 is clear in the absolute flag word; preserve the existing value otherwise. */
void func_L00_00309B80(void)
{
    if ((D_001A7424_3 & 0x40u) == 0)
        D_001A7420_4 = 3;
}
