/* func_L04_0033CB08 (124 bytes, 3 places) -- NEAR MISS, not integrated.
   state: 124 bytes, every retail instruction present, one slot swapped.
     check.py (04_barlow):
     0033CB50  mov.s  $f12, $f20        |  daddu $a2, $s2, $zero
     0033CB54  mul.s  $f13, $f0, $f13   |  mul.s $f13, $f0, $f13
     0033CB58  mov.s  $f14, $f21        |  mov.s $f12, $f20
     0033CB5C  jal    func_002A8240     |  jal   func_002A8240
     0033CB60  daddu  $a2, $s2, $zero   |  mov.s $f14, $f21
   Both compile the same three integer moves and three float moves; cc1 here emits the
   third integer move with the other two, retail leaves it for the call's delay slot (the
   float move takes that slot here). Everything before +0x48 - both prologues, the
   func_00283CE0(a2) call, the 1/1024 scaling - is byte-identical, and the epilogue too.

   Measured: the scaled value held in a local, the third argument cast to `void *`, the
   constant written on the left of the multiply, and the product computed before the
   argument copies - all 124 bytes, same three words. The scaling is a `mul.s` by
   0.0009765625f (= 0x3A800000, which cc1 materialises as `lui $at,0x3A80` +
   `mtc1 $at,$f13`), not a division: `div.s` by 1024 would not match. */

typedef int s32;

extern float func_00283CE0(s32);
extern float func_002A8240(void *, void *, s32, float, float, float);

/* Scales what func_00283CE0(a2) returns by 1/1024 and hands it to func_002A8240 together
   with the two floats the caller passed through. */
float func_L04_0033CB08(void *a0, void *a1, s32 a2, s32 a3, float f12, float f13) {
    return func_002A8240(a0, a1, a3, f12, func_00283CE0(a2) * 0.0009765625f, f13);
}
