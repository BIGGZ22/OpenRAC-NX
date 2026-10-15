/* Length of the xy part of the 4-float vector at v (VU0: vmul.xyz, vaddy.x,
   vsqrt, the x lane returned): sqrt(x*x + y*y). z and w are not read. The
   square root is Newton's method from a bit-level seed, since the port has
   no libm call here. */
float func_001F9CE8(float *v)
{
    union { float f; unsigned int i; } u;
    float s;
    float y;
    int k;

    s = v[0] * v[0] + v[1] * v[1];
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
