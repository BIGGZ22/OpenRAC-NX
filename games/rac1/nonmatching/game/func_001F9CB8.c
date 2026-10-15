/* Length of the vector at a0 (floats x, y, z): sqrt(x*x + y*y + z*z).
   The retail routine does it on VU0 (vmul, vadda, vmadda, vsqrt); the
   sums are taken in the same order. The square root is done inline by
   Newton steps from a bit-pattern guess, since the EE has no sqrt on
   the FPU and the linked code has no libm. */
float func_001F9CB8(void *v) {
    float *f = v;
    float x = f[0];
    float y = f[1];
    float z = f[2];
    float s = x * x + y * y;
    float r;
    union {
        float f;
        unsigned int i;
    } u;
    s = s + z * z;
    if (s <= 0.0f) return 0.0f;
    u.f = s;
    u.i = 0x1FBD1DF5 + (u.i >> 1);
    r = u.f;
    r = 0.5f * (r + s / r);
    r = 0.5f * (r + s / r);
    r = 0.5f * (r + s / r);
    r = 0.5f * (r + s / r);
    return r;
}
