#include "include_asm.h"

/* -G8 unit: the two registry words this body passes by address are small data (the retail reaches
 * them with `addiu $a0,$gp,0x35A0`), so the function lives here; see config/cflags.txt. */

extern int D_L00_0022F7C0[];

extern int D_001B2590 __attribute__((sda));

extern int D_001B2594 __attribute__((sda));

extern void func_002F8918(int *a, int *b);

extern void func_L00_00373F48(int *a, int *b);

extern void func_L00_00374348(int *a, int *b);

extern void func_L00_00373D40(int *a, int *b);

/* Dispatch on a mode word held in the level's own data, once the module's registry word is set. */
void func_L00_00374708(void)
{
    int *d = D_L00_0022F7C0;
    int v;

    if (d[0x84 / 4] == 0)
        return;
    v = d[0x50 / 4];
    if (v == 0)
        func_002F8918(&D_001B2590, &D_001B2594);
    else if (v == 1)
        func_L00_00373F48(&D_001B2590, &D_001B2594);
    else if (v == 2)
        func_L00_00374348(&D_001B2590, &D_001B2594);
    else if (v == 3)
        func_L00_00373D40(&D_001B2590, &D_001B2594);
}

extern int D_L00_001AD410 __attribute__((sda));
extern void ref_L00_0036ADB0_00(void *, float);
void func_L00_0036ADB0(void)
{
    int i = 0;
    do {
        ref_L00_0036ADB0_00((void *)(D_L00_001AD410 + i * 64), 1.0f);
        i++;
    } while (i < 64);
}
