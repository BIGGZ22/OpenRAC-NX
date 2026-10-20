#include "include_asm.h"

extern float ref_L02_00385E18_00;

/* Resident fade state, addressed through $gp: the value the setter moves, and its flag. */
extern int ref_L00_0037E670_01;

/* The same state word where it is NOT small data: the retail materialises its address. */
extern float D_L00_001B26B0;

extern void func_32B2C8(int, int, int, int, int);

extern short D_L00_001B2678;
extern short D_L00_001B267A;

float func_L00_0037E668(void)
{
    return ref_L02_00385E18_00;
}

INCLUDE_ASM("asm/levels", func_L00_0037E670);

/* the reset side: clears the absolute word and the flag */
void func_L00_0037E698(void)
{
    D_L00_001B26B0 = 0;
    ref_L00_0037E670_01 = 0;
}

/* records a pair of 16-bit values, then one fixed five-argument engine call */
void func_L00_0037E6A8(int x, int y)
{
    D_L00_001B2678 = x;
    D_L00_001B267A = y;
    func_32B2C8(8, 2, 0, 0, 0);
}
