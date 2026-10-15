/* out.xyz = a.xyz * (1 / s) (VU0 vdiv Q = vf0.w / s, then vmulq.xyz), out.w = a.w; a zero s is
   the VU's division by zero, the largest float. */
void func_L00_001FF328(float *out, float *a, float s) {
    union { unsigned int u; float f; } q;
    float x, y, z, w;

    if (s != 0.0f) {
        q.f = 1.0f / s;
    } else {
        q.u = (*(unsigned int *)&s & 0x80000000) | 0x7F7FFFFF;
    }
    x = a[0] * q.f;
    y = a[1] * q.f;
    z = a[2] * q.f;
    w = a[3];
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
