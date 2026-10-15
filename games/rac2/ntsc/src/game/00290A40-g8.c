#include "include_asm.h"

extern void func_002E37B0(void);
extern void func_00291158(void);
extern void func_002E3548(void);
extern void func_002E3820(void);
extern void func_002FC560(unsigned int, unsigned long);
extern float D_00255B40[];
extern float *D_001B1990 __attribute__((sda));
extern int D_001A79F0;
extern int D_001A72E0;

void func_00291268(void)
{
    func_002E37B0();
    D_001B1990 = D_00255B40;
    if (D_001A79F0 == 0)
        func_00291158();
    else
        func_002E3548();
    func_002E3820();
    func_002FC560(0x47, 0x5360BUL);
    func_002FC560(0x4E, (D_001A72E0 >> 13) | 0x1000000);
}
