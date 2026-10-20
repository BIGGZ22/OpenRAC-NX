/* func_00283B98 -- the sum of two angles, wrapped to [-pi, pi], 72 bytes in retail.

   Retail: a0 + a1, then -2*pi when it is not below pi, and +2*pi when it is
   below -pi.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00283B98: 0 of 28 places match", 92 bytes against retail's 72.

   What differs:
   - retail keeps the sum in `$f0` (the return register), pi in `$f14` and -pi
     in `$f15`, and lays the body out as one compare, one `bc1t` over the
     subtract arm, the subtract arm, one `bc1f` over the add arm, the add arm
     and the return; this cc1 keeps the sum in `$f12`, materialises pi in `$f1`
     and -pi in `$f0`, and needs one more branch and a `mov.s` in the return
     delay slot;
   - retail computes the -pi test in the delay slot of the first branch, this
     cc1 emits a separate compare.
*/
typedef float f32;

f32 func_00283B98(f32 a0, f32 a1) {
    f32 r = a0 + a1;
    if (r < 3.1415927f) {
        if (r < -3.1415927f) {
            r = r + 3.1415927f + 3.1415927f;
        }
    } else {
        r = r - 3.1415927f - 3.1415927f;
    }
    return r;
}
