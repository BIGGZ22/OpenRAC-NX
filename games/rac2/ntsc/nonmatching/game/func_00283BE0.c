/* func_00283BE0 -- the difference of two angles, wrapped to [-pi, pi],
   72 bytes in retail.  It is func_00283B98 with a subtraction.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00283BE0: 0 of 28 places match", 92 bytes against retail's 72.

   What differs: the same as func_00283B98 (register allocation, the block
   layout, the extra branch and the `mov.s` in the return delay slot).
*/
typedef float f32;

f32 func_00283BE0(f32 a0, f32 a1) {
    f32 r = a0 - a1;
    if (r < 3.1415927f) {
        if (r < -3.1415927f) {
            r = r + 3.1415927f + 3.1415927f;
        }
    } else {
        r = r - 3.1415927f - 3.1415927f;
    }
    return r;
}
