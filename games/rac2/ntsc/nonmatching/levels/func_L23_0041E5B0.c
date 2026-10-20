#include "include_asm.h"

typedef unsigned char u8;
typedef int s32;
typedef float f32;

/* func_L23_0041E5B0 -- 168 bytes, level 23.  Not placed: the body needs a single
 * 16-byte `lq`/`sq` copy that this cc1 does not emit from a struct assignment.
 *
 * retail:
 *   0x0041E5B0  addiu sp,sp,-0x30 ; v1 = a0 ; sd s0,0x10(sp) ; sd s1,0x18(sp)
 *               s0 = a1 ; sd ra,0x20(sp) ; s1 = t1 (6th argument)
 *   0x0041E5CC  lq v0,0x0(a3) ; beqz t0,+0x54 ; sq v0,0x0(sp) [delay]
 *   0x0041E5D8  mtc1 a2,f12 ; cvt.s.w f12,f12 ; a0 = 0x0018C0B0 ; a1 = v1 ; a2 = s0
 *   0x0041E5F4  jal func_002A8F70 (a3 = sp)
 *   0x0041E604  the same block for the t0 != 0 arm, calling func_002A90C8
 *   0x0041E628  andi v0,s0,0x4 ; bnez v0,+0x8C ; lw v0,0x23E4(gp) [delay]
 *   0x0041E634  beqz s1,+0x98 ; ld s0,0x10(sp) [delay]
 *   0x0041E63C  lui at,0x1B ; sw v0,0x3718(at)
 *   0x0041E648  epilogue
 *
 * So the C is: copy the 4th argument's vector to the stack, convert the 3rd argument to
 * float, call one of two handlers with (0x0018C0B0, a0, a1, f12, &copy), then
 * `if ((a1 & 4) || a5) *(s32 *)0x001B3718 = D_L23_001B13D4;`.
 *
 * measured (wslcmp.py, unit's -g8 sibling, -O2 -G8 -ffunction-sections): `typedef int
 * TI16 __attribute__((mode(TI)));` is the lq/sq spelling - the candidate is then exactly
 * 168 bytes and 41 of 42 words match, the ONE left being the block order of the two
 * handler arms:
 *   +0x20  retail 1100000C beqz $t0,.+4+(0xC<<2)   mine 1500000C bnez $t0,.+4+(0xC<<2)
 * Both source orders (a4 == 0 -> func_002A8F70, and the swapped form) compile to the
 * SAME word, so the fall-through arm is this cc1's choice, not the source's.  Before
 * that, a plain struct copied with ldl/ldr/sdl/sdr and an aligned(16) struct with two
 * ld/sd pairs.  The unit must be the -g8 sibling: retail reads `$gp` at 0x001B13D4, and
 * that file also carries `extern s32 D_0018C0B0 __attribute__((nosda));` which is the
 * loaded 0x0018C0B0 value the two handlers need (not the address).
 */

extern s32 D_0018C0B0 __attribute__((nosda));
extern s32 D_L23_001B13D4 __attribute__((sda));
extern void func_002A8F70(s32, void *, void *, f32, TI16 *);
extern void func_002A90C8(s32, void *, void *, f32, TI16 *);

typedef int TI16 __attribute__((mode(TI)));

void func_L23_0041E5B0(void *a0, s32 a1, s32 a2, TI16 *a3, s32 a4, s32 a5) {
    TI16 v = *a3;

    if (a4 == 0)
        func_002A90C8(D_0018C0B0, a0, (void *)a1, (f32)a2, &v);
    else
        func_002A8F70(D_0018C0B0, a0, (void *)a1, (f32)a2, &v);
    if ((a1 & 4) != 0 || a5 != 0)
        *(s32 *)0x001B3718 = D_L23_001B13D4;
}
