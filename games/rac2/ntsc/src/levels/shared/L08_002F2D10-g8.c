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

/* The level's shared scalars: the aim values the camera reads, and the object
   the level keeps at a fixed place (its flag at +0x86, its target at +0x70). */
extern f32 D_L08_001B00E0 __attribute__((sda));
extern f32 D_L08_001B00E4 __attribute__((sda));
extern f32 D_L08_001B00E8 __attribute__((sda));
extern f32 D_L08_001B00F0 __attribute__((sda));
extern s32 D_L08_001B00F4 __attribute__((sda));
extern f32 D_L08_001B00F8 __attribute__((sda));
extern u8 *D_L08_001B9B50 __attribute__((nosda));

/* The boot's own vector, the one the aim is measured against. */
extern char D_00189EA0[] __attribute__((nosda));

extern void func_L00_00424A08(s32, f32, f32);
extern void func_L00_00424A50(f32, f32);
/* The prototype's order is the one the call site uses: the first float, then the
   flag, then the second float. The registers are the same either way (the int
   in $a0, the floats in $f12 and $f13). */
extern void func_L00_00424A88(f32, s32, f32);
extern void func_L00_00424B20(f32);
extern void func_L00_00424B70(f32, f32);
extern void func_L00_00424BB8(f32, f32);
extern void func_L00_00424C00(void);
extern void func_L00_00424C28(void);
extern s32 func_002ACBC8(void *, s32);
extern f32 func_00283CE0(s32);

/* Flags the level's object, feeds the camera its aim values and advances the
   counter the aim is scaled by. */
void func_L08_003CD488(s32 *counter, u8 *arg) {
    u8 *owner;
    f32 t, a, b, c;

    owner = *(u8 **)((u8 *)arg + 0x68);
    if (*(s16 *)((u8 *)D_L08_001B9B50 + 0x86) == 0) {
        *(s32 *)((u8 *)*(u8 **)((u8 *)D_L08_001B9B50 + 0x70) + 0x250) = 2;
    }

    func_L00_00424A88(D_L08_001B00F0, D_L08_001B00F4, D_L08_001B00F8);
    func_L00_00424A08(0, D_L08_001B00E4, D_L08_001B00E0);

    if (*(s32 *)((u8 *)owner + 0xC8) >= 0 &&
        func_002ACBC8(D_00189EA0, *(s32 *)((u8 *)owner + 0xC8)) != 0) {
        func_L00_00424B20(0.52359885f);
    }

    func_L00_00424A50(D_L08_001B00E8, 0.003f);
    func_L00_00424C28();

    t = func_00283CE0(*counter) / func_00283CE0(0x5A);
    if (t > 1.0f)
        t = 1.0f;

    a = t * 0.985f + 0.015f;
    b = t * 0.8f + 0.2f;
    c = t * 0.044999998062849045f + 0.015f;

    func_L00_00424B70(a, b);
    func_L00_00424BB8(c, 0.2f);
    func_L00_00424C00();

    if (*counter < 0x5A)
        *counter = *counter + 1;
}
