#include "include_asm.h"
#include "common.h"

extern s32 D_L10_001B0F44 __attribute__((sda));
extern s32 D_L10_001B0F48 __attribute__((sda));

s32 func_L10_004176A0(void) {
    s32 v = D_L10_001B0F44;
    D_L10_001B0F44 = 0;
    return v;
}

s32 func_L10_004176B0(void) {
    s32 v = D_L10_001B0F48;
    D_L10_001B0F48 = 0;
    return v;
}
