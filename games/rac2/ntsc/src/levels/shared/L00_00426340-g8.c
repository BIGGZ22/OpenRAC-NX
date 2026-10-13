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
