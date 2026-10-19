/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). out.xy = v.xy * (a / |v.xy|) as VU0 computes it with vrsqrt. Edited in place: a sum of squares below the smallest normal float is treated as zero (the VU flushes denormals, so retail takes the zero branch, func_L00_001FF538: out.xy = 0), and the scale is clamped to +-FLT_MAX because vrsqrt saturates and a/sqrt could overflow to inf (inf times a zero component gives NaN on the host). out.zw = v.zw in both cases (the quadword store). VU0's rsqrt rounding may differ in the last bit. */
void func_L00_001FF500(float *out, float *v, float a) {
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float w = v[3];
    float len2 = x * x + y * y;

    if (len2 < 1.17549435e-38f) {
        x = 0.0f;
        y = 0.0f;
    } else {
        float s = a / __builtin_sqrtf(len2);
        if (s > 3.40282347e+38f) {
            s = 3.40282347e+38f;
        } else if (s < -3.40282347e+38f) {
            s = -3.40282347e+38f;
        }
        x = x * s;
        y = y * s;
    }
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
