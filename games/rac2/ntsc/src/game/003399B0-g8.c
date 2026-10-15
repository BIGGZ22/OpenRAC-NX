#include "include_asm.h"

/* The X bias lives in small data, so this file is built with -G8. */
extern float ref_L00_0033A688_00 __attribute__((sda));
extern float D_001ADD0C;
extern void func_00335E38(char *, float, float, float, float);

/* Offsets the record position by the two resident biases and hands it to the
   shared spawn helper with no rotation. */
void func_0033A688(char *a0)
{
    if (*(int *)(a0 + 0x20C) != 0) {
        float *position = *(float **)(a0 + 0x1FC);

        func_00335E38(a0 + 0x170, position[0] + ref_L00_0033A688_00,
                      position[1] + D_001ADD0C, 0.0f, 0.0f);
    }
}
