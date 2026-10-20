#include "common.h"

/* The fade alpha of a mesh: the alpha byte of its colours, and the two
   sub-objects' fade values.  Both callees and both fade limits sit at a
   different address in every program that holds this function, so they are
   named after the function that uses them. */
extern int *ref_L00_00338810_00(void *);

extern void ref_L00_00338810_01(void *, float);

extern float ref_L00_00338810_02 __attribute__((sda));

extern float ref_L00_00338810_03 __attribute__((sda));

void func_00338810(void *object)
{
    char *obj = (char *)object;
    u32 *c1;
    u32 *c2;
    u32 *c3;
    u32 *c4;
    int counter = *(int *)(obj + 0x1B8);
    int alpha = *(u8 *)((u8 *)*(u32 **)(obj + 8) + 3);
    int v0;
    int v1;
    float f0;
    float f20;
    float f21;

    if (counter > 0) {
        alpha += 8;
        counter--;
        alpha = (alpha < 0x61) ? alpha : 0x60;
        *(int *)(obj + 0x1B8) = counter;
    } else {
        alpha = (alpha - 8 >= 0) ? alpha - 8 : 0;
    }

    if (alpha == 0) {
        *(int *)*(int *)(obj + 4) = 0;
        return;
    }

    f20 = 0.0f;
    alpha <<= 24;

    c1 = *(u32 **)(obj + 8);
    *c1 = (*c1 & 0x00FFFFFF) | (u32)alpha;
    c2 = *(u32 **)(obj + 0xC);
    *c2 = (*c2 & 0x00FFFFFF) | (u32)alpha;
    c3 = (u32 *)ref_L00_00338810_00(obj + 0x10);
    *c3 = (*c3 & 0x00FFFFFF) | (u32)alpha;
    c4 = (u32 *)ref_L00_00338810_00(obj + 0x5C);
    *c4 = (*c4 & 0x00FFFFFF) | (u32)alpha;

    if (*(int *)(obj + 0x1C4) != 1)
        return;

    v1 = *(int *)(obj + 0x1D4);
    v0 = *(int *)(obj + 0x1B0);
    *(int *)(obj + 0x1BC) = 8;
    *(int *)(obj + 0x1C0) = 6;
    *(int *)(obj + 0x1AC) = ((v0 << 4) >> 1) + 6;
    *(int *)(obj + 0x1A8) = (v1 >> 1) + 8;
    f21 = f20;
    ref_L00_00338810_01(obj + 0x5C, ((float)((v1 >> 1) + 8) - 4.0000000000000000000000e+01f)
            * 4.9504949711263179779053e-03f * 5.6749999523162841796875e-01f
            + 4.9999998882412910461426e-03f);
    if (f21 < ref_L00_00338810_02)
        ref_L00_00338810_01(obj + 0x5C, ref_L00_00338810_02);
    v0 = *(int *)(obj + 0x1AC);
    f0 = (float)v0;
    f20 = (f0 - 1.4000000000000000000000e+01f) * 6.5789474174380302429199e-03f;
    ref_L00_00338810_01(obj + 0x10, f20 * 8.5000002384185791015625e-01f
            + 2.9999999329447746276855e-02f);
    if (f21 <= ref_L00_00338810_03)
        ref_L00_00338810_01(obj + 0x10, ref_L00_00338810_03);
    *(float *)(obj + 0x1D0) = f20 * 3.0600000000000000000000e+02f
            + 2.3500000000000000000000e+01f;
}
