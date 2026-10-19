/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Retail func_001FA9E8 is handwritten: it calls func_001FA8F0 (a VU0 sign test over the point at a, with the float f and the table D_0018D080), returns that result r, and stores 0 to *dst when r < 0; otherwise it stores min(int(y) >> 7, 128), y being the second value func_001FA8F0 leaves in vf3.y. The test is rebuilt here from its VU code in plain float: the VU sign bits are read as IEEE sign bits, and VU rounding is not reproduced, so a value that is nearly zero can take the other branch. No NaN or infinity is produced. */

extern float D_0018D080[];

static int ftoi_sat(float v)
{
    if (v >= 2147483648.0f) {
        return 0x7FFFFFFF;
    }
    if (v <= -2147483648.0f) {
        return (int)0x80000000u;
    }
    return (int)v;
}

static int fneg(float v)
{
    union {
        float f;
        unsigned int u;
    } t;

    t.f = v;
    return (int)(t.u >> 31);
}

/* func_001FA8F0 as func_001FA9E8 uses it: the return value, and the vf3.y it leaves in *y. */
static int ftest_001FA8F0(float *p, float f, float *y)
{
    const float *T = D_0018D080;
    float s = T[15];
    float ps[4];
    float u[3];
    float X, Y, Z, a, AX, AY;
    float v1x, v1y, v3x, v3y;
    float g_x, g_y;
    float v5x, v5y, v9x, v9y, v6x, v6y;
    float v7x, v7y, v8x, v8y, v4x, v4y;
    float v7ox, v7oy;
    int k;

    for (k = 0; k < 4; k++) {
        ps[k] = p[k] * s;
    }
    a = ps[3];
    for (k = 0; k < 3; k++) {
        u[k] = ps[k] - T[12 + k];
    }

    /* vf2.xyz = M * u, M = columns T0, T1, T2 (rows 0x00, 0x10, 0x20). */
    X = (T[0] * u[0] + T[4] * u[1]) + T[8] * u[2];
    Y = (T[1] * u[0] + T[5] * u[1]) + T[9] * u[2];
    Z = (T[2] * u[0] + T[6] * u[1]) + T[10] * u[2];

    AX = X < 0.0f ? -X : X;
    AY = Y < 0.0f ? -Y : Y;

    v1x = (0.0f + a) + Z;
    v1y = (0.0f - a) + Z;
    v3x = 0.0f - v1x;
    v3y = (f * s) - v1y;

    g_x = T[24] * a;
    g_y = T[25] * a;
    v7ox = g_x * T[26];
    v7oy = g_y * T[26];

    v5x = T[16] * Z;
    v5y = T[17] * Z;
    v9x = v5x * T[20];
    v9y = v5y * T[21];

    v6x = AX - g_x;
    v6y = AY - g_y;

    v8x = AX + v7ox;
    v8y = AY + v7oy;

    v7x = v5x - v6x;
    v7y = v5y - v6y;
    v8x = v9x - v8x;
    v8y = v9y - v8y;

    v4x = v1x - T[28];
    v4y = v1y - T[29];

    *y = v3y;

    if (!fneg(v3x) || fneg(v3y)) {
        return -1;
    }
    if (fneg(v7x) || fneg(v7y)) {
        return -1;
    }
    if (fneg(v8y) || fneg(v4y)) {
        return 0;
    }
    if (fneg(v8x) || !fneg(v4x)) {
        return 0;
    }
    return 1;
}

int func_001FA9E8(void *a, int *dst, float f)
{
    float y;
    int r;
    unsigned int u;

    r = ftest_001FA8F0((float *)a, f, &y);
    if (r < 0) {
        *dst = 0;
        return r;
    }
    u = (unsigned int)ftoi_sat(y) >> 7;
    if (u > 128u) {
        u = 128u;
    }
    *dst = (int)u;
    return r;
}
