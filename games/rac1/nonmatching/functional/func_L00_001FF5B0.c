/* functional: high. Clamps the xy length of the vector at src to maxlen and writes the result to dst (z and w copied unchanged). Returns 1 when it scaled, 0 when the length is zero or already below maxlen (dst untouched, as the retail 0 path). Retail is VU0: vsqrt of x*x + y*y, then vdiv maxlen / len and a scale of xy. Host: the quotient is clamped to +-FLT_MAX where VU0 saturates instead of giving inf; the squared length is never negative, so sqrt never sees a NaN. */
#define PS2_MAX_F 0x1.fffffep127f

int func_L00_001FF5B0(float maxlen, float *dst, float *src)
{
    float x = src[0];
    float y = src[1];
    float len = __builtin_sqrtf(x * x + y * y);
    float q;

    if (len == 0.0f) {
        return 0;
    }
    if (maxlen - len > 0.0f) {
        return 0;
    }
    q = maxlen / len;
    if (q > PS2_MAX_F) {
        q = PS2_MAX_F;
    }
    if (q < -PS2_MAX_F) {
        q = -PS2_MAX_F;
    }
    dst[0] = x * q;
    dst[1] = y * q;
    dst[2] = src[2];
    dst[3] = src[3];
    return 1;
}
