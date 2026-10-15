/* functional: medium. Blend of two float buffers: dst = a*(1-f) + b*f over 23 iterations of 16 floats (368 floats), as the VU0 loop does per 16-byte vector. VU rounding is not reproduced. */
void func_L01_00262D28(float *dst, float *a, float *b, float f) {
    int i;
    for (i = 0; i < 23 * 16; i++) {
        dst[i] = a[i] * (1.0f - f) + b[i] * f;
    }
}
