#include "include_asm.h"

#include "common.h"

extern f32 func_2E5318(f32 a, f32 b);

extern f32 func_2E4918(f32 a, f32 b);

extern f32 D_L07_001B0F84 __attribute__((sda));

extern u8 *D_L07_001B22DC __attribute__((sda));

extern void func_317520(u8 *a, u8 *b, u8 *c, u8 *d, u8 *e, u8 *f);

extern void func_421288(s32 a, f32 b, f32 c);

extern void func_4212D0(f32 a, f32 b);

extern u8 *func_2CD050(s32 a);

extern void func_3109C8(u8 *a, s32 b, s32 c);

extern s32 func_312FA8(u8 *a, s32 b, s32 c);

extern f32 func_313008(u8 *a, s32 b);

extern void func_313DD8(void *a, f32 b, f32 c);

extern f32 D_L07_001B10B8 __attribute__((sda));

extern f32 D_L07_001B10BC __attribute__((sda));

extern u32 D_L07_001B10C0 __attribute__((sda));

extern f32 D_L07_001B0FD0 __attribute__((sda));

extern f32 D_L07_001B0E98 __attribute__((sda));

extern f32 D_L07_001B0E9C __attribute__((sda));

extern f32 D_L07_001B0EA0 __attribute__((sda));

extern f32 func_002AA2E8(f32 *value, f32 target, f32 step);

extern void func_002A7750(u8 *a, u8 b, s32 c);

extern void func_0029F338(void *a);

extern f32 func_00283B98(f32 a, f32 b);

extern void func_L00_002E6CF0(void *a, void *b);

extern void func_L07_003F4C18(void *a, void *b, void *c);

extern s32 func_L01_003120A8(void *a, void *b, void *c, s32 d, f32 e, f32 f, s32 g, s32 h, s32 i);

extern char func_L07_003F4CC0[];

/* Advances the object's own state: three step kinds by the 0x20 flag, then the shared
   spin and the two sampled vectors. */
void func_L07_003F4970(u8 *a0)
{
    u8 *s1 = a0;
    u8 *s2 = *(u8 **)(a0 + 0x68);
    u8 v1 = *(u8 *)(a0 + 0x20);
    f32 v[4];
    f32 w[4];

    switch (v1) {
    case 0:
        {
            u32 n = *(u16 *)(s1 + 0x96) + 1;

            *(u16 *)(s1 + 0x96) = n;
            if ((u16)n == 1) {
                break;
            }
            func_002AA2E8((f32 *)(s2 + 0x24), 1.0f, D_L07_001B0E98 * (1.0f / 60.0f));
            if (1.0f <= *(f32 *)(s2 + 0x24)) {
                func_002A7750(s1, 1, -1);
            }
        }
        break;
    case 1:
        func_002AA2E8((f32 *)(s2 + 0x24), 0.0f, D_L07_001B0E9C * (1.0f / 60.0f));
        if (*(f32 *)(s2 + 0x24) <= 0.0f) {
            func_0029F338(s1);
            return;
        }
        break;
    }
    *(f32 *)(s2 + 0x28) = func_00283B98(*(f32 *)(s2 + 0x28), D_L07_001B0EA0 * 0.00029088825f);
    func_L00_002E6CF0(func_L07_003F4CC0, s1);
    if (0.333f < *(f32 *)(s2 + 0x24)) {
        func_L07_003F4C18(s1, v, w);
        func_L01_003120A8(*(u32 *)(s1 + 0xB8), v, w, 1, *(f32 *)(s2 + 0x2C), 1.0f, 0, 1, 0);
    }
}

/* Seeds the two shared field defaults, then fills the 0x450 record once. */
INCLUDE_ASM("asm/levels", func_L07_003F8780);

/* Arms the spinner on the first pass, otherwise steers its 0x20 angle down. */
INCLUDE_ASM("asm/levels", func_L07_003F6CF0);

/* Steers the 0x54 yaw toward the argument and refreshes the two 0xF8 angles. */

/* Feeds the four 0xA8 ids to the shared geometry pass. */
INCLUDE_ASM("asm/levels", func_L07_0040F6B8);
