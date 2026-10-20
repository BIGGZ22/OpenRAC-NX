/* func_L04_0033D0C0 (148 bytes, 2 places) -- NEAR MISS, not integrated.
   Decoded (04_barlow): a 16-byte vector is copied out of a0+0x10 into a stack buffer, the
   buffer's first two floats are each advanced by a call scaled by 100.0f, and the pair of
   buffers (the second one at +0x10, only ever addressed) is handed to func_L04_0033CC00:
     0033D0DC  addiu $v1, $s0, 0x10      <- the source pointer kept as a value
     0033D0E0  lq    $v0, 0x0($v1)
     0033D0E4  sq    $v0, 0x0($sp)
     0033D0E8  lui   $at, 0x42C8         ; mtc1 -> f20 = 100.0f (callee-saved, used after
     0033D0F0  jal   func_00283180       ;        both calls, so the constant lives in f20)
     0033D0F4  lwc1  $f12, 0xF8($s0)     <- the object's float passed to both calls
     0033D138  jal   func_L04_0033CC00   ; (a0, a1, v, v + 0x10, 1.0f)

   Measured: with the source pointer taken first (`float *src = (float *)(a0 + 0x10);`) the
   body is 144 bytes - one word short, because cc1 folds the address into `lq $v0,0x10($s0)`
   instead of keeping the `addiu $v1,$s0,0x10` retail has - and the frame comes out 0x40
   against retail's 0x50 (f20 sits right after $ra instead of at 0x40), which moves every
   stack offset: 32 diffs. The element-wise loop form is 196 bytes. */

float func_00283180(char *a, char *b, float f);
float func_00283198(float f);
void func_L04_0033CC00(char *a, char *b, float *v, float *w, float f);

void func_L04_0033D0C0(char *a0, char *a1)
{
    float v[4];
    float w[4];
    float *src = (float *)(a0 + 0x10);

    *(int (*)[4])v = *(int (*)[4])src;
    v[0] += func_00283180(a0, a1, *(float *)(a0 + 0xF8)) * 100.0f;
    v[1] += func_00283198(*(float *)(a0 + 0xF8)) * 100.0f;
    func_L04_0033CC00(a0, a1, v, w, 1.0f);
}
