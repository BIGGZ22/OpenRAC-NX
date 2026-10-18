/*
 * FastVecCross(out, a, b): out = b x a, from the xyz of the two vectors (VU0
 * vopmula / vopmsub: ACC = b.yzx * a.zxy, then out = ACC - a.yzx * b.zxy).
 * The VU writes only x, y and z of the result; the sqc2 also stores vf3.w,
 * which is whatever the VU0 register held (left as it is here).
 */
void func_001F9CA0(float *out, float *a, float *b) {
    float acc0 = b[1] * a[2];
    float acc1 = b[2] * a[0];
    float acc2 = b[0] * a[1];
    /* All three before any store, as the VU does: out may be a or b (cross in place). */
    float x = acc0 - a[1] * b[2];
    float y = acc1 - a[2] * b[0];
    float z = acc2 - a[0] * b[1];
    out[0] = x;
    out[1] = y;
    out[2] = z;
}
