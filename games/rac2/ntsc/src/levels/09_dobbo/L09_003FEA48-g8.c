#include "common.h"

extern f32 D_L09_001B13F0 __attribute__((sda));
extern f32 D_L09_001B13F4 __attribute__((sda));
extern void func_L08_00316678(char *a, s32 b, s32 c, s32 d);

void func_L09_00404B90(char *a0, s32 a1) {
    char *v = a0;
    f32 f = D_L09_001B13F0 * 0.0166666675f;
    char *p = *(char **)(v + 0x68);
    *(f32 *)(p + 0x1D4) = 3.0f;
    *(f32 *)(p + 0x1D0) = f;
    if (*(u8 *)(v + 0x43) == 7) {
        return;
    }
    func_L08_00316678(a0, 7, 0, a1);
}

void func_L09_00404BF0(char *a0, s32 a1) {
    char *v = a0;
    f32 f = D_L09_001B13F4 * 0.0166666675f;
    char *p = *(char **)(v + 0x68);
    *(f32 *)(p + 0x1D4) = 1.75f;
    *(f32 *)(p + 0x1D0) = f;
    if (*(u8 *)(v + 0x43) == 6) {
        return;
    }
    func_L08_00316678(a0, 6, 0, a1);
}
