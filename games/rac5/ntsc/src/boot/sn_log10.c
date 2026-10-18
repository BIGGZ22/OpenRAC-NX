/* sn_log10 -- newlib libm/math/e_log10.c (fdlibm), compiled soft-float.
 * Provenance: newlib-2000-02-17 e_log10.c; compiler SN ProDG 3.01
 * ee-gcc2953 (GCC 2.95.3) + Ps2EeAs 2.0, -O2 -G0 -msoft-float.
 * Retail replaced the NaN return `(x-x)/zero` by +0 and folded
 * `-two54/zero` into the shared -inf pool constant at 0x01F27358. */
typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union {
    double value;
    struct {
        __uint32_t lsw;
        __uint32_t msw;
    } parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0, ix1, d)                     \
    do {                                               \
        ieee_double_shape_type ew_u;                   \
        ew_u.value = (d);                              \
        (ix0) = ew_u.parts.msw;                        \
        (ix1) = ew_u.parts.lsw;                        \
    } while (0)

#define GET_HIGH_WORD(i, d)                            \
    do {                                               \
        ieee_double_shape_type gh_u;                   \
        gh_u.value = (d);                              \
        (i) = gh_u.parts.msw;                          \
    } while (0)

#define SET_HIGH_WORD(d, v)                            \
    do {                                               \
        ieee_double_shape_type sh_u;                   \
        sh_u.value = (d);                              \
        sh_u.parts.msw = (v);                          \
        (d) = sh_u.value;                              \
    } while (0)

static const double two54 = 1.80143985094819840000e+16;
static const double ivln10 = 4.34294481903251816668e-01;
static const double log10_2hi = 3.01029995663611771306e-01;
static const double log10_2lo = 3.69423907715893078616e-13;

extern const double neg_inf; /* -two54/zero, pool 0x01F27358 */
extern double sn_log(double);

double sn_log10(double x)
{
    double y, z;
    __int32_t i, k, hx;
    __uint32_t lx;

    EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if (hx < 0x00100000) {
        if (((hx & 0x7fffffff) | lx) == 0)
            return neg_inf;
        if (hx < 0)
            return 0;
        k -= 54;
        x *= two54;
        GET_HIGH_WORD(hx, x);
    }
    if (hx >= 0x7ff00000)
        return x + x;
    k += (hx >> 20) - 1023;
    i = ((__uint32_t)k & 0x80000000) >> 31;
    hx = (hx & 0x000fffff) | ((0x3ff - i) << 20);
    y = (double)(k + i);
    SET_HIGH_WORD(x, hx);
    z = y * log10_2lo + ivln10 * sn_log(x);
    return z + y * log10_2hi;
}
