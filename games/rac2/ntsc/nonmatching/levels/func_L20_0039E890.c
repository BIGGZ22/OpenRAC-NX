/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   The body is five instructions short of the retail's 360 bytes and the
   structure is right: the walk over the record list, the per-record
   update (the 0xFFBE/0x1000 mask on the u16 at +0x34, the copy of the
   int at +0x10 of the pointer at +0x24 into +0x98, the call to
   func_0032F5B0 with (p, 2, -1, w)) and the float store through the
   $gp base D_L20_001B308C all match.

   What still differs, and what was measured:

   - The retail keeps the constants 1 and 0xFF in callee-saved
     registers ($s6 and $s5) because they are compared inside the
     loop, across the call to func_0032F5B0.  This attempt
     rematerialises them (`addiu $v0,$zero,0x1`).  Spelling them as C
     variables (`u32 ff = 0xFF; u32 one = 1;`) does put them in
     registers, but then the frame grows to 0x60 (one register too
     many) and the `addiu $sp,$sp,-0x50` no longer matches.  The
     retail fits both in the frame because its state pointer load
     (`lw $s5,0x68($s2)`, in the jal's delay slot) is dead: the
     register is reused for the constant 0xFF right after.

   - The retail's `lw $s5,0x68($s2)` is a dead load of
     *(arg0 + 0x68).  The attempt does not have it at all (gcc
     removes a dead load), and a `volatile` spelling of it spills the
     value to the stack instead.

   - The retail's exit paths are three separate labels
     (.L0039E9C8 restores $s0, .L0039E9CC restores $s1, .L0039E9D0
     the rest) and the two of them that skip a restore use a
     branch-LIKELY (`beql $s1,$zero,.L0039E9D0` with `ld $s1,0x8($sp)`
     in the delay slot, `beql $s0,$zero,.L0039E9CC` with
     `ld $s0,0x0($sp)`).  This attempt has one shared exit and plain
     `beqz`/`beq`.  The branch-likely form needs the loop's check
     block to end in an unconditional jump to the body, the way
     func_L20_0039D680's does (see the note there); in this body the
     check block ends in the fall-through to the exit, and no
     spelling tried (an explicit `goto out;`, `return;` instead of
     the label, the value load moved into the two `if`s) makes the
     cc1 invert the branches.

   - The retail's `lw $a3,0x24($s0)` loads the fourth argument of
     func_0032F5B0 straight into the argument register and uses the
     same register for the address (`lw $v1,0x10($a3)`); this attempt
     loads it into $t0 and copies it (`daddu $a3,$t0,$zero`), one
     instruction too many.

   The body below is the closest attempt (85 instructions against the
   retail's 90).
*/
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern void func_0032F5B0(void *, s32, s32, s32);

extern u16 *D_L20_001FBB20[];

/* The retail body of func_L20_0039E890 reaches D_L20_001B2F9C through $at/$lo. */
extern char *D_L20_001B2F9C __attribute__((nosda));

/* The retail body of func_L20_0039E890 reaches D_L20_001B2FB8 through $at/$lo. */
extern int D_L20_001B2FB8_1 __attribute__((nosda));

/* 0x1AEFF0 + 0x409C : the retail body reaches this one through $gp. */
extern char *D_L20_001B308C __attribute__((sda));

void func_L20_0039E890(s32 index, float arg1) {
    u16 **slot = &D_L20_001FBB20[index];
    u16 *rec = *slot;
    float f = arg1;
    float g;
    u16 val;
    u32 mask;

    if ((index & 0xFF) == 0xFF) {
        goto out;
    }
    if (D_L20_001B2FB8_1 < index) {
        goto out;
    }
    if (rec == 0) {
        goto out;
    }
    mask = index & 0xFF;
    g = 0.0f;
    val = *rec;
    goto body;
body:
    {
        char *p = D_L20_001B2F9C + ((val & 0x7FFF) << 8);

        if (p == 0) {
            goto out;
        }
        if (*(u8 *)(p + 0x20) != 1) {
            u16 v = *(u16 *)(p + 0x34);
            s32 w = *(s32 *)(p + 0x24);

            v = (v & 0xFFBE) | 0x1000;
            *(u16 *)(p + 0x34) = v;
            *(int *)(p + 0x98) = *(int *)((u8 *)w + 0x10);
            func_0032F5B0(p, 2, -1, w);
            if (g < f) {
                char *t = *(char **)(p + 0x68);

                *(float *)(D_L20_001B308C + (*(int *)(t + 0x84) << 7) + 0x38) = f;
            }
        }
    }
check:
    if (mask == 0xFF) {
        goto out;
    }
    rec++;
    if (D_L20_001B2FB8_1 < index) {
        goto out;
    }
    if (rec == 0) {
        goto out;
    }
    if (*(s16 *)(rec - 1) >= 0) {
        val = *(u16 *)rec;
        goto body;
    }
    if (rec == *slot) {
        val = *(u16 *)rec;
        goto body;
    }
out:
    return;
}
