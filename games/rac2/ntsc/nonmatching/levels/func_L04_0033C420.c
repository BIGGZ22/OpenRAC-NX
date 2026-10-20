/* func_L04_0033C420 (160 bytes, 2 places) -- NEAR MISS, not integrated.
   Decoded (04_barlow): two 16-byte buffers are copied out of a0, the +8 floats of each are
   nudged by the two incoming floats (add f12, sub f13, clamped at 0 for the second), the
   pair is handed to func_002761F8(v1, v2, 0x22, a1, 0), and the result is zeroed by two
   flags before being returned:
     0033C434  lq    $v0, 0x0($a0)        <- 16 bytes of the parameter into buffer 1
     0033C438  sq    $v0, 0x0($sp)
     0033C44C  lq    $v0, 0x0($a0)        <- the same 16 bytes into buffer 2 at 0x10($sp)
     0033C450  sq    $v0, 0x10($sp)
     0033C494  slt   $a0, -1, A           ; movz  $v0,$zero,$a0   -> returns 0 when !(A > -1)
     0033C4A8  movn  $a0, $zero, B        -> returns 0 when (a2 != 0 && B != 0)

   The wall is the frame layout: retail is 0x30 with the buffers at 0x00 and 0x10 and the
   block base materialised for the two flag loads at +0x18/+0x1C. Measured: the copies as
   `*(TI128 *)v = *(TI128 *)a0` (frame 0x40, 37 diffs), as a struct assignment (212 bytes),
   as `memcpy(v, a0, 16)` with s32 or float buffers (212 bytes), and with the guard written
   as `!= -1`/`<= -1`/conditional - all four shapes land on the same diffs. Retail's lq/sq
   pair is a single 16-byte copy with the source address in a register, which none of these
   spellings produced here. */

typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);
extern u8 D_L04_001AFAC0[];

int func_L04_0033C420(void *a0, void *a1, s32 a2, float f12, float f13)
{
    float v1[4];
    float v2[4];
    u8 *base = D_L04_001AFAC0;
    s32 r;

    *(int (*)[4])v1 = *(int (*)[4])a0;
    *(int (*)[4])v2 = *(int (*)[4])a0;
    v1[2] += f12;
    v2[2] -= f13;
    if (v2[2] < 0.0f)
        v2[2] = 0.0f;
    r = func_002761F8(v1, v2, 0x22, a1, 0);
    if (*(s32 *)(base + 0x1C) <= -1)
        r = 0;
    if (a2 != 0 && *(s32 *)(base + 0x18) != 0)
        r = 0;
    return r;
}
