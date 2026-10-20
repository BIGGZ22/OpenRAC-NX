/* func_L01_00416B30 (212 bytes, 1 place: 01_oozla) -- NEAR MISS.
   state: exactly 212 bytes, every instruction and every constant in place; the first difference is
   the prologue's saved-register pair. check.py-style ruler (wslcmp, 01_oozla @0x00416b30):
     00416B40  addiu $s1, $zero, 0x3   |  addiu $s2, $zero, 0x3
     00416B44  sd    $s2, 0x10($sp)    |  sd    $s1, 0x8($sp)
     00416B48  lw    $s2, 0x68($a0)    |  lw    $s1, 0x68($a0)
     00416B4C  addiu $s0, $s2, 0x20    |  addiu $s0, $s1, 0x20
   Retail keeps the loop counter in $s2 and the handle in $s1 and saves both; this cc1 assigns the
   counter $s1 and the handle $s2, so the frame and the whole body shift. This is the recorded
   swapped-callee-saved wall.

   The last four stores are also ordered differently: retail's bytes are 0x5C, 0x58, 0x50, 0x54
   and this cc1 emits 0x50, 0x54, 0x5C, 0x58 for every source order tried.

   What is measured and right:
   - the second float is -2.25f (0xC0100000), not -2.5f;
   - the first loop is `for (i = 3; i >= 0; i--) *p++ = (f32)func_3104E8(0xFF);` with the counter
     decremented in the jal's delay slot and the store following the `cvt.s.w`;
   - the second loop stores 0xFF, 0xBF, 0x7F, 0x3F descending from h+0x4C with `v -= 0x40` in the
     `bgez` delay slot, and its two assembled nops are the loop's alignment padding.

   Tried and not enough: declaring the counter before the handle, assigning it in the for header,
   and **all 24 permutations of the four tail stores** (the lead's lever 1, brute-forced as 24
   `_vN` variants in one ruler call) - every one still reports its first difference at +0x8, i.e.
   the $s1/$s2 pair is fixed before the stores are even reached, so the store order is not the
   observable fault here. */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern s32 func_3104E8(s32 mode);

/* Arms the measured primitive block of the given object. */
void func_L01_00416B30(u8 *a0)
{
    u8 *h = *(u8 **)(a0 + 0x68);
    f32 *p = (f32 *)(h + 0x20);
    s32 *q;
    s32 i;
    s32 v;

    for (i = 3; i >= 0; i--)
        *p++ = (f32)func_3104E8(0xFF);
    *(f32 *)(h + 0x30) = -1.0f;
    *(f32 *)(h + 0x34) = -2.25f;
    *(f32 *)(h + 0x38) = 1.25f;
    *(f32 *)(h + 0x3C) = 2.5f;
    q = (s32 *)(h + 0x4C);
    v = 0xFF;
    for (i = 3; i >= 0; i--) {
        *q-- = v;
        v -= 0x40;
    }
    *(f32 *)(h + 0x5C) = 2.5f;
    *(f32 *)(h + 0x58) = 3.0f;
    *(f32 *)(h + 0x50) = 2.5f;
    *(f32 *)(h + 0x54) = 3.0f;
}
