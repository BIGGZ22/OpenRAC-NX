#include "include_asm.h"

extern char D_00189E20[];

extern float D_L00_001A7FA4 __attribute__((sda));

extern float D_L00_001A7F9C __attribute__((sda));

int func_L00_002D64B8(float *a0, float *a1, float *a2)
{
    char *p = D_00189E20;
    int v = *(int *)(p + 8852);

    if (v >= 98 && v <= 100) {
        if (a0) *a0 = *(float *)(p + 6316);
        if (a1) *a1 = D_L00_001A7FA4 * 4.8481374506081920117140e-06f;
        if (a2) *a2 = D_L00_001A7F9C * 2.9088824521750211715698e-04f;
        return 1;
    }
    if (v == 17) {
        if (a0) *a0 = *(float *)(p + 152);
        if (a1) *a1 = D_L00_001A7FA4 * 1.4544411897077225148678e-05f;
        if (a2) *a2 = D_L00_001A7F9C * 8.7266473565250635147095e-04f;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002D6588);
