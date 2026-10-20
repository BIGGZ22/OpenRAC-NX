/* 0 of 2 places match - last line: 22_dobbo_orbit: instruction at +0x30 differs
   Right size (160 bytes). Spawns object class 3413, arms its animation fields, copies the
   caller's 16-byte block into the object's slot and into its private block, then hands it to
   the engine.  Two differences: retail finishes $a1 (`lui $a1,0x5; ori`) and puts the first
   argument's `addiu $a0,$zero,0x47`-style move in the jal delay slot, and it emits the three
   byte/half stores in the source order (sb 0x31, sb 0x30, sh 0x32, sb 0x20) where this cc1
   swaps the `sh` ahead; the final `daddu $a0,$s0,$zero` for func_3180D8 is hoisted early
   here and lands in the jal slot in retail.  Tried and identical: four store orders. */

#include "include_asm.h"

typedef unsigned char u8;
typedef float f32;
typedef int V4;

extern u8 *func_315688(int);
extern void func_3180D8(u8 *);
extern f32 ref_L22_003F85A0_00 __attribute__((sda));

u8 *func_L22_003F85A0(int a0, u8 *a1)
{
    u8 *o = func_315688(3413);

    if (o != 0) {
        u8 *p;

        *(u8 *)(o + 49) = 1;
        *(u8 *)(o + 48) = 0xFF;
        *(unsigned short *)(o + 50) = 0xFF;
        *(u8 *)(o + 32) = 0;
        *(f32 *)(o + 44) = *(f32 *)(*(int *)(o + 36) + 36) * ref_L22_003F85A0_00;
        p = *(u8 **)(o + 104);
        *(int *)(p + 16) = a0;
        *(V4 *)p = *(V4 *)a1;
        *(V4 *)(o + 16) = *(V4 *)a1;
        *(int *)(p + 20) = 0;
        func_3180D8(o);
    }
    return o;
}
