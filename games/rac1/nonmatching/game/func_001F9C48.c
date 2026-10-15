/* func_001F9C48(s, dst, a): VU0 vmulx.xyzw: dst = a * s on all four lanes
   (s broadcast from the x lane of the scalar). */
void func_001F9C48(float s, float *dst, float *a) {
    float x = a[0] * s;
    float y = a[1] * s;
    float z = a[2] * s;
    float w = a[3] * s;

    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}
