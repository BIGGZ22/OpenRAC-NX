#include "common.h"

/* Small-data unit: the bodies whose state lives at $gp-relative offsets. */

extern void func_00281EB8(void *a0, void *a1, void *a2, float f12, float f13, float f14,
                          int a3);

extern float func_002A7940(void);

extern int func_002A7820(int a0, int a1);

extern float func_002A7878(float a, float b);

extern void func_L11_00458DF0(char *a0, void *a1, void *a2, int a3, int t0, float f12,
                              float f13);

extern void func_00282F20(char *a0, void *a1, float f12);

extern void func_002AB668(void *a0, void *a1, float f12);

extern void func_00282D30(char *a0, void *a1, float f12);

extern void func_002ACDE8(void *a0, void *a1, void *a2);

extern void func_00282CC0(void *a0, void *a1, void *a2);

extern float func_00283BE0(float a, float b);

extern void func_L00_002E6CF0(void *a0, void *a1);

extern void func_L11_00459E48(char *p);

extern void *D_L11_001B1DB4 __attribute__((sda));

extern void *D_L11_001B1DB8 __attribute__((sda));

extern float D_L11_001B1DBC __attribute__((sda));

extern void *D_L11_001B1FBC __attribute__((sda));

extern void *D_L11_001B1FC0 __attribute__((sda));

extern float D_L11_001B1FC8 __attribute__((sda));

extern float D_L11_001B1FCC __attribute__((sda));

extern float D_L11_001B1FFC __attribute__((sda));


/* Rebuilds the state's offset block from its rotation and installs the next handler. */
void func_L11_00459D48(char *p)
{
    char *q = *(char **)(p + 0x68);
    float v[4];

    func_00282F20((char *)v, q + 0x40, 1.0f);
    func_002AB668(v, v, *(float *)(q + 0x30));
    func_00282D30(q + 0x10, q, *(float *)(q + 0x60));
    func_002ACDE8(q + 0x10, q + 0x10, v);
    *(float *)(q + 0x20) = -*(float *)(q + 0x10);
    *(float *)(q + 0x24) = -*(float *)(q + 0x14);
    *(float *)(q + 0x28) = -*(float *)(q + 0x18);
    func_00282CC0(q + 0x10, p + 0x10, q + 0x10);
    func_00282CC0(q + 0x20, p + 0x10, q + 0x20);
    *(float *)(q + 0x30) = func_00283BE0(*(float *)(q + 0x30),
                                         D_L11_001B1FFC * 0.000290888245f);
    func_L00_002E6CF0(func_L11_00459E48, p);
}
