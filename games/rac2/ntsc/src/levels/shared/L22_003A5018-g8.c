#include "include_asm.h"

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

/* The resident global is an array here on purpose: with a scalar declaration
 * cc1 folds the load and reuses the destination as the address base, while the
 * retail materialises the address in its own register first -- declaring the
 * symbol as an array and indexing it [0] restores the two-instruction form
 * (`lui $v1,%hi; lw $a1,%lo($v1)`). */
extern u32 D_0018C0B0[];

extern int D_L22_001AFA0C __attribute__((sda));

extern int D_L22_001AFA08 __attribute__((sda));

extern float D_L22_001AF3EC __attribute__((sda));

extern float D_L22_001AF5C8 __attribute__((sda));

extern s16 D_0018A16E[];

extern f32 func_00282C48(f32 f);

s32 func_L22_003B9428(u32 a0) {
    u32 g = D_0018C0B0[0];
    int *p = (a0 == g) ? &D_L22_001AFA08 : &D_L22_001AFA0C;
    return *p != 0;
}

s32 func_L22_003A61A8(u8 *a0) {
    u8 *s = *(u8 **)(a0 + 0x68);
    f32 t;
    if (*(f32 *)(s + 0x3DC) < D_L22_001AF3EC) return 1;
    t = func_00282C48(*(f32 *)(s + 0x414) - *(f32 *)(s + 0x410));
    if (D_0018A16E[0] < 0xB) return 0;
    if (*(f32 *)(s + 0x3E0) < D_L22_001AF3EC) {
        if (t < D_L22_001AF5C8) return 1;
    }
    return 0;
}
