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

/* Seeds the two shared field defaults, then fills the 0x450 record once. */
INCLUDE_ASM("asm/levels", func_L07_003F8780);

/* Arms the spinner on the first pass, otherwise steers its 0x20 angle down. */
INCLUDE_ASM("asm/levels", func_L07_003F6CF0);

/* Steers the 0x54 yaw toward the argument and refreshes the two 0xF8 angles. */
INCLUDE_ASM("asm/levels", func_L07_003F68F0);

/* Feeds the four 0xA8 ids to the shared geometry pass. */
INCLUDE_ASM("asm/levels", func_L07_0040F6B8);
