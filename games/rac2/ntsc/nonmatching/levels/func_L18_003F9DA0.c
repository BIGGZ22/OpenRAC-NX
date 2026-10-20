/* func_L18_003F9DA0 (176 bytes, 3 places) -- NEAR MISS, not integrated.  Needs the
   -G8 unit (the six angle constants are lwc1 off $gp, gp+0x1440..0x1460).
   Allocates the 0x80 sub-object through func_L00_002D70A0(5) when absent, else
   seeds its three floats from one of three gp triples selected by a1 (1, 2 or
   default).  The switch spelling reproduces retail's compare chain (beq a1,1 /
   beq a1,2 / default) and puts the first store in each case, giving 172 bytes
   against 176 - the one instruction left is block layout: retail keeps the
   default case's p[0] store after the other two blocks, this cc1 emits it inline
   with the case.  Plain if/else-if/else instead inverts the chain (bne) and is
   164 bytes; a struct-copy spelling is 232. */
#include "common.h"

extern s32 D_L18_001C9D40;
extern float D_L18_001B0430 __attribute__((sda));
extern float D_L18_001B0434 __attribute__((sda));
extern float D_L18_001B0438 __attribute__((sda));
extern float D_L18_001B043C __attribute__((sda));
extern float D_L18_001B0440 __attribute__((sda));
extern float D_L18_001B0444 __attribute__((sda));
extern float D_L18_001B0448 __attribute__((sda));
extern float D_L18_001B044C __attribute__((sda));
extern float D_L18_001B0450 __attribute__((sda));
extern void *func_L00_002D70A0(s32);
extern void func_L00_002D7140(void *);

void func_L18_003F9DA0(char *a0, s32 a1)
{
    char *s0 = *(char **)(a0 + 0x68);
    float f1, f2;

    if (D_L18_001C9D40 == 0) {
        if (*(void **)(s0 + 0x80) == 0) {
            *(void **)(s0 + 0x80) = func_L00_002D70A0(5);
        } else {
            switch (a1) {
            case 1:
                *(float *)(*(char **)(s0 + 0x80) + 0x0) = D_L18_001B043C;
                f1 = D_L18_001B0440;
                f2 = D_L18_001B0444;
                break;
            case 2:
                *(float *)(*(char **)(s0 + 0x80) + 0x0) = D_L18_001B0448;
                f1 = D_L18_001B044C;
                f2 = D_L18_001B0450;
                break;
            default:
                *(float *)(*(char **)(s0 + 0x80) + 0x0) = D_L18_001B0430;
                f1 = D_L18_001B0434;
                f2 = D_L18_001B0438;
                break;
            }
            *(float *)(*(char **)(s0 + 0x80) + 0x4) = f1;
            *(float *)(*(char **)(s0 + 0x80) + 0x8) = f2;
        }
    } else {
        func_L00_002D7140(*(void **)(s0 + 0x80));
        *(int *)(s0 + 0x80) = 0;
    }
}
