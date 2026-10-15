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

/* ---- 0x002E73E0: a counter bump around three measured callees. ---- */

extern int D_L00_001B18D8;
extern int D_001A72DC;
extern int D_001A7344;

extern void ref_L00_002E73E0_00(int);
extern int ref_L00_002E73E0_01(void);
extern void ref_L00_002E73E0_02(void *, int, int);

extern int func_001261F0();

void func_L00_002E73E0(void *a0) {
    ref_L00_002E73E0_00(1);
    func_001261F0(0);
    D_L00_001B18D8 = D_L00_001B18D8 + 1;
    ref_L00_002E73E0_01();
    ref_L00_002E73E0_02(a0, D_001A72DC, D_001A7344 << 11);
}
