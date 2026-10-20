/* 0 of 2 places match - last line: 04_barlow: 120 bytes, expected 132
   Candidate is twelve bytes short (three instructions).  Runs the object's 0x38C counter
   through a scale/damp pair keyed by the two float arguments and adds the same product to
   the record's 0xF0 and 0xF4.  Differences: retail keeps the second pointer in $s0 and the
   first in $s1 (this cc1 the other way round), retail preserves the third float in $f21
   and the product in $f20 (this cc1 keeps only one float live, so it saves and restores one
   pair less), and retail multiplies twice (a rematerialised `mul.s` for each field) where
   this cc1 multiplies once.  The prologue/epilogue therefore differ by one saved pair plus
   the second multiply.  Tried: the product in a named local (that is the form below). */

typedef int s32;
typedef unsigned char u8;
typedef float f32;

extern f32 func_0030FE40(f32 x, f32 y);
extern f32 func_0030F3A0(f32 x);
extern f32 func_0030F3B8(f32 x);

void func_L04_00412E38(u8 *a0, u8 *a1, f32 f12, f32 f13) {
    f32 k;

    *(f32 *)(a1 + 0x38C) = func_0030F3A0(func_0030FE40(*(f32 *)(a1 + 0x38C), f12));
    k = func_0030F3B8(*(f32 *)(a1 + 0x38C));
    *(f32 *)(a0 + 0xF0) = *(f32 *)(a0 + 0xF0) + k * f13;
    *(f32 *)(a0 + 0xF4) = *(f32 *)(a0 + 0xF4) + k * f13;
}
