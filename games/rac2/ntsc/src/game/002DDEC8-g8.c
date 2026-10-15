#include "include_asm.h"

/* The resident flag is small data: this file is built with -G8. Its address
   differs from program to program, so it has no level in its name. */
extern int ref_L00_002DF190_00 __attribute__((sda));
extern void func_002E5E58(int, int, int);

void func_002DF190(int a0, int a1)
{
    if (ref_L00_002DF190_00 != 0)
        func_002E5E58(a0, a1, 0);
}
