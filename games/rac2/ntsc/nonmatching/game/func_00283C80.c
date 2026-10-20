/* func_00283C80 -- the angular distance between two angles, 56 bytes in retail.

   Retail: |a0 - a1|, and 2*pi - that when it is not below pi.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00283C80: 0 of 28 places match", 64 bytes against retail's 56.

   What differs:
   - retail doubles pi at run time (`add.s $f1,$f14,$f14` with pi in `$f14`),
     this cc1 folds `pi + pi` into the constant 2*pi at compile time;
   - retail keeps the difference in `$f0` (the return register) and pi in
     `$f14`, this cc1 keeps the difference in `$f12` and pi in `$f0`, and
     moves the result into `$f0` in the return delay slot;
   - retail has a `nop` after the `mtc1` that materialises pi, this cc1 has
     none there.
*/
typedef float f32;

f32 func_00283C80(f32 a0, f32 a1) {
    f32 d = __builtin_fabsf(a0 - a1);
    if (!(d < 3.1415927f)) {
        d = (3.1415927f + 3.1415927f) - d;
    }
    return d;
}
