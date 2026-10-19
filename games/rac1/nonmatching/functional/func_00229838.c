/* functional: high. fcheck: unchecked. Slides each shadow-volume edge in the scratchpad (D_001604A0 pairs of quadwords from 0x70000000, as func_00229098/func_002291E8 left them) along itself to the two clip depths: with d = p1 - p0 (xyz), p0.xyz += d * (a.z - p0.z) / d.z and p1.xyz += d * (b.z - p1.z) / d.z; w is kept. a and b are the caller's two vectors (only .z is read). VU0 macro code (vdiv Q = 1 / d.z; the vf20 = (1,1,1) it leaves for later code is not reproduced). A zero d.z is not handled specially (VU0 would give +/-FLT_MAX). Differential test: 64 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 vs FPU rounding). */
extern int D_001604A0;

void func_00229838(float *a, float *b)
{
    float *p = (float *)0x70000000;
    float az = a[2];
    float bz = b[2];
    int n = D_001604A0;

    do {
        float dx = p[4] - p[0];
        float dy = p[5] - p[1];
        float dz = p[6] - p[2];
        float q = 1.0f / dz;
        float t0 = az - p[2];
        float t1 = bz - p[6];
        p[0] += dx * t0 * q;
        p[1] += dy * t0 * q;
        p[2] += dz * t0 * q;
        p[4] += dx * t1 * q;
        p[5] += dy * t1 * q;
        p[6] += dz * t1 * q;
        p += 8;
    } while (--n != 0);
}
