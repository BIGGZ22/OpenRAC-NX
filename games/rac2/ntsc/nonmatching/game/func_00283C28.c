/* func_00283C28 -- an angle wrapped to [-pi, pi], 88 bytes in retail.

   Retail: the fraction of a turn is taken out of the angle and what is left
   is scaled back to radians.

   Measured with tools/check.py in src/game/0027F128.c (the -O2 -G0 profile):
   "func_00283C28: 0 of 28 places match", 80 bytes against retail's 88.

   What differs:
   - retail rounds the scaled angle down with a branchless floor (`cvt.w.s`,
     then the sign of the float shifted into the integer and added), this cc1
     has no floor: a plain `(s32)` cast truncates towards zero and the
     candidate is eight bytes short.  This cc1 has no floor builtin at all
     (`floorf`, `__builtin_floor` and `__builtin_floorf` all become a call to
     an undefined symbol), so the floor cannot be written in plain C here;
   - retail keeps the scaled angle in `$f12` and the two constants in `$f1`
     and `$f2`, this cc1 puts the constants in `$f0` and `$f2` and the floor
     in `$f1`.
*/
typedef float f32;
typedef int s32;

f32 func_00283C28(f32 a0) {
    f32 t = a0 * 0.15915494f + 0.5f;
    f32 fl = (f32)(s32)t;
    return (t - fl - 0.5f) * 6.28318548f;
}
