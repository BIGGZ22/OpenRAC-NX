#include "include_asm.h"

/* The published pair lives in small data, so this file is built with -G8. */
extern int D_001B22B4;
extern int D_001B22B0;
extern int D_001B22A0[];
extern int D_001A8BC0;
extern int D_001B16B0;
extern int ref_L00_002FC190_00 __attribute__((sda));

/* Rebuilds the viewport position from the indexed table and the two resident
   offsets, then publishes it and one lowered copy. */
void func_002FC190(void)
{
    int value = D_001B22A0[D_001B22B4] + D_001B22B0 - D_001A8BC0;

    D_001B16B0 = value;
    ref_L00_002FC190_00 = value - 0x2000;
}

extern char D_001BAEC0[];
extern int D_001B22A8;
extern int ref_L00_002FC1D8_00 __attribute__((sda));

/* Seeds the viewport from the resident pair, clears the index and rebuilds. */
void func_002FC1D8(void)
{
    char *base = D_001BAEC0;
    int value = *(int *)(base + 0xC);

    D_001B22A0[0] = value;
    D_001B22A0[1] = *(int *)(base + 0x10);
    D_001B22A8 = value;
    ref_L00_002FC1D8_00 = 0;
    func_002FC190();
}

extern int D_001A79F0;
extern int D_00262D70[];
extern int D_00262DD8[];
extern int ref_L00_002FC138_00 __attribute__((sda));

/* Clamps the resident index, publishes the two selected values and rebuilds. */
void func_002FC138(void)
{
    int index = D_001A79F0;

    if ((unsigned int)index >= 0x19)
        index = 0;
    D_001A8BC0 = D_00262DD8[index];
    ref_L00_002FC138_00 = D_00262D70[index];
    func_002FC190();
}
