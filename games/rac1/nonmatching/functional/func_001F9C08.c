/* functional: medium. out.xyz = a.xyz + (b.xyz - a.xyz) * t, out.w = a.w (the quadword store writes all four). tools/behave.sh: 76 of 3,000 trials differ, all where the sum cancels to near zero (VU0 adds, the FPU here may fuse the multiply-add). */
void func_001F9C08(float *out, float *a, float *b, float t) {
    float w = a[3];
    float x = a[0] + (b[0] - a[0]) * t;
    float y = a[1] + (b[1] - a[1]) * t;
    float z = a[2] + (b[2] - a[2]) * t;
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
