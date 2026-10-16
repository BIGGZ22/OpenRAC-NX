/* VU0 4x4 matrix times vector: out = M * v, with M given as four columns of
   four floats (m + 0, +4, +8, +12). Reads the vector and all of M first (so
   out may be v), then sums c0*x + c1*y + c2*z + c3*w in that order. */
void func_001F9EE8(float *out, float *v, float *m)
{
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float w = v[3];
    float r0, r1, r2, r3;

    r0 = ((m[0] * x + m[4] * y) + m[8] * z) + m[12] * w;
    r1 = ((m[1] * x + m[5] * y) + m[9] * z) + m[13] * w;
    r2 = ((m[2] * x + m[6] * y) + m[10] * z) + m[14] * w;
    r3 = ((m[3] * x + m[7] * y) + m[11] * z) + m[15] * w;
    out[0] = r0;
    out[1] = r1;
    out[2] = r2;
    out[3] = r3;
}
