/* func_L00_003A2018 - 288 bytes, 27 places, level 00_aranos_tutorial, retail at
   0x003a2018. The body is readable:
     - two early-outs, `if (t == 0.0f) { *out = *v1; return; }` and
       `if (t == 1.0f) { *out = *v2; return; }`, both a 128-bit copy. The copy
       reaches retail's `lq $v0,0($a1)` / `sq $v0,0($a0)` only through a
       `mode(TI)` local - a struct copy gives ldl/ldr/sdl/sdr instead, measured;
     - a cubic in t, componentwise: with A = v1, B = v2, C = v3, D = v4 and
       x = C-A, y = D-B, z = B-C, e = y-x, g = x-e,
       out[i] = e*t*t*t + g*t*t + z*t + A, and out[3] = 0.0f.
   The candidate below is 280 bytes against retail's 288 and its instruction
   SEQUENCE is retail's instruction for instruction: the same two branches, the
   same lq/sq pair, the same load order, the same 48 float operations in the
   same order and the same three stores. Two things are left:
     - this cc1 fills the `jr $ra` delay slot of the two copy branches with the
       `sq` (lq; jr; sq), retail leaves the slot empty (lq; sq; jr; nop): one
       word per branch, two words over the body;
     - the float registers. Retail puts x (C-A) in $f1, B in $f4, tt (t*t) in
       $f5 and A in $f6; this cc1 puts B in $f1, tt in $f4, A in $f5 and x in
       $f6. The candidate also reuses $f0 for y, e*t^3 and the result, as
       retail does.
   Eleven source shapes were tried: the two copy forms (a `mode(TI)` temporary
   and a direct assignment), the statement order of the three differences in
   each of the three components, an explicit load local, the two orders of the
   final sum, the inline and the separate forms of e, g and z*t. None moves the
   float registers. The wall is the float register allocation.
   func_L11_00472090 is the same body (the catalogue gives both the same
   fingerprint). */
typedef int TI128 __attribute__((mode(TI)));

void func_L00_003A2018(TI128 *out, TI128 *a1, TI128 *a2, TI128 *a3, TI128 *a4, float t)
{
    float *o = (float *)out;
    float *p = (float *)a1;
    float *q = (float *)a2;
    float *r = (float *)a3;
    float *s = (float *)a4;
    float tt, ttt, x, y, z, e, g;

    if (t == 0.0f) {
        TI128 tmp = *a1;
        *out = tmp;
        return;
    }
    if (t == 1.0f) {
        TI128 tmp = *a2;
        *out = tmp;
        return;
    }
    tt = t * t;
    ttt = tt * t;
    x = r[0] - p[0];
    y = s[0] - q[0];
    z = q[0] - r[0];
    e = y - x;
    g = x - e;
    o[0] = e * ttt + g * tt + z * t + p[0];
    x = r[1] - p[1];
    y = s[1] - q[1];
    z = q[1] - r[1];
    e = y - x;
    g = x - e;
    o[1] = e * ttt + g * tt + z * t + p[1];
    x = r[2] - p[2];
    y = s[2] - q[2];
    z = q[2] - r[2];
    e = y - x;
    g = x - e;
    o[2] = e * ttt + g * tt + z * t + p[2];
    o[3] = 0.0f;
}
