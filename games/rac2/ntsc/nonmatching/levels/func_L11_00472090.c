/* func_L11_00472090 - 288 bytes, 3 places, level 11_joba, retail at 0x00472090.
   The same body as func_L00_003A2018 (the catalogue gives both the same
   fingerprint): two early-outs, `if (t == 0.0f) { *out = *v1; return; }` and
   `if (t == 1.0f) { *out = *v2; return; }`, both a 128-bit copy that reaches
   retail's `lq $v0,0($a1)` / `sq $v0,0($a0)` only through a `mode(TI)` local,
   then a cubic in t, componentwise: with A = v1, B = v2, C = v3, D = v4 and
   x = C-A, y = D-B, z = B-C, e = y-x, g = x-e,
   out[i] = e*t*t*t + g*t*t + z*t + A, and out[3] = 0.0f.
   The candidate is 280 bytes against retail's 288 and its instruction sequence
   is retail's instruction for instruction; what is left is the float register
   allocation (retail puts x (C-A) in $f1, this cc1 in $f6) and the two words
   this cc1 spends filling the `jr $ra` delay slot of the copy branches with
   the `sq` where retail leaves it empty. Eleven source shapes were tried.
   See func_L00_003A2018 for the full measurement. */
typedef int TI128 __attribute__((mode(TI)));

void func_L11_00472090(TI128 *out, TI128 *a1, TI128 *a2, TI128 *a3, TI128 *a4, float t)
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
