/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). out.xyz = a.xyz + (b.xyz - a.xyz) * t, out.w = a.w (the quadword store writes all four). Each VU0 step is its own statement, as the port's hand C writes it, so that clang on arm64 does not fuse the multiply and add; that changed no result in tools/behave.sh (7 of 300 trials differ both ways, where the sum cancels to near zero, so the cause is the VU0 arithmetic, not the fusing). Not checked against retail beyond that run. */
void func_001F9C08(float *out, float *a, float *b, float t) {
    float w = a[3];
    float x = b[0] - a[0];
    float y = b[1] - a[1];
    float z = b[2] - a[2];
    x = x * t;
    y = y * t;
    z = z * t;
    x = a[0] + x;
    y = a[1] + y;
    z = a[2] + z;
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}
