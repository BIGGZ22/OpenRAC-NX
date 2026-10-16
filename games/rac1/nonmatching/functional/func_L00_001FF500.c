/* functional: medium. out.xy = v.xy * (a / |v.xy|); when x*x + y*y is zero, out.xy = 0 (the joined piece func_L00_001FF538). out.zw = v.zw in both cases (the quadword store). VU0's rsqrt and rounding may differ in the last bit. */
void func_L00_001FF500(float *out, float *v, float a) {
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float w = v[3];
    float len2 = x * x + y * y;

    if (len2 == 0.0f) {
        x = 0.0f;
        y = 0.0f;
    } else {
        float s = a / sqrtf(len2);
        x = x * s;
        y = y * s;
    }
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
