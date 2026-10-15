#include "include_asm.h"

extern char *D_001A8CFC;
extern volatile unsigned char D_001A7A0A;
extern int ref_L00_002D1CB0_01;
extern int ref_L00_002D1CB0_02;
extern int ref_L00_002D1CB0_03 __attribute__((sda));
extern int ref_L00_002D1CB0_04;
extern int ref_L00_002D1CB0_05 __attribute__((sda));
extern int ref_L00_002D1CB0_06;
extern int ref_L00_002D1CB0_07 __attribute__((sda));
extern int ref_L00_002D1CB0_08 __attribute__((sda));

int func_002D1CB0(void)
{
    if (D_001A8CFC != 0) {
        if (D_001A7A0A != 0) {
            *(volatile int *)&ref_L00_002D1CB0_01 = 1;
            *(volatile int *)&ref_L00_002D1CB0_02 = 1;
        }
        if (ref_L00_002D1CB0_03 != 0)
            *(volatile int *)&ref_L00_002D1CB0_04 = 1;
        if (ref_L00_002D1CB0_05 != 0)
            *(volatile int *)&ref_L00_002D1CB0_06 = 1;
        if (ref_L00_002D1CB0_07 != 0)
            ref_L00_002D1CB0_08 = 1;
    }
    return 0;
}
