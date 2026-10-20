#include "include_asm.h"

extern unsigned char D_L05_001B2000;
extern unsigned char D_L05_001B2001;
extern unsigned char D_L05_001B2002;
extern float D_L05_001B2004 __attribute__((sda));
extern float D_L05_001B2008 __attribute__((sda));
extern float D_L05_001B200C __attribute__((sda));
extern float D_L05_001B2010 __attribute__((sda));
extern int D_L05_001B2014;
extern unsigned char D_L05_001B3630;
extern unsigned char D_L05_001B3631;
extern unsigned char D_L05_001B3632;
extern float D_L05_001B3634 __attribute__((sda));
extern float D_L05_001B3638 __attribute__((sda));
extern float D_L05_001B363C __attribute__((sda));
extern float D_L05_001B3640 __attribute__((sda));
extern int D_L05_001B3644;

void func_L05_00442720(void)
{
    unsigned int b0 = D_L05_001B2000;
    unsigned int b1 = D_L05_001B2001;
    unsigned int b2 = D_L05_001B2002;
    float f0 = D_L05_001B2008;
    float f1 = D_L05_001B2010;
    float f2 = D_L05_001B2004;
    float f3 = D_L05_001B200C;
    unsigned int w = D_L05_001B2014;

    D_L05_001B3630 = b0;
    D_L05_001B3631 = b1;
    D_L05_001B3632 = b2;
    D_L05_001B3638 = f0;
    D_L05_001B3640 = f1;
    D_L05_001B3634 = f2;
    D_L05_001B363C = f3;
    D_L05_001B3644 = w;
}
