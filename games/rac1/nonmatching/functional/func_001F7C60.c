/* functional: high. fcheck: unchecked. FastIntersectVert (hand-written, called by the clipper func_001F856C): dst = the point where edge a->b crosses clip plane `plane` (0..5: +x,-x,+y,-y,+z,-z against w) of two 0x20-byte clip vertices {x y z w, s t, rgba}. t = (+-a[axis] - a.w) / ((b.w - a.w) + (+-a[axis] - +-b[axis])); xyzw = (a + b*t) - a*t, s and t lerped on the FPU, each colour byte = ((int)(16 * ((a + b*t) - a*t)) + 2) >> 4 as VU0 vftoi4 and the halfword pack give it. The word at dst+0x1C is not written. VU0 rounding of the multiply-adds is not reproduced. Differential test: 300 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative. */

typedef union {
    float f;
    unsigned int u;
} FU_1F7C60;

/* VU0 vftoi4: x * 16 truncated toward zero, saturated to 32 bits. */
static int ftoi4_1F7C60(float x)
{
    FU_1F7C60 v;
    int e;
    unsigned int m;
    unsigned int r;

    v.f = x * 16.0f;
    e = (int)((v.u >> 23) & 0xFF) - 127;
    if (e < 0) {
        return 0;
    }
    if (e >= 31) {
        return (v.u >> 31) ? (int)0x80000000u : 0x7FFFFFFF;
    }
    m = (v.u & 0x7FFFFF) | 0x800000;
    r = e >= 23 ? m << (e - 23) : m >> (23 - e);
    return (v.u >> 31) ? -(int)r : (int)r;
}

void func_001F7C60(void *dst, void *a, void *b, int plane)
{
    float *d = (float *)dst;
    float *pa = (float *)a;
    float *pb = (float *)b;
    unsigned char *ca = (unsigned char *)a + 0x18;
    unsigned char *cb = (unsigned char *)b + 0x18;
    unsigned int axis = (unsigned int)plane >> 1;
    unsigned int sign = (unsigned int)plane << 31;
    FU_1F7C60 sa, sb;
    float den, num, t, s0, s1;
    unsigned int col;
    int k;

    sa.f = pa[axis];
    sa.u ^= sign;
    sb.f = pb[axis];
    sb.u ^= sign;
    den = (pb[3] - pa[3]) + (sa.f - sb.f);
    num = sa.f - pa[3];
    t = num / den;

    s0 = (pb[4] - pa[4]) * t;
    s0 = s0 + pa[4];
    s1 = (pb[5] - pa[5]) * t;
    s1 = s1 + pa[5];

    col = 0;
    for (k = 0; k < 4; k++) {
        float fa = (float)ca[k];
        float fb = (float)cb[k];
        float c = (fa + fb * t) - fa * t;
        unsigned int h = ((unsigned int)ftoi4_1F7C60(c) + 2) & 0xFFFF;
        col |= ((h >> 4) & 0xFF) << (8 * k);
    }

    for (k = 0; k < 4; k++) {
        d[k] = (pa[k] + pb[k] * t) - pa[k] * t;
    }
    d[4] = s0;
    d[5] = s1;
    *(unsigned int *)((unsigned char *)dst + 0x18) = col;
}
