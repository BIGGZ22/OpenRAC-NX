/* func_L08_002F5CB8 (192 bytes, 3 places, default profile -O2 -G0)
   -- measured 0 of 3: candidate is 188 bytes, one instruction short.

   Call plumbing for a spawn: converts the second integer argument to float, asks
   func_L00_002E9210(p3, p4) for a size, builds the second float as
   (float)(p0 - (size >> 1)), takes func_0027C878(1) as a fourth word argument and
   passes everything to func_0027F640(p2, p3, p4, r, (s32)D_00296CD0, f20, f21,
   fp0). Five integer parameters, so p4 arrives in `$t0`; one float parameter, so
   fp0 arrives in `$f12`.

   THE ABI AND THE ARGUMENT ORDER ARE EXACT (the `$a0..$a3`, `$t0`, `$f12`-`$f14`
   setup of the last call matches word for word). Residues, both register
   allocation:
     - retail saves `$f22 = $f12` and converts into `$f21`; this cc1 has them the
       other way round (`$f21 = $f12`, conversion into `$f22`);
     - retail opens with `daddu $v0,$a1,$zero` and converts `mtc1 $v0,$f21`, one
       instruction more than `mtc1 $a1,...`. Spelling the copy (`s32 n = p1;`),
       saving the float in a local, or inlining the conversion all fold away.

   The `mtc1`/`nop`/`cvt.s.w` hazard pair and both call sites are otherwise exact.

   Wave 7's "nested calls in the arguments of one enclosing call" lever was tried
   here (the whole body as one func_0027F640(...) with func_0027C878(1) and
   func_L00_002E9210(p3, p4) inline): 200 bytes with the size call inlined, 188
   with it kept in a local - worse than or equal to the four-statement form, and
   it does not produce retail's `daddu $v0,$a1,$zero` or the `$f22`/`$f21` order. */

typedef int s32;

extern s32 func_L00_002E9210(s32, s32);
extern s32 func_0027C878(s32);
extern void func_0027F640(s32, s32, s32, s32, s32, float, float, float);
extern char D_00296CD0[];

void func_L08_002F5CB8(s32 p0, s32 p1, s32 p2, s32 p3, s32 p4, float fp0) {
    float f21 = (float)p1;
    s32 x = func_L00_002E9210(p3, p4);
    float f20 = (float)(p0 - (x >> 1));
    s32 r = func_0027C878(1);

    func_0027F640(p2, p3, p4, r, (s32)D_00296CD0, f20, f21, fp0);
}
