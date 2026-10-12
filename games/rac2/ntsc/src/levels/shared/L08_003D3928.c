#include "include_asm.h"

/* family c459a266b43242d6 - 352 bytes, 2 placements
   levels/19_grelbin @0x00412F70, levels/8_tabora @0x0041E630

   Fills five output floats from two pinned tables and a resident flag word,
   then clamps the three still free outputs to [-1, 1]. */

extern float D_00189E20_14[];

extern int D_00138180[];

INCLUDE_ASM("asm/levels", func_L08_003D3928);

INCLUDE_ASM("asm/levels", func_L08_003D39F0);

INCLUDE_ASM("asm/levels", func_L08_003D3EB8);

INCLUDE_ASM("asm/levels", func_L08_003D4190);

INCLUDE_ASM("asm/levels", func_L08_003D4600);

INCLUDE_ASM("asm/levels", func_L08_003D4690);

INCLUDE_ASM("asm/levels", func_L08_003DB4A0);

INCLUDE_ASM("asm/levels", func_L08_003DB4E8);

INCLUDE_ASM("asm/levels", func_L08_003DB518);

INCLUDE_ASM("asm/levels", func_L08_003DB8E8);

INCLUDE_ASM("asm/levels", func_L08_003DE5E8);

INCLUDE_ASM("asm/levels", func_L08_003E5BB8);

INCLUDE_ASM("asm/levels", func_L08_003F6018);

INCLUDE_ASM("asm/levels", func_L08_003F6348);

INCLUDE_ASM("asm/levels", func_L08_003F6568);

INCLUDE_ASM("asm/levels", func_L08_003F6610);

INCLUDE_ASM("asm/levels", func_L08_0041E4F8);

/* shared native body of family c459a266b43242d6 */
void func_L08_0041E630(float *a0, float *a1, float *a2, float *a3, float *p4)
{
    float f1;
    float f2;
    int v1;

    *a0 = -D_00189E20_14[2028];
    *a2 = -D_00189E20_14[2029];
    v1 = D_00138180[104];
    f2 = 0.0f;
    if (v1 & 0x40)
        f2 = 1.0f;
    f1 = (v1 & 0x20) ? (f2 - 1.0f) : f2;
    *a1 = f1;
    *a3 = *(float *)&D_00138180[75];
    v1 = D_00138180[104];
    f1 = 0.0f;
    if (v1 & 0x1)
        f1 = 1.0f;
    if (v1 & 0x2)
        f1 = f1 - 1.0f;
    *p4 = f1;
    if (*a0 > 1.0f)
        *a0 = 1.0f;
    else if (*a0 < -1.0f)
        *a0 = -1.0f;
    if (*a1 > 1.0f)
        *a1 = 1.0f;
    else if (*a1 < -1.0f)
        *a1 = -1.0f;
    if (*p4 > 1.0f)
        *p4 = 1.0f;
    else if (*p4 < -1.0f)
        *p4 = -1.0f;
}

INCLUDE_ASM("asm/levels", func_L08_0041E790);

INCLUDE_ASM("asm/levels", func_L08_0041EA88);

INCLUDE_ASM("asm/levels", func_L08_0041EBF8);

INCLUDE_ASM("asm/levels", func_L08_004207D0);

INCLUDE_ASM("asm/levels", func_L08_0042E678);

INCLUDE_ASM("asm/levels", func_L08_0042EF00);

INCLUDE_ASM("asm/levels", func_L08_00435600);

INCLUDE_ASM("asm/levels", func_L08_004356A0);

INCLUDE_ASM("asm/levels", func_L08_004356B8);

INCLUDE_ASM("asm/levels", func_L08_00435938);

INCLUDE_ASM("asm/levels", func_L08_00435990);

INCLUDE_ASM("asm/levels", func_L08_00435A78);
