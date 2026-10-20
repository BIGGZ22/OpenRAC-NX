#include "include_asm.h"

extern char D_00189E20[];

extern int D_L23_001B374C __attribute__((sda));

extern int D_L23_001B374C_2;

extern int D_L23_001B3750 __attribute__((sda));

extern int D_L23_001B3750_2;

extern int D_L23_001B3754;

extern int D_L23_001B14A4 __attribute__((sda));

extern char D_L23_001B1F90[];

/* Raises one of three level values by its step and clamps it to its limit. Retail reads the
 * entry word again after the stores to the globals; built with -fno-gcse, as here, this cc1
 * does too (gcse otherwise keeps the earlier load across those stores). The stores through
 * $gp and through $at are retail's two forms of the same globals. */
int func_L23_0041D1F0(char *obj, int unused, int mode, int flag) {
    char *state = *(char **)(obj + 0x68);
    char *entry = D_L23_001B1F90 + flag * 16;
    int result = 0;

    switch (mode) {
    case 0:
        if (flag) {
            char *g = D_00189E20;

            if (*(int *)(g + 0x1874) < *(short *)(state + 0x2F4)) {
                *(int *)(g + 0x1874) = *(int *)(g + 0x1874) + *(short *)(state + 0x2F0);
                if (*(short *)(state + 0x2F4) < *(int *)(g + 0x1874))
                    *(int *)(g + 0x1874) = *(short *)(state + 0x2F4);
                result = 1;
                D_L23_001B374C = 0;
            }
        } else {
            float lim = (float)*(short *)(state + 0x24);

            if (*(float *)(state + 0x20) < lim) {
                *(float *)(state + 0x20) = *(float *)(state + 0x20) + (float)*(short *)(state + 0x2F2);
                if (lim < *(float *)(state + 0x20))
                    *(float *)(state + 0x20) = lim;
                result = 1;
            }
        }
        break;
    case 1:
        if (*(int *)(entry + 8) < *(short *)(state + 0x2FA)) {
            if (*(short *)(state + 0x2FA) < *(int *)(entry + 8) + *(short *)(state + 0x2F8))
                D_L23_001B3750 = *(short *)(state + 0x2FA) - *(int *)(entry + 8);
            else
                D_L23_001B3750_2 = *(short *)(state + 0x2F8);
            *(int *)(entry + 8) = *(int *)(entry + 8) + *(short *)(state + 0x2F8);
            if (*(short *)(state + 0x2FA) < *(int *)(entry + 8))
                *(int *)(entry + 8) = *(short *)(state + 0x2FA);
            result = 1;
            if (flag)
                D_L23_001B374C = 1;
        }
        break;
    case 2:
        if (*(int *)(entry + 12) < *(short *)(state + 0x2FE)) {
            *(int *)(entry + 12) = *(int *)(entry + 12) + *(short *)(state + 0x2FC);
            if (*(short *)(state + 0x2FE) < *(int *)(entry + 12))
                *(int *)(entry + 12) = *(short *)(state + 0x2FE);
            result = 1;
            if (flag)
                D_L23_001B374C_2 = 2;
        }
        break;
    }
    if (result)
        D_L23_001B3754 = D_L23_001B14A4;
    return result;
}
