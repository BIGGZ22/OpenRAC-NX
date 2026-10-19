/* functional: high. fcheck: unchecked. Sets a vector's length: dst.xyz = src.xyz * (len / |src.xyz|), dst.w = src.w; a zero-length src gives dst.xyz = 0 (VU0 macro code: vrsqrt Q = len / sqrt(x*x + y*y + z*z), guarded by a zero test on the squared length). Differential test: 61 random cases agree with retail in an EE emulator to 1e-6 relative (VU0 rsqrt vs FPU sqrt/div rounding). */
void func_001F9DC0(void *dst, void *src, float len) {
    float *d = (float *)dst;
    const float *s = (const float *)src;
    float x = s[0], y = s[1], z = s[2], w = s[3];
    float sq = x * x + y * y + z * z;
    if (sq == 0.0f) {
        d[0] = 0.0f;
        d[1] = 0.0f;
        d[2] = 0.0f;
    } else {
        float k = len / __builtin_sqrtf(sq);
        d[0] = x * k;
        d[1] = y * k;
        d[2] = z * k;
    }
    d[3] = w;
}
