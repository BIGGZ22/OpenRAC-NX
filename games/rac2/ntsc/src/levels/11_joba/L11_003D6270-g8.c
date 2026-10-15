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

extern void func_L11_003F4F30(u8 *arg0);

extern void func_002AB8C0(u8 *arg0, void *arg1);

extern void func_002A0488(u8 *arg0, u32 arg1, u32 arg2, u32 arg3);

extern void func_002B0990(u8 *arg0, void *arg1, f32 arg2, s32 arg3, s32 arg4);

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
