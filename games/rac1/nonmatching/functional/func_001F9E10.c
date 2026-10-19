/* functional: high. fcheck: unchecked. 2D form of func_001F9DC0: dst.xy = src.xy * (len / |src.xy|), dst.zw = src.zw; a zero-length src.xy gives dst.xy = 0 (VU0 macro code: vrsqrt Q = len / sqrt(x*x + y*y)). Differential test: 61 random cases agree with retail in an EE emulator to 1e-6 relative. */
void func_001F9E10(float *dst, float *src, float len) {
    float x = src[0], y = src[1], z = src[2], w = src[3];
    float sq = x * x + y * y;
    if (sq == 0.0f) {
        dst[0] = 0.0f;
        dst[1] = 0.0f;
    } else {
        float k = len / __builtin_sqrtf(sq);
        dst[0] = x * k;
        dst[1] = y * k;
    }
    dst[2] = z;
    dst[3] = w;
}
