#include "include_asm.h"

extern int D_L02_001B04DC __attribute__((sda));

extern void func_L02_003DBA70();

extern void func_0029FCC0(char *arg0, int arg1, char *arg2);

extern char D_L02_001B0F60[4] __attribute__((sda));

extern int D_L02_001B0FA0 __attribute__((sda));

extern int D_L02_001B0FA4;

extern void func_2EEFB0(char *, int, char *, char *, int, int, int, int, float);

/* Hands the actor's four widget slots to the effect scheduler. */
void func_L02_003F16A8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    func_2EEFB0(D_L02_001B0F60, 4, state + 0x2A0, state + 0x3A0,
                *(int *)(state + 0x4D4), *(int *)(state + 0x4D0), D_L02_001B0FA0,
                D_L02_001B0FA4, *(float *)(state + 0x4D8));
}

/* Rebuild the four widget descriptions from the resident template. */
void func_L02_003DB9E0(char *arg0) {
    char buffer[16];

    func_0029FCC0(arg0, 2, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 4, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 8, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
    func_0029FCC0(arg0, 6, buffer);
    func_L02_003DBA70(buffer, D_L02_001B04DC);
}
