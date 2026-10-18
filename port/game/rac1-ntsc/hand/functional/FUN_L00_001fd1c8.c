/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001F7E98.c (games/rac1): func_001F7E98, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: high (fcheck SAME on 3 real calls). corrected from nonmatching/functional/func_001F7E98.c (in place): retail's sqc2 stores all four lanes so dst[3] = a[3], which the C never wrote; added that store. Also guarded den == 0 to +/-FLT_MAX (VU0 vdiv saturates, not NaN) so the host never produces NaN. Retail divides on the VU0, so the last bits can differ. */

float FUN_L00_001fd1c8(float *dst, float *a, float *b, float *c) {
    float v4x = b[0] - a[0];
    float v4y = b[1] - a[1];
    float v4z = b[2] - a[2];
    float v5x = c[0] - a[0];
    float v5y = c[1] - a[1];
    float v5z = c[2] - a[2];
    float num = v4x * v5x + v4y * v5y + v4z * v5z;
    float den = v4x * v4x + v4y * v4y + v4z * v4z;
    float t;
    float tc;
    if (den == 0.0f) {
        int big = 0x7F7FFFFF;
        t = *(float *)&big;
        if (num < 0.0f) {
            t = -t;
        }
    } else {
        t = num / den;
    }
    tc = t;
    if (tc > 1.0f) {
        tc = 1.0f;
    }
    if (tc < 0.0f) {
        tc = 0.0f;
    }
    dst[0] = a[0] + v4x * tc;
    dst[1] = a[1] + v4y * tc;
    dst[2] = a[2] + v4z * tc;
    dst[3] = a[3];
    return t;
}
