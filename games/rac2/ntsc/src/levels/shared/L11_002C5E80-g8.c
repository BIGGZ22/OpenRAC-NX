#include "include_asm.h"

/* Level 11's own data: the addresses are the ones in level 11, and every other
 * level that holds this function reads its own copy of them. */
extern float D_L11_001B2600 __attribute__((sda));
extern float D_L11_001B2604 __attribute__((sda));
extern int D_L11_001B25E8 __attribute__((sda));
extern int D_L11_001B25F0 __attribute__((sda));
extern float D_L11_001B2610 __attribute__((sda));
extern float D_L11_001B2614 __attribute__((sda));
extern float D_L11_001B2628 __attribute__((sda));
extern int D_L11_001B2618 __attribute__((sda));
extern int D_L11_001B261C __attribute__((sda));
extern float D_L11_001B2620 __attribute__((sda));
extern float D_L11_001B2624 __attribute__((sda));
extern float D_L11_001B262C __attribute__((sda));

/* The level's table of entries: addressed absolutely, never $gp-relative. */
extern char D_L11_00264760[] __attribute__((nosda));

extern void func_L11_0046C1E0(int a0, int a1, int a2, int a3, float f12, float f13, float f14);

/* Clamps the level's timer and hands two table entries to the level's helper:
 * the first at the table's head, the second one record further on. */
void func_L11_0046C118(char *a0)
{
    char *s2 = *(char **)(a0 + 0x68);
    float v;

    v = D_L11_001B2600 - D_L11_001B2604;
    D_L11_001B2600 = v;
    if (v <= -8.0f) {
        D_L11_001B2600 = v + 8.0f;
    }
    func_L11_0046C1E0(*(int *)(s2 + 0x38) * 640 + (int)D_L11_00264760,
                      D_L11_001B25E8, D_L11_001B25F0, *(int *)(s2 + 0x2C) - 2,
                      D_L11_001B2610, D_L11_001B2614, D_L11_001B2628);
    func_L11_0046C1E0(*(int *)(s2 + 0x38) * 640 + (int)(D_L11_00264760 + 0x140),
                      D_L11_001B2618, D_L11_001B261C, *(int *)(s2 + 0x2C),
                      D_L11_001B2620, D_L11_001B2624, D_L11_001B262C);
}
