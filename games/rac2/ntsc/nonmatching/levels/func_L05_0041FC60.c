#include "include_asm.h"

/* The three small-data globals are reached through $gp in retail; the rest are addressed
   absolutely. */
extern int D_L05_00221D10[];

extern unsigned char D_L05_001B2000;

extern unsigned char D_L05_001B2001;

extern unsigned char D_L05_001B2002;

extern float D_L05_001B2004;

extern float D_L05_001B2008;

extern float D_L05_001B200C;

extern float D_L05_001B2010;

extern int D_L05_001B2014;

extern unsigned char D_L05_001B3558;

extern unsigned char D_L05_001B3559;

extern unsigned char D_L05_001B355A;

extern float D_L05_001B355C;

extern float D_L05_001B3560;

extern float D_L05_001B3564;

extern float D_L05_001B3568;

extern int D_L05_001B356C;

extern int D_L05_001D1360 __attribute__((sda));

extern int D_L05_001D1364 __attribute__((sda));

extern int D_L05_001D13E8 __attribute__((sda));

extern void func_379F28(int a0);

extern void func_37D6E8(void);

extern void func_379F68(void);

/* Resets the shared spawn table and copies the mode record into the live one. */
void func_L05_0041FC60(void)
{
    *(float *)&D_L05_00221D10[0xC] = 8.0f;
    D_L05_001D1360 = -2;
    *(int *)(D_L05_00221D10 + 0x28) = 100;
    *(int *)(D_L05_00221D10 + 0x2C) = 1;
    D_L05_001D13E8 = 1;
    D_L05_001D1364 = 0;
    *(int *)&D_L05_00221D10[5] = 0;
    func_379F28(0);
    func_37D6E8();
    func_379F68();
    D_L05_001B2000 = D_L05_001B3558;
    D_L05_001B2001 = D_L05_001B3559;
    D_L05_001B2002 = D_L05_001B355A;
    D_L05_001B2008 = D_L05_001B3560;
    D_L05_001B2010 = D_L05_001B3568;
    D_L05_001B2004 = D_L05_001B355C;
    D_L05_001B200C = D_L05_001B3564;
    D_L05_001B2014 = D_L05_001B356C;
}
