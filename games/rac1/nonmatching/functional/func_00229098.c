/* functional: high. fcheck: unchecked. Shadow record type 0 (a sphere; ParseTypedRecord, 0x20 bytes): builds the shadow-volume ring in the scratchpad. p+4 = n (points), p+0x10 = centre xyz and radius w. The start vector D_00160480.xyz * radius is rotated n times by 6.28/n about -D_00160470 (the light direction; Rodrigues matrix from FastSin/FastCos, the VU0 micro programs at 0xC90/0xC80 retail calls with vcallms), and each rotated point centre + v goes to 0x70000000 + 0x20*i, with that point + D_00160490 (the extrusion offset) at +0x10. The first pair is repeated after the last and D_001604A0 = n + 1. Retail leaves the w word of every point as stale VU0 vf5.w (the next consumers, func_00229838 and func_00228D20, never read it before func_00228D20 overwrites it); this writes 0, which is what the emulator's reset VU0 state gives. n = 0 is not guarded (retail would loop 2^32 times). Differential test: 64 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 vs FPU rounding). */
extern float D_00160470[4];
extern float D_00160480[4];
extern float D_00160490[4];
extern int D_001604A0;
extern float func_001F9FA8(float); /* FastSin */
extern float func_001F9F90(float); /* FastCos */

/* Rows r[0..2] of the rotation by the angle with sine s, cosine c about axis a, in VU0's op order. */
static void rot_00229098(float r[3][3], const float *a, float s, float c)
{
    float k = 1.0f - c;
    float tx = a[0] * s, ty = a[1] * s, tz = a[2] * s;

    r[0][0] = a[0] * a[0] * k + c;
    r[0][1] = a[1] * a[0] * k - tz;
    r[0][2] = a[2] * a[0] * k + ty;
    r[1][0] = a[0] * a[1] * k + tz;
    r[1][1] = a[1] * a[1] * k + c;
    r[1][2] = a[2] * a[1] * k - tx;
    r[2][0] = a[0] * a[2] * k - ty;
    r[2][1] = a[1] * a[2] * k + tx;
    r[2][2] = a[2] * a[2] * k + c;
}

void func_00229098(char *p)
{
    int n = *(int *)(p + 4);
    float *c = (float *)(p + 0x10);
    float *out = (float *)0x70000000;
    float *first = out;
    float axis[3];
    float r[3][3];
    float v[3];
    float ang, s, co;
    int i, k;

    axis[0] = -D_00160470[0];
    axis[1] = -D_00160470[1];
    axis[2] = -D_00160470[2];
    ang = 6.28f / (float)n;
    s = func_001F9FA8(ang);
    co = func_001F9F90(ang);
    rot_00229098(r, axis, s, co);
    v[0] = D_00160480[0] * c[3];
    v[1] = D_00160480[1] * c[3];
    v[2] = D_00160480[2] * c[3];
    i = n;
    do {
        float x = v[0], y = v[1], z = v[2];
        for (k = 0; k < 3; k++) {
            v[k] = r[0][k] * x + r[1][k] * y + r[2][k] * z;
        }
        for (k = 0; k < 3; k++) {
            out[k] = v[k] + c[k];
            out[4 + k] = out[k] + D_00160490[k];
        }
        out[3] = 0.0f;
        out[7] = 0.0f;
        out += 8;
    } while (--i != 0);
    for (k = 0; k < 8; k++) {
        out[k] = first[k];
    }
    D_001604A0 = n + 1;
}
