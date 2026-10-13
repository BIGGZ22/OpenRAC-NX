#include "include_asm.h"

extern void func_0027BD70(void);

extern unsigned int D_L00_001B18D4;

void func_L00_002E6348(void) {
 D_L00_001B18D4 = 0x100FF;
 func_0027BD70();
}

extern void func_00284F40(int);

extern void func_0027DDC0(int, int, int, int);

void func_L00_002E6400(void) {
 func_00284F40(0);
 func_0027DDC0(0, 0, 0, 0x80);
 D_L00_001B18D4 = 0xF0;
 func_0027BD70();
}

extern int D_L00_001B19A8;

void func_L00_002E9098(void) {
    D_L00_001B19A8 = 1;
}

void func_L00_002E90A8(void) {
    D_L00_001B19A8 = 0;
}
