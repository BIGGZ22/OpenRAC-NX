/* powf -- newlib libm/math/wf_pow.c (SunPro fdlibm, adapted 2026-10-18).
 * Header inlined: struct exception, _LIB_VERSION enum, HUGE/HUGE_VAL.
 * The "powf" name string and the double constants live in the shared .rodata
 * pool (sidecar .map anchors the section at the retail address).  */

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
    int err;
};

#define _IEEE_ (-1)
#define _SVID_ 0
#define _XOPEN_ 1
#define _POSIX_ 2
#define DOMAIN 1
#define OVERFLOW 3
#define UNDERFLOW 4
#define EDOM 33
#define ERANGE 34

#define HUGE ((float)3.40282346638528860e+38)
extern const union { double d; } __infinity;
#define HUGE_VAL (__infinity.d)

extern const int __fdlib_version;
extern int *__errno(void);
#define errno (*__errno())
extern int matherr(struct exception *e);
extern int isnanf(float x);
extern int finitef(float x);
extern double rint(double x);
extern float __ieee754_powf(float x, float y);

float powf(float x, float y) /* wrapper powf */
{
    float z;
    struct exception exc;
    z = __ieee754_powf(x, y);
    if (__fdlib_version == _IEEE_ || isnanf(y)) return z;
    if (isnanf(x)) {
        if (y == (float)0.0) {
            /* powf(NaN,0.0) */
            /* error only if _LIB_VERSION == _SVID_ & _XOPEN_ */
            exc.type = DOMAIN;
            exc.name = "powf";
            exc.err = 0;
            exc.arg1 = (double)x;
            exc.arg2 = (double)y;
            exc.retval = x;
            if (__fdlib_version == _IEEE_ ||
                __fdlib_version == _POSIX_) exc.retval = 1.0;
            else if (!matherr(&exc)) {
                errno = EDOM;
            }
            if (exc.err != 0)
                errno = exc.err;
            return (float)exc.retval;
        } else
            return z;
    }
    if (x == (float)0.0) {
        if (y == (float)0.0) {
            /* powf(0.0,0.0) */
            /* error only if _LIB_VERSION == _SVID_ */
            exc.type = DOMAIN;
            exc.name = "powf";
            exc.err = 0;
            exc.arg1 = (double)x;
            exc.arg2 = (double)y;
            exc.retval = 0.0;
            if (__fdlib_version != _SVID_) exc.retval = 1.0;
            else if (!matherr(&exc)) {
                errno = EDOM;
            }
            if (exc.err != 0)
                errno = exc.err;
            return (float)exc.retval;
        }
        if (finitef(y) && y < (float)0.0) {
            /* 0**neg */
            exc.type = DOMAIN;
            exc.name = "powf";
            exc.err = 0;
            exc.arg1 = (double)x;
            exc.arg2 = (double)y;
            if (__fdlib_version == _SVID_)
                exc.retval = 0.0;
            else
                exc.retval = -HUGE_VAL;
            if (__fdlib_version == _POSIX_)
                errno = EDOM;
            else if (!matherr(&exc)) {
                errno = EDOM;
            }
            if (exc.err != 0)
                errno = exc.err;
            return (float)exc.retval;
        }
        return z;
    }
    if (!finitef(z)) {
        if (finitef(x) && finitef(y)) {
            if (isnanf(z)) {
                /* neg**non-integral */
                exc.type = DOMAIN;
                exc.name = "powf";
                exc.err = 0;
                exc.arg1 = (double)x;
                exc.arg2 = (double)y;
                if (__fdlib_version == _SVID_)
                    exc.retval = 0.0;
                else
                    exc.retval = 0.0 / 0.0;	/* X/Open allow NaN */
                if (__fdlib_version == _POSIX_)
                    errno = EDOM;
                else if (!matherr(&exc)) {
                    errno = EDOM;
                }
                if (exc.err != 0)
                    errno = exc.err;
                return (float)exc.retval;
            } else {
                /* powf(x,y) overflow */
                exc.type = OVERFLOW;
                exc.name = "powf";
                exc.err = 0;
                exc.arg1 = (double)x;
                exc.arg2 = (double)y;
                if (__fdlib_version == _SVID_) {
                    exc.retval = HUGE;
                    y *= 0.5;
                    if (x < 0.0 && rint(y) != y) exc.retval = -HUGE;
                } else {
                    exc.retval = HUGE_VAL;
                    y *= 0.5;
                    if (x < 0.0 && rint(y) != y) exc.retval = -HUGE_VAL;
                }
                if (__fdlib_version == _POSIX_)
                    errno = ERANGE;
                else if (!matherr(&exc)) {
                    errno = ERANGE;
                }
                if (exc.err != 0)
                    errno = exc.err;
                return (float)exc.retval;
            }
        }
    }
    if (z == (float)0.0 && finitef(x) && finitef(y)) {
        /* powf(x,y) underflow */
        exc.type = UNDERFLOW;
        exc.name = "powf";
        exc.err = 0;
        exc.arg1 = (double)x;
        exc.arg2 = (double)y;
        exc.retval = 0.0;
        if (__fdlib_version == _POSIX_)
            errno = ERANGE;
        else if (!matherr(&exc)) {
            errno = ERANGE;
        }
        if (exc.err != 0)
            errno = exc.err;
        return (float)exc.retval;
    }
    return z;
}
