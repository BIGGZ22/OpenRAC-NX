/* 0 of 27 places match - last line: 00_aranos_tutorial: 228 bytes, expected 196
   Everything but the clamp is recovered and byte-identical in shape: the two
   0x00282CF0 differences (buf1 = a2-a1, buf2 = a3-a1), the two 0x00282DB0 dot
   products, `t = dot(buf1,buf2) / dot(buf1,buf1)` (retail's div.s sits directly in
   the second call's delay-slot successor, no nop), 0x00282D30(buf1,buf1,t),
   0x00282CC0(dst,a1,buf1) and `return t` in $f0.
   The 32 bytes are the clamp: retail has exactly two instructions,
     max.s $f20,$f20,$f0 (f0 = 0.0f) then min.s $f20,$f20,$f1 (f1 = 1.0f),
   i.e. COP1 fmt=S funct 0x28/0x29. This cc1 does not reach them from C (the same wall
   as nonmatching/levels/func_L08_00425328.c): `if (t < 0) t = 0; if (t > 1) t = 1;`
   gives c.olt.s + bc1tl + mov.s (228), the two ternaries and the FMAX/FMIN macros give
   the same shape (236), and `<=`/`>=` give 228. Five clamp spellings measured. */

typedef float f32;

extern void func_00282CF0(f32 *, const f32 *, const f32 *);
extern f32 func_00282DB0(const f32 *, const f32 *);
extern void func_00282D30(f32 *, const f32 *, f32);
extern void func_00282CC0(f32 *, const f32 *, const f32 *);

f32 func_L00_0031BDC0(const f32 *dst, const f32 *a1, const f32 *a2, const f32 *a3) {
    f32 buf1[4];
    f32 buf2[4];
    f32 t;

    func_00282CF0(buf1, a2, a1);
    func_00282CF0(buf2, a3, a1);
    t = func_00282DB0(buf1, buf2) / func_00282DB0(buf1, buf1);
    if (t < 0.0f)
        t = 0.0f;
    if (t > 1.0f)
        t = 1.0f;
    func_00282D30(buf1, buf1, t);
    func_00282CC0(dst, a1, buf1);
    return t;
}
