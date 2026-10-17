/* functional: medium. Returns (distance between a.xyz and b.xyz minus (a.w + b.w)) / 1024, as the VU0 code does (squared sum, sqrt, scale by 2^-10, which is exact); VU0 sqrt and sum rounding and denormal flushing can differ from C float in the last bit. Checked against the assembly: same reads, same arithmetic, same result; only the sqrt is the port's openrac_sqrt (the sum of squares is never negative), so the declaration was added to make it compile. */
extern float openrac_sqrt(float x);

float func_L00_00251468(float *a, float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    float d = openrac_sqrt(dx * dx + dy * dy + dz * dz);
    return (d - (a[3] + b[3])) * 0.0009765625f;
}
