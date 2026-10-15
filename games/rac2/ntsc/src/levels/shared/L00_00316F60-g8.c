#include "include_asm.h"
#include "common.h"

extern int D_L00_001B1B30;
extern int D_L00_001A9034 __attribute__((sda));
extern int D_L00_001B1B48;
extern int D_L00_001A9038 __attribute__((sda));

void func_L00_0032A138(void) {
    int count = 4;
    int *cursor = &D_L00_001B1B30;

    do {
        *cursor = 0;
        --cursor;
        --count;
    } while (count >= 0);
    D_L00_001A9034 = 0;
}

void func_L00_0032A170(void) {
    int count = 4;
    int *cursor = &D_L00_001B1B48;

    do {
        *cursor = 0;
        --cursor;
        --count;
    } while (count >= 0);
    D_L00_001A9038 = 0;
}

extern int D_L00_001A8FB4;
extern int D_L00_001A8FB4_1 __attribute__((sda));
extern int D_L00_001A8FB0;
extern int D_L00_001B2084;
extern int D_L00_001B2084_1 __attribute__((sda));
extern void func_L00_0030DDF0(void);
extern void func_0028B5D0(int);

int func_L00_003268E8(int value) {
    if (D_L00_001A8FB4 != value)
        return 0;
    if (D_L00_001B2084 == -1)
        return 0;

    D_L00_001A8FB0 = 0;
    D_L00_001A8FB4_1 = 0;
    func_L00_0030DDF0();
    func_0028B5D0(D_L00_001B2084_1);
    D_L00_001B2084 = -1;
    return 1;
}
