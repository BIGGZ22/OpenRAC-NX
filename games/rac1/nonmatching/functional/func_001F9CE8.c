/* functional: medium. corrected from nonmatching/game/func_001F9CE8.c: the VU0 flushes a square that is denormal to zero and saturates one that overflows to the largest float, so the sum is never inf and the Newton step never gives NaN (the old version did for |x| or |y| above about 1.8e19); Newton's seed and five steps are kept, and VU0's vsqrt may still differ from it in the last bit. Returns sqrt(x*x + y*y) over x and y only; z and w are not read. */
float func_001F9CE8(float *v)
{
    union { float f; unsigned int i; } u;
    union { float f; unsigned int i; } big;
    union { float f; unsigned int i; } tiny;
    float xx;
    float yy;
    float s;
    float y;
    int k;

    big.i = 0x7F7FFFFFu;
    tiny.i = 0x00800000u;

    xx = v[0] * v[0];
    yy = v[1] * v[1];
    if (xx > big.f) {
        xx = big.f;
    }
    if (yy > big.f) {
        yy = big.f;
    }
    if (xx < tiny.f) {
        xx = 0.0f;
    }
    if (yy < tiny.f) {
        yy = 0.0f;
    }
    s = xx + yy;
    if (s > big.f) {
        s = big.f;
    }
    if (s == 0.0f) {
        return 0.0f;
    }
    u.f = s;
    u.i = 0x1FBD1DF5u + (u.i >> 1);
    y = u.f;
    for (k = 0; k < 5; k++) {
        y = 0.5f * (y + s / y);
    }
    return y;
}
