#include "include_asm.h"

extern void func_002E37B0(void);

extern void func_002E3548(void);

extern void func_002E3820(void);

extern void func_002FC560(unsigned int, unsigned long);

extern void func_L02_00363D78(void);

extern int D_001A79F0;

extern int D_001A72E0;

extern char D_L02_00280A10[];

extern int D_L02_001B33D0;

void func_L02_00300738(void) {
    func_002E37B0();
    D_L02_001B33D0 = (int)D_L02_00280A10;

    if (D_001A79F0 == 2)
        func_L02_00363D78();
    else
        func_002E3548();
    func_002E3820();
    func_002FC560(0x47, 0x5360B);
    func_002FC560(0x4E, 0x01000000 | (D_001A72E0 >> 13));
}
