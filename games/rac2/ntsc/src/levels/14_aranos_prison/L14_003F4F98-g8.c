#include "include_asm.h"

extern int D_L14_001B17D8 __attribute__((sda));
extern int D_L14_001B17DC;
extern int D_001A72E0;

extern int func_2F6E20(int a, int b, float c);
extern void func_385CB8(unsigned int reg, unsigned long value);
extern void func_3030C0(int, int, int, int, int, int);

/* Plays one sound effect derived from the owner's state block and then queues the level jingle. */
void func_L14_00427250(char *a0)
{
    char *v0;
    int s0;

    v0 = *(char **)(a0 + 0x68);
    s0 = func_2F6E20(D_L14_001B17D8, D_L14_001B17DC, *(float *)v0);
    func_385CB8(0x42, 0x8000000044UL);
    func_3030C0(0, 0, 600, 500, s0, 0);
    func_385CB8(0x4E, 0x1000000UL | (D_001A72E0 >> 13));
}
