#include "include_asm.h"

extern int D_L01_001A8F40;
extern char D_L01_001A90C8 __attribute__((sda));
extern int D_L01_001A90D0 __attribute__((sda));
extern int func_00282948(char *);
extern void func_0029B810(int);

int func_L01_002F1888(void) {
    if (D_L01_001A8F40 == 0) {
        if (func_00282948(&D_L01_001A90C8))
            func_0029B810(0);
    }
    return D_L01_001A90D0;
}
