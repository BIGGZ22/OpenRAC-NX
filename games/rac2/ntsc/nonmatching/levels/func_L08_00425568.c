/* 0 of 1 places match - last line: 08_tabora: 196 bytes, expected 192
   The plain `if (byte31 != 0) store 0; else counter++` reading gives retail's instruction
   set and the guard/call/float tail, but two things stay outside it, both in the 0x1BC
   block at +0x1c.
   (a) Retail's `sh $zero,0x1BC($s1)` sits in the delay slot of a `bnel $v0,$zero` that
   SKIPS the increment, so in retail the store and the `lhu`/`addiu +1`/`sh` all run on the
   byte31 == 0 path - i.e. the source holds BOTH statements under `if (byte31 == 0)`, and
   the store really is dead (`counter = 0` then `counter + 1`). This cc1 forwards the store
   into the load and folds the pair to `addiu $v0,$zero,1 ; sh $v0,0x1BC($s1)`, dropping
   eight bytes (see form 1). Blocking the forward - a distinct `char *r` loaded from the
   same 0x68 slot, or `r = s1`, or a local copy - does not: cc1 CSEs them to one pseudo and
   still folds. Marking only the LOAD `volatile` does block the fold and brings the unit to
   retail's 192 bytes, but it also costs the annulled branch (`bnez` with a `lw` in the slot
   instead of `bnel` with the store), see form 2.
   (b) With the fold blocked the rest of the body lines up except allocation: retail hoists
   `lw $s1,0x68($s0)` into the delay slot of the 0x96 guard's `beqz` and keeps the 0x20
   byte in `$v1`, this cc1 loads it after the branch and uses `$v0`, and the 2.0f constant
   lands in `$f0` where retail uses `$f2`. */
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;
typedef float f32;

extern void func_002F9A90(char *a);

/* Per-frame tick of the record's 0x68 context: bump the two saturating counters, pass the
   0x1B0 block to the shared step, refresh the 0x1BE timer while the 0x20 mode is not yet
   down to 2, then stamp the transition fields once it reaches 2. */
void func_L08_00425568(char *a0)
{
    char *s0 = a0;
    char *s1;

    if (*(u16 *)(s0 + 0x96) < 0x7FFD)
        *(u16 *)(s0 + 0x96) = *(u16 *)(s0 + 0x96) + 1;
    s1 = *(char **)(s0 + 0x68);
    if (*(u8 *)(s0 + 0x31) != 0)
        *(u16 *)(s1 + 0x1BC) = 0;
    else
        *(u16 *)(s1 + 0x1BC) = *(u16 *)(s1 + 0x1BC) + 1;
    func_002F9A90(s1 + 0x1B0);
    if (*(u8 *)(s0 + 0x20) >= 3) {
        if (*(s16 *)(s1 + 0x1BE) < 0x78)
            *(u16 *)(s1 + 0x1BE) = *(u16 *)(s1 + 0x1BE) + 1;
    }
    if (*(u8 *)(s0 + 0x20) >= 2) {
        *(f32 *)(s0 + 0x74) = 2.0f;
        *(u8 *)(s0 + 0x6F) = *(u8 *)(s0 + 0x32);
        *(f32 *)(s0 + 0x70) = *(f32 *)(s1 + 0x1C4) - *(f32 *)(s0 + 0x18);
        *(u8 *)(s0 + 0xBD) = 0;
    }
}
