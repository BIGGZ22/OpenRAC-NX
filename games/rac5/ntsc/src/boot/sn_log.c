/* sn_log -- newlib libm/math/e_log.c (fdlibm), compiled soft-float.
 * Provenance: newlib-2000-02-17 e_log.c; compiler SN ProDG 3.01
 * ee-gcc2953 (GCC 2.95.3) + Ps2EeAs 2.0, -O2 -G0.
 * Retail returns +0 for x<0 and puts -two54/zero in the shared pool at
 * 0x01F27338 (empty slot below neg_inf), reached through a distinct alias. */
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

static const double ln2_hi = 6.93147180369123816490e-01;
static const double ln2_lo = 1.90821492927058770002e-10;
static const double two54 = 1.80143985094819840000e+16;
static const double Lg1 = 6.666666666666735130e-01;
static const double Lg2 = 3.999999999940941908e-01;
static const double Lg3 = 2.857142874366239149e-01;
static const double Lg4 = 2.222219843214978396e-01;
static const double Lg5 = 1.818357216161805012e-01;
static const double Lg6 = 1.531383769920937332e-01;
static const double Lg7 = 1.479819860511658591e-01;
static const double zero = 0.0;

extern const double pool_neg_two54; /* -two54/zero, pool 0x01F27338 */

double sn_log(double x)
{
    double hfsq, f, s, z, R, w, t1, t2, dk;
    __int32_t k, hx, i, j;
    __uint32_t lx;

    EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if (hx < 0x00100000) {
        if (((hx & 0x7fffffff) | lx) == 0)
            return pool_neg_two54;
        if (hx < 0)
            return 0;
        k -= 54;
        x *= two54;
        GET_HIGH_WORD(hx, x);
    }
    if (hx >= 0x7ff00000)
        return x + x;
    k += (hx >> 20) - 1023;
    hx &= 0x000fffff;
    i = (hx + 0x95f64) & 0x100000;
    SET_HIGH_WORD(x, hx | (i ^ 0x3ff00000));
    k += (i >> 20);
    f = x - 1.0;
    if ((0x000fffff & (2 + hx)) < 3) {
        if (f == zero)
            if (k == 0)
                return zero;
            else {
                dk = (double)k;
                return dk * ln2_hi + dk * ln2_lo;
            }
        R = f * f * (0.5 - 0.33333333333333333 * f);
        if (k == 0)
            return f - R;
        else {
            dk = (double)k;
            return dk * ln2_hi - ((R - dk * ln2_lo) - f);
        }
    }
    s = f / (2.0 + f);
    dk = (double)k;
    z = s * s;
    i = hx - 0x6147a;
    w = z * z;
    j = 0x6b851 - hx;
    t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
    t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
    i |= j;
    R = t2 + t1;
    if (i > 0) {
        hfsq = f * (0.5 * f);
        if (k == 0)
            return f - (hfsq - s * (hfsq + R));
        else
            return dk * ln2_hi - ((hfsq - (s * (hfsq + R) + dk * ln2_lo)) - f);
    } else {
        if (k == 0)
            return f - s * (f - R);
        else
            return dk * ln2_hi - ((s * (f - R) - dk * ln2_lo) - f);
    }
}
