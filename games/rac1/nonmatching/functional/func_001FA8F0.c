/* functional: high. fcheck: unchecked. Retail func_001FA8F0 is hand-written VU0 code: a screen test of the point p (4 floats, scaled by D_0018D080[15]) against the view table D_0018D080 (rows 0..2 a 3x3 matrix, row 3 the eye position and scale, rows 4..7 the bounds); f is a height. Returns -1 when the point is behind the eye or out of the view cone, 0 when it is inside but near an edge, 1 when it is well inside. Retail also leaves vf3.y (f * scale - (Z - w)) for its hand-written callers (func_001FA9E8, func_001F91B8 do not use it from here: func_001FA9E8 recomputes it). The VU sign bits are read as IEEE sign bits; VU rounding is not reproduced, so a value within rounding of zero can take the other branch. Differential test: 400 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */

extern float D_0018D080[];

static int fneg_1FA8F0(float v)
{
    union {
        float f;
        unsigned int u;
    } t;

    t.f = v;
    return (int)(t.u >> 31);
}

int func_001FA8F0(float *p, float f)
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

    /* vf2.xyz = u.x * row0 + u.y * row1 + u.z * row2 */
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

    if (fneg_1FA8F0(v3y) || !fneg_1FA8F0(v3x)) {
        return -1;
    }
    if (fneg_1FA8F0(v7y) || fneg_1FA8F0(v7x)) {
        return -1;
    }
    /* sign of v8 | sign of (v4 with x negated): y lane first, then x lane */
    if (fneg_1FA8F0(v8y) || fneg_1FA8F0(v4y)) {
        return 0;
    }
    if (fneg_1FA8F0(v8x) || !fneg_1FA8F0(v4x)) {
        return 0;
    }
    return 1;
}
