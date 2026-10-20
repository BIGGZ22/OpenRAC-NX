#include "common.h"

extern void func_002A7750(u8 *a0, u8 a1, s32 a2);

extern int func_002A9640(char *a0, int a1, int a2);

extern int func_002E5CF8(int index, int a1, char *a2);

extern void func_L00_0032A258(void *a0, void *a1, void *a2, void *a3, int a4, int a5,
                               int a6, int a7, int a8, u8 a9, int a10);

extern void func_0029F338(void *object);

/* The three float pairs the level passes on, at $gp + 0x2830, + 0x2838 and + 0x2840;
   their addresses differ from level to level, so they carry no address. */
extern f32 ref_L07_00410B48_00[2] __attribute__((sda));
extern f32 ref_L07_00410B48_01[2] __attribute__((sda));
extern f32 ref_L07_00410B48_02[2] __attribute__((sda));

/* The state byte at 0x20 picks the path: 0 clears the moby list of class 1, 1 asks the
   spawner and marks the object, 2 replaces the moby and runs the level's setup. */
void func_L07_00410B48(u8 *a0)
{
    u8 *s0 = a0;
    u8 *s1 = *(u8 **)(s0 + 0x68);

    switch (s0[0x20]) {
    case 0:
        func_002A7750(s0, 1, -1);
        break;
    case 1:
        if (func_002A9640(s0, 0x04010000, 0) == 0)
            goto done;
        if (*(int *)s1 != 0)
            goto done;
        func_002A7750(s0, 2, -1);
    done:
        s0[0xA8] = 0xFF;
        break;
    case 2:
        func_002E5CF8(0, 0, s0);
        func_L00_0032A258(s0, ref_L07_00410B48_02, ref_L07_00410B48_01,
                          ref_L07_00410B48_00, 0xFFFF, 0, -1, 0, 0xFFFF, 0xFF, 0);
        func_0029F338(s0);
        break;
    }
}
