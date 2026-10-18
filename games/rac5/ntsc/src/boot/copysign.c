/* copysign - magnitude of x with the sign bit of y (fdlibm bit trick). */

typedef union {
    double value;
    struct {
        unsigned int lsw;
        unsigned int msw;
    } word;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i, d)                     \
    do {                                        \
        ieee_double_shape_type gh_u;            \
        gh_u.value = (d);                       \
        (i) = gh_u.word.msw;                    \
    } while (0)

#define SET_HIGH_WORD(d, v)                     \
    do {                                        \
        ieee_double_shape_type sh_u;            \
        sh_u.value = (d);                       \
        sh_u.word.msw = (v);                    \
        (d) = sh_u.value;                       \
    } while (0)

double copysign(double x, double y)
{
    unsigned int hx;
    unsigned int hy;

    GET_HIGH_WORD(hx, x);
    GET_HIGH_WORD(hy, y);
    SET_HIGH_WORD(x, (hx & 0x7fffffff) | (hy & 0x80000000));
    return x;
}
