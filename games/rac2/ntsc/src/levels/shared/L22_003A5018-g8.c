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

/* The four resident words sit at the level-22 addresses; in the other program that carries
 * func_L22_003B9358 they are elsewhere, so they take per-program names. */
extern int ref_L22_003B9358_00 __attribute__((sda));   /* the level-22 word D_L22_001AFA08 */

extern int ref_L22_003B9358_01 __attribute__((sda));   /* the level-22 word D_L22_001AFA00 */

extern int ref_L22_003B9358_02 __attribute__((sda));   /* the level-22 word D_L22_001AFA0C */

extern int ref_L22_003B9358_03 __attribute__((sda));   /* the level-22 word D_L22_001AFA04 */

extern void func_002E59F8(int handle);

extern void func_00291450(int index);

extern void func_0029F338_22(char *object);

/* Drops the resident slot the argument's record selects: the slot word is released and stamped
 * -1, and the object the slot holds is torn down when its id is 0x5F3 and it has not started
 * (the two resident words it owns are cleared first, and its stored index is released when it is
 * not -1). An object with any other id only clears the slot. */
void func_L22_003B9358(u32 a0)
{
    u32 g = D_0018C0B0[0];
    int *slot;
    char *object;

    if (a0 == g) {
        slot = &ref_L22_003B9358_00;
        if (ref_L22_003B9358_01 != -1) {
            func_002E59F8(ref_L22_003B9358_01);
            ref_L22_003B9358_01 = -1;
        }
    } else {
        slot = &ref_L22_003B9358_02;
        if (ref_L22_003B9358_03 != -1) {
            func_002E59F8(ref_L22_003B9358_03);
            ref_L22_003B9358_03 = -1;
        }
    }
    object = (char *)*slot;
    if (object == 0)
        return;
    if (*(short *)(object + 0xAA) != 0x5F3) {
        *slot = 0;
        return;
    }
    if (*(unsigned char *)(object + 0x20) != 0)
        return;
    {
        char *inner = *(char **)(object + 0x68);
        int index;

        *(int *)0x001A8AB0 = 0;
        *(int *)0x001A8AC0 = 0;
        index = *(short *)(inner + 0x2C);
        if (index != -1)
            func_00291450(index);
    }
    func_0029F338_22((char *)*slot);
    *slot = 0;
}
