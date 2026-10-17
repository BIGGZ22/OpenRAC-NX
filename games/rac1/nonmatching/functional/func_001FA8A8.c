/* functional: high. Corrected from nonmatching/game/func_001FA8A8.c: its float-to-int step was a plain C conversion, which is undefined out of range; the VU vftoi0 saturates (to 0x7FFFFFFF, or 0x80000000 below), so this does too. Otherwise the same: each byte of a and b is mixed as a*(1-t) + b*t in float, truncated, and packed back low byte first. */
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

int func_001FA8A8(int a, int b, float t)
{
    unsigned int out = 0;
    int c;
    float ac;
    float bc;
    float p;
    float v;
    float w;
    int iv;

    w = 1.0f - t;
    for (c = 0; c < 4; c++) {
        ac = (float)((a >> (8 * c)) & 0xFF);
        bc = (float)((b >> (8 * c)) & 0xFF);
        p = ac * w;
        v = bc * t + p;
        iv = ftoi_sat(v);
        out |= ((unsigned int)iv & 0xFF) << (8 * c);
    }
    return (int)out;
}
