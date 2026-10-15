/* functional: medium. Returns the unclamped t = dot(b-a, c-a) / dot(b-a, b-a) and writes dst = a + clamp(t, 0, 1) * (b-a) for xyz, leaving dst[3] equal to a[3] as retail does; retail divides on the VU0, so the last bits can differ. */

float func_001F7E98(float *dst, float *a, float *b, float *c) {
    float v4x = b[0] - a[0];
    float v4y = b[1] - a[1];
    float v4z = b[2] - a[2];
    float v5x = c[0] - a[0];
    float v5y = c[1] - a[1];
    float v5z = c[2] - a[2];
    float num = v4x * v5x + v4y * v5y + v4z * v5z;
    float den = v4x * v4x + v4y * v4y + v4z * v4z;
    float t = num / den;
    float tc = t;
    if (tc > 1.0f) {
        tc = 1.0f;
    }
    if (tc < 0.0f) {
        tc = 0.0f;
    }
    dst[0] = a[0] + v4x * tc;
    dst[1] = a[1] + v4y * tc;
    dst[2] = a[2] + v4z * tc;
    return t;
}
