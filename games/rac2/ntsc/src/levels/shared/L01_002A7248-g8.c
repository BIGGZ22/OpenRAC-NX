#include "include_asm.h"

extern int D_L01_001A8F40;
extern char D_L01_001A90C8 __attribute__((sda));
extern int D_L01_001A90D0 __attribute__((sda));
extern int func_00282948(char *);
extern void func_0029B810(int);

int func_L01_002F1888(void) {
    if (D_L01_001A8F40 == 0) {
        if (func_00282948(&D_L01_001A90C8))
            func_0029B810(0);
    }
    return D_L01_001A90D0;
}

extern int D_001B2088;
extern int D_001B2458;

int func_L01_00305E18(int a0)
{
    int a1 = a0;
    int p;

    if (D_001B2088 < 5 || (p = D_001B2458) == 0)
        return -1;
    return *(short *)(p + a1 * 0x28 + 0x24);
}

/* func_L01_002F1F70 is defined in L01_002A7248.c (this unit has no $gp access for it). */

typedef unsigned char u8;
typedef signed short s16;
typedef float f32;

extern u8 D_L01_00189E20[];

/* The same object as plain D_L01_001A8F40 above: that use stays absolute, this one is the
   $gp form the reach test reads. */
extern int D_L01_001A8F40_gp __asm__("D_L01_001A8F40") __attribute__((sda));

extern f32 func_002E3618(void *a0, void *a1);

/* Reach test against the level's live record: rejects the two "gone" states, an empty
   pair, a closed door, a nonzero key field, the third mode and the $gp flag, then asks
   the helper whether the doubled radius fits, and finally whether the stored count
   covers the caller's index. */
int func_L01_003245E0(u8 *a0, int a1, f32 f12) {
    u8 *s2 = D_L01_00189E20;
    int s0 = a1;
    f32 f20 = f12;
    int v1 = *(int *)(s2 + 0x2294);
    f32 f0;

    if (v1 == 0x1C)
        return 0;
    if (v1 == 0x31)
        return 0;
    if (*(int *)(s2 + 0x24CC) == 0)
        return 0;
    if (*(int *)(s2 + 0x340) == 0)
        return 0;
    if (*(s16 *)(s2 + 0x34E) != 0)
        return 0;
    if (*(int *)(s2 + 0x229C) == 3)
        return 0;
    if (D_L01_001A8F40_gp != 0)
        return 0;
    f0 = func_002E3618(a0 + 0x10, s2 + 0x80);
    if (f20 + f20 < f0)
        return 0;
    return D_001B2088 >= s0;
}
