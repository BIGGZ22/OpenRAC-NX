/* functional: high. fcheck: unchecked. Rotates the four rows of the matrix at src (arg1) about Y by angle (radians) and writes them to dst (arg0; callers pass dst == src): c and s come from func_00125380's sine polynomial (D_00132F00, degree 9) evaluated at pi/2 - angle (pi/2 + angle when angle < 0), c = the polynomial, s = sqrt(|1 - c*c|) negated when angle < 0; each row v becomes v.x*(c,0,-s,0) + v.y*(0,1,0,0) + v.z*(s,0,c,0) + v.w*(0,0,0,1), summed in VU0's ACC order. The VU0 registers retail leaves behind (vf4..vf9, ACC, Q) are not reproduced. Differential test: 74 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 vs FPU rounding; 44 of 74 agree bit for bit). */
extern float sqrtf(float);

static float bits_125548(unsigned int u)
{
    union { unsigned int u; float f; } c;
    c.u = u;
    return c.f;
}

/* func_00125380's sine polynomial: with a = pi/2 -+ angle it gives cos(angle) and |sin(angle)|. */
static void sincos_125548(float ang, float *sn, float *cs)
{
    float a, a2, t, s, q;
    int neg;

    if (ang < 0.0f) {
        a = bits_125548(0x3FC90FDB) + ang;
        neg = 1;
    } else {
        a = bits_125548(0x3FC90FDB) - ang;
        neg = 0;
    }
    a2 = a * a;
    s = a;
    t = bits_125548(0xBE2AAAA4) * a * a2;                  /* D_00132F00.w */
    s = s + t;
    t = bits_125548(0x3C08873E) * a * a2 * a2;             /* .z */
    s = s + t;
    t = bits_125548(0xB94FB21F) * a * a2 * a2 * a2;        /* .y */
    s = s + t;
    t = bits_125548(0x362E9C14) * a * a2 * a2 * a2 * a2;   /* .x */
    s = s + t;
    q = 1.0f - s * s;
    if (q < 0.0f)
        q = -q;                                         /* vsqrt takes |x| */
    q = sqrtf(q);
    *cs = s;
    *sn = neg ? 0.0f - q : q;
}

void func_00125548(float *dst, float *src, float angle)
{
    float r[4][4];
    float sn, cs, x, y, z, w;
    int i, j;

    sincos_125548(angle, &sn, &cs);
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = 0.0f;
    r[0][0] = cs;
    r[0][2] = 0.0f - sn;
    r[1][1] = 1.0f;
    r[2][0] = sn;
    r[2][2] = cs;
    r[3][3] = 1.0f;
    for (i = 0; i < 4; i++) {
        x = src[0];
        y = src[1];
        z = src[2];
        w = src[3];
        for (j = 0; j < 4; j++)
            dst[j] = ((r[0][j] * x + r[1][j] * y) + r[2][j] * z) + r[3][j] * w;
        src += 4;
        dst += 4;
    }
}
