/* dst = a * b, componentwise on four floats (VU0 vmul.xyzw: load both, multiply, store). */
void func_001F9C60(float *dst, float *a, float *b) {
    float r0 = a[0] * b[0];
    float r1 = a[1] * b[1];
    float r2 = a[2] * b[2];
    float r3 = a[3] * b[3];
    dst[0] = r0;
    dst[1] = r1;
    dst[2] = r2;
    dst[3] = r3;
}
