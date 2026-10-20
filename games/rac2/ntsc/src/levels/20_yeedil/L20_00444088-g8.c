#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B2090 __attribute__((sda));

extern int D_L20_001B2094 __attribute__((sda));

extern void func_L20_00443B60(char *, char *, char *, float *);

extern void func_L20_0044B100(char *, char *, char *, char *, int, float, int);

extern void func_00282F20(char *, char *, float);

void func_L20_00444088(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 a;
    TI128 b;
    float f;

    f = 0.78539824f;
    func_L20_00443B60(arg0, (char *)&a, (char *)&b, (float *)&f);
    func_00282F20((char *)&b, (char *)&b, D_L20_001B2090 * 0.0166666675f);
    func_L20_0044B100(arg0, (char *)&a, (char *)&b, state + 0x1F0, D_L20_001B2094,
                      (float)*(unsigned char *)(state + 0x35), *(int *)(state + 0x210));
}
