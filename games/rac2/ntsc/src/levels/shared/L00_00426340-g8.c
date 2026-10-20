#include "include_asm.h"

extern void ref_L00_00428560_00(char *);

extern void ref_L00_0042ABF8_00(char *);

extern void ref_L00_0042B688_00(char *);

void func_L00_00428560(char *o)
{
    ref_L00_00428560_00(o);
}

void func_L00_0042ABF8(char *o)
{
    ref_L00_0042ABF8_00(o);
}

void func_L00_0042B688(char *o)
{
    ref_L00_0042B688_00(o);
}
extern char D_L00_001ADC68[4] __attribute__((sda));

extern char D_L00_001ADC48[4] __attribute__((sda));

extern void func_L00_0042B408(char *);

void func_L00_0042B6A8(char *a0)
{
    *(char **)a0 = D_L00_001ADC68;
    func_L00_0042B408(a0);
}

void func_L00_0042B6C8(char *a0)
{
    *(char **)a0 = D_L00_001ADC48;
    func_L00_0042B408(a0);
}

/* The global this one writes sits at a different address from level to level, and it is
 * not the one func_L00_004272A0 reads in every level, so it takes a name of its own. */
extern int ref_L00_00427290_00 __attribute__((sda));

void func_L00_00427290(void)
{
    ref_L00_00427290_00 = 1;
}


extern int D_L00_001B2700 __attribute__((sda));

extern int D_L00_001B15DC __attribute__((sda));

int func_L00_004272A0(void)
{
    return D_L00_001B15DC;
}

int func_L00_0042B730(void)
{
    return D_L00_001B2700;
}
