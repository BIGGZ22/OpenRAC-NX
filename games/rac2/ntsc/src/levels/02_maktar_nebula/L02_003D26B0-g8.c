#include "include_asm.h"

extern float func_00282E80(char *, void *);

extern void func_002830C0(void *, void *, void *);

extern void func_0029FC20(char *, int, void *);

extern void *func_0029FCC0(char *, int, void *);

extern int func_002A77E0(int);

extern void func_L00_00325AE8(void *, int, float, float);

extern int D_L02_001B01B0 __attribute__((sda));

int func_L02_003D4D68(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    short timer;
    int delta;

    if (func_00282E80(arg0 + 0x10, &D_L02_001B01B0) > 18.0f) {
        return 0;
    }
    timer = *(short *)(state + 0x32E);
    if (timer < 0x1F4) {
        return 0;
    }
    if (timer < 0x385) {
        delta = 0x384 - *(short *)(state + 0x32A);
        if (delta > 0) {
            if (func_002A77E0(delta) != 0) {
                return 0;
            }
        }
    }
    *(short *)(state + 0x32A) = 0;
    *(short *)(state + 0x32E) = 0;
    return 1;
}

