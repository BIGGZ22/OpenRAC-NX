/* functional: high. Corrected from the port's hand C (port/game/rac1-pal/hand/game_fastfunc.c, func_001FA5C8): the branch tests the sign bit of the dot sum (bgez on vf6.y), so -0.0 takes the difference branch as retail does, and a zero-length blend returns the zero vector (retail's vrsqrt of 0 saturates and 0 times that is 0) instead of 0 * inf = NaN. out = normalise(a(1-t) + b t), or normalise(a(1-t) - b t) when the dot of those two is negative; t is arg3, a and b are 4-float vectors, out is written to arg0. */
void func_001FA5C8(float *out, const float *a, const float *b, float t) {
    float p[4], q[4], v[4];
    float dot = 0.0f, len2 = 0.0f, inv;
    union { float f; unsigned int u; } sign;
    float s = 1.0f - t;
    int i;
    int neg;

    for (i = 0; i < 4; i++) {
        p[i] = a[i] * s;
        q[i] = b[i] * t;
        dot += p[i] * q[i];
    }
    sign.f = dot;
    neg = (int)(sign.u >> 31);
    for (i = 0; i < 4; i++) {
        v[i] = neg ? p[i] - q[i] : p[i] + q[i];
        len2 += v[i] * v[i];
    }
    if (len2 > 0.0f) {
        inv = 1.0f / __builtin_sqrtf(__builtin_fabsf(len2));
    } else {
        inv = 0.0f;
    }
    for (i = 0; i < 4; i++) {
        out[i] = v[i] * inv;
    }
}
