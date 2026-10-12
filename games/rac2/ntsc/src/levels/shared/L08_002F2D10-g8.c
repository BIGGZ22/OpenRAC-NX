#include "include_asm.h"
#include "common.h"

extern s32 D_L08_001A9050 __attribute__((sda));
extern f32 D_L08_001A9054 __attribute__((sda));
extern f32 D_L08_001BE0B0 __attribute__((nosda));

extern void func_0027AB40();

s32 func_L08_00300090(void) {
    if (D_L08_001A9050 != 0)
        return 0;
    D_L08_001A9050 = 1;
    D_L08_001A9054 = D_L08_001BE0B0;
    return 1;
}

s32 func_L08_003000C0(void) {
    if (D_L08_001A9050 == 0)
        return 0;
    D_L08_001BE0B0 = D_L08_001A9054;
    func_0027AB40();
    D_L08_001A9050 = 0;
    return 1;
}
