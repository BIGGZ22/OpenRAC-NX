#include "include_asm.h"

/* These four resident thunks need the -mno-split-addresses profile to come out exact:
 * without it this cc1 schedules the constant's `lui` between retail's `lui $v0,%hi` /
 * `lw $v0,%lo($v0)` pair and loads into $v1. The flag cannot cover the whole source unit
 * (func_L00_004450C0 in L00_00433560.c only matches with the default profile), so they
 * live here, in a unit of their own. */

extern char *D_L00_001B272C[];

extern int func_0034FAE8(char *);

extern void func_0034FAF8(int *);

extern void func_00350DC8(char *);

extern void func_00350ED8(char *);

/* Loads the resident pointer word, adds 0xD9100, and calls the update helper with it. */
int func_L00_00444D18(void)
{
    return func_0034FAE8(D_L00_001B272C[0] + 0xD9100);
}

/* The release twin of func_L00_00444D18. */
void func_L00_004450E8(void)
{
    func_0034FAF8((int *)(D_L00_001B272C[0] + 0xD9100));
}

/* The same thunk with the first sub-object's lookup helper and a constant of 0xD9090. */
int func_L00_00447678(void)
{
    func_00350DC8(D_L00_001B272C[0] + 0xD9090);
    return 1;
}

/* The twin of func_L00_00447678 with the second sub-object's lookup helper. */
int func_L00_004476A8(void)
{
    func_00350ED8(D_L00_001B272C[0] + 0xD9090);
    return 1;
}
