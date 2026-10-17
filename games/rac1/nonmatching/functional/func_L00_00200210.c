/* functional: medium. Plain C for the VU0 sequence: q = a / b (vdiv), truncate q toward zero (vftoi0 then vitof0), take the fraction (vsub), multiply by b (vmul). VU rounding is not reproduced. Corrected from the original for the host: vdiv by zero and overflow saturate to +-FLT_MAX (sign = sign(a) xor sign(b)), vftoi0 saturates to 0x7FFFFFFF / 0x80000000 instead of overflowing the (int) cast, and the product saturates too; no inf or NaN is produced. */
#define PS2_MAX_F 0x1.fffffep127f

static float ps2_saturate(float v) {
    if (v > PS2_MAX_F) {
        return PS2_MAX_F;
    }
    if (v < -PS2_MAX_F) {
        return -PS2_MAX_F;
    }
    return v;
}

float func_L00_00200210(float a, float b) {
    float q;
    float t;
    float r;
    if (b == 0.0f) {
        int neg = (a < 0.0f) != (b < 0.0f);
        q = neg ? -PS2_MAX_F : PS2_MAX_F;
    } else {
        q = ps2_saturate(a / b);
    }
    if (q >= 2147483648.0f) {
        t = 2147483648.0f;
    } else if (q <= -2147483648.0f) {
        t = -2147483648.0f;
    } else {
        t = (float)(int)q;
    }
    r = (q - t) * b;
    return ps2_saturate(r);
}
