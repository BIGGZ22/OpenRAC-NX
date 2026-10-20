#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned int u32;

INCLUDE_ASM("asm/levels", func_L09_002AFC90);

extern u8 D_00189E20[];

extern float func_00282E80(void *, void *);
extern float func_002AA2E8(float *, float, float);
extern float func_002AA3A8(float *, float *, float, float, float, float);

/* The level's runtime block: the three channel floats are set from the mode
   word, the two state words and the mode byte, then the aim check decides
   whether the channels are ramped towards their targets or the distance to
   the target is measured. */
void func_L09_002B5CE8(void) {
    int a3;
    int v1;
    float r;

    if (D_00189E20[0x22B4] == 0) {
        *(float *)(D_00189E20 + 0x258) = 0.8f;
        *(float *)(D_00189E20 + 0x25C) = 0.7f;
        *(float *)(D_00189E20 + 0x260) = 0.45f;
    }

    a3 = *(int *)(D_00189E20 + 0x229C);
    if (a3 == 4) {
        if (*(short *)(D_00189E20 + 0x4AC) < *(int *)(D_00189E20 + 0x1B8)) {
            if (D_00189E20[0x49E] == 0) {
                *(float *)(D_00189E20 + 0x25C) = *(float *)(D_00189E20 + 0x4B8);
            }
        }
    } else {
        v1 = *(int *)(D_00189E20 + 0x2294);
        if (v1 == 6) {
            *(float *)(D_00189E20 + 0x25C) = 0.5f;
        } else if (v1 == 4) {
            *(float *)(D_00189E20 + 0x258) = 0.35000002384185791f;
        } else if ((u32)(a3 - 0x11) < 2) {
            *(int *)(D_00189E20 + 0x258) = 0;
            *(int *)(D_00189E20 + 0x25C) = 0;
        } else if (v1 == 0x7C) {
            *(float *)(D_00189E20 + 0x260) = 0.8f;
        }
    }

    if (D_00189E20[0x287] == 0)
        goto slow;
    if (*(int *)(D_00189E20 + 0x22A4) == 0x12)
        goto slow;
    if (*(int *)(D_00189E20 + 0x22B0) == 0x12)
        goto slow;
    if (D_00189E20[0x22B4] == 0) {
        if (*(float *)(D_00189E20 + 0x254) < 0.53f)
            goto slow;
    }
    if (*(int *)(D_00189E20 + 0x229C) == 0x11)
        goto slow;
    if (D_00189E20[0x1494] != 0)
        goto slow;

    r = func_00282E80(D_00189E20 + 0x240, D_00189E20 + 0x80);
    if (r > *(float *)(D_00189E20 + 0x264) * 0.5f)
        goto slow;
    return;

slow:
    {
        /* The channel block sits 0x250 into the runtime block; the block pointer
           is taken first and the runtime block is reached back from it. */
        u8 *p = D_00189E20 + 0x250;
        u8 *s = p - 0x250;
        float step = 0.02f;
        func_002AA2E8((float *)p, *(float *)(s + 0x258), step);
        func_002AA2E8((float *)(p + 4), *(float *)(s + 0x25C), step);
        func_002AA3A8(
            (float *)(p + 0x14),
            (float *)(p + 0x18),
            *(float *)(s + 0x260),
            step,
            0.3f,
            0.06666667f);
    }
}

INCLUDE_ASM("asm/levels", func_L09_002BC490);
