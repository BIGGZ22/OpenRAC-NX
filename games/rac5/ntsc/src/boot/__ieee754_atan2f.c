/* ef_atan2.c -- float version of e_atan2.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Source: newlib/libm/math/ef_atan2.c (RAC2 sdk-compiler-build).
 * `-zero` is the one constant the compiler keeps in memory; it is declared
 * extern and resolved through the .map sidecar at its retail .rodata address.
 */

typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union {
    float value;
    __uint32_t word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i, d)                                                   \
    do {                                                                       \
        ieee_float_shape_type gf_u;                                            \
        gf_u.value = (d);                                                      \
        (i) = gf_u.word;                                                       \
    } while (0)

#define SET_FLOAT_WORD(d, i)                                                   \
    do {                                                                       \
        ieee_float_shape_type sf_u;                                            \
        sf_u.word = (i);                                                       \
        (d) = sf_u.value;                                                      \
    } while (0)

extern float atanf(float);
extern float fabsf(float);

extern const float zero_neg; /* -zero, retail .rodata 0x01F21070 */

static const float tiny = 1.0e-30, zero = 0.0,
                   pi_o_4 = 7.8539818525e-01, /* 0x3f490fdb */
    pi_o_2 = 1.5707963705e+00,                /* 0x3fc90fdb */
    pi = 3.1415925026e+00,                    /* 0x40490fda */
    pi_lo = 1.5099578832e-07;                 /* 0x34222168 */

float __ieee754_atan2f(float y, float x)
{
    float z;
    __int32_t k, m, hx, hy, ix, iy;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    GET_FLOAT_WORD(hy, y);
    iy = hy & 0x7fffffff;
    if ((ix > 0x7f800000) || (iy > 0x7f800000)) /* x or y is NaN */
        return x + y;
    if (hx == 0x3f800000) return atanf(y); /* x=1.0 */
    m = ((hy >> 31) & 1) | ((hx >> 30) & 2); /* 2*sign(x)+sign(y) */

    /* when y = 0 */
    if (iy == 0) {
        switch (m) {
            case 0:
            case 1: return y;           /* atan(+-0,+anything)=+-0 */
            case 2: return pi + tiny;   /* atan(+0,-anything) = pi */
            case 3: return -pi - tiny;  /* atan(-0,-anything) =-pi */
        }
    }
    /* when x = 0 */
    if (ix == 0) return (hy < 0) ? -pi_o_2 - tiny : pi_o_2 + tiny;

    /* when x is INF */
    if (ix == 0x7f800000) {
        if (iy == 0x7f800000) {
            switch (m) {
                case 0: return pi_o_4 + tiny;          /* atan(+INF,+INF) */
                case 1: return -pi_o_4 - tiny;         /* atan(-INF,+INF) */
                case 2: return (float)3.0 * pi_o_4 + tiny;  /*atan(+INF,-INF)*/
                case 3: return (float)-3.0 * pi_o_4 - tiny; /*atan(-INF,-INF)*/
            }
        } else {
            switch (m) {
                case 0: return zero;      /* atan(+...,+INF) */
                case 1: return zero_neg;  /* atan(-...,+INF) */
                case 2: return pi + tiny; /* atan(+...,-INF) */
                case 3: return -pi - tiny; /* atan(-...,-INF) */
            }
        }
    }
    /* when y is INF */
    if (iy == 0x7f800000) return (hy < 0) ? -pi_o_2 - tiny : pi_o_2 + tiny;

    /* compute y/x */
    k = (iy - ix) >> 23;
    if (k > 60)
        z = pi_o_2 + (float)0.5 * pi_lo; /* |y/x| >  2**60 */
    else if (hx < 0 && k < -60)
        z = 0.0;                         /* |y|/x < -2**60 */
    else
        z = atanf(fabsf(y / x));         /* safe to do y/x */
    switch (m) {
        case 0: return z; /* atan(+,+) */
        case 1: {
            __uint32_t zh;
            GET_FLOAT_WORD(zh, z);
            SET_FLOAT_WORD(z, zh ^ 0x80000000);
        }
            return z; /* atan(-,+) */
        case 2: return pi - (z - pi_lo); /* atan(+,-) */
        default: /* case 3 */
            return (z - pi_lo) - pi; /* atan(-,-) */
    }
}
