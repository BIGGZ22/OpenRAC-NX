/* func_001F9C30(s, dst, a): VU0 vmulx.xyz: dst.xyz = a.xyz * s (s broadcast
   from the x lane of the scalar); the w of a is kept. */
void func_001F9C30(float s, float *dst, float *a) {
    float x = a[0] * s;
    float y = a[1] * s;
    float z = a[2] * s;
    float w = a[3];

    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}
