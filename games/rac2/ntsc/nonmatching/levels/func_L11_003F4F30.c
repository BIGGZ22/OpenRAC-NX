/* Correct table extents retained; the small-data address expands by eight bytes.
 * Independent check in the original G8 unit: 0 of 1 places. */
#include "include_asm.h"

#include "common.h"

extern void func_002A7750(u8 *arg0, s32 arg1, s32 arg2);

extern s32 func_002A77E0(s32 arg0);

extern s32 func_L11_00440C80(s32 arg0);

extern s32 D_L11_001B056C __attribute__((sda));

extern s32 D_L11_001B058C __attribute__((sda));

extern f32 D_L11_001B0670;

extern void func_L11_003F33E0(u8 *arg0);

extern void func_L00_002E6EF8(void *arg0, u8 *arg1);

extern void func_L11_003F3CB8(u8 *arg0);

extern void func_002AB8C0(u8 *arg0, void *arg1);

extern void func_002A0488(u8 *arg0, u32 arg1, u32 arg2, u32 arg3);

extern void func_002B0990(u8 *arg0, void *arg1, f32 arg2, s32 arg3, s32 arg4);

typedef int TI128 __attribute__((mode(TI)));

extern void func_0029FC20(u8 *arg0, s32 arg1, u8 *arg2);

extern void func_002830C0(void *arg0, void *arg1, void *arg2);

extern void func_00282CC0(void *arg0, void *arg1, void *arg2);

extern void func_L00_00325AE8(void *arg0, s32 arg1, f32 arg2, f32 arg3);

extern s32 D_L11_001B0674 __attribute__((sda));

extern f32 D_L11_001B0678[4] __attribute__((sda));

extern f32 D_L11_001B0688[4] __attribute__((sda));

extern TI128 D_L11_001B06A0[4] __attribute__((sda));

/* Poses the object's four frames from the level's four pose vectors and the level's four
   angle pairs. */
void func_L11_003F4F30(u8 *arg0) {
    u8 buf[0x50];
    u8 *s0 = buf + 0x10;
    s32 i;

    for (i = 0; i < 4; i++) {
        func_0029FC20(arg0, i + 8, s0);
        func_002830C0(buf, (void *)&((TI128 *)D_L11_001B06A0)[i], s0);
        func_00282CC0(buf, buf, buf + 0x40);
        func_L00_00325AE8(buf, D_L11_001B0674, D_L11_001B0688[i], D_L11_001B0678[i]);
    }
}

/* Arms the object's two state records through two callbacks and the class setup, then, when
   the twin record is clear, starts the 0x2A0 animation. */
INCLUDE_ASM("asm/levels", func_L11_003F2C28);

/* Retires the object and copies two measured state bytes into its state block. */
void func_L11_003F2D38(u8 *arg0) {
    u8 *s0 = arg0;
    s32 v1 = *(s32 *)(s0 + 0x68);
    if (*(u16 *)(v1 + 0x278) >= 2)
        return;
    if (*(u16 *)(v1 + 0x27A) >= 2)
        return;
    if (func_002A77E0(D_L11_001B058C) != 0
        && func_L11_00440C80(*(s32 *)(s0 + 0xB8)) < D_L11_001B056C) {
        func_002A7750(s0, 7, -1);
    } else {
        func_002A7750(s0, 6, -1);
    }
}
