#include "include_asm.h"

extern float D_L00_001B13D8 __attribute__((sda));
extern int D_L00_001B13C4 __attribute__((sda));
extern int D_L00_001B13C8 __attribute__((sda));
extern int D_L00_001B13CC __attribute__((sda));
extern int D_L00_001B13D0 __attribute__((sda));
extern void func_002FC560(int, int);
extern void func_L00_00413918(char *, int, int, int, int);

void func_L00_00413870(char *object)
{
    char *state = *(char **)(object + 0x68);
    float value = *(float *)(state + 0x14) - D_L00_001B13D8;

    *(float *)(state + 0x14) = value;
    if (value <= -8.0f) {
        *(float *)(state + 0x14) = value + 8.0f;
    }
    func_002FC560(0x47, 0x51001);

    func_L00_00413918(object, *(int *)(state + 0), D_L00_001B13C4, D_L00_001B13CC, 10);
    func_L00_00413918(object, *(int *)(state + 4), D_L00_001B13C8, D_L00_001B13D0, 20);
}
