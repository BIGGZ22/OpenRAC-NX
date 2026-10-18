/* logf -- newlib libm/math/wf_log.c (public Newlib 1.9.0, adapted 2026-10-17).
 * HUGE is FLT_MAX as a double (retail pool 0x01F20DB0 = 0x47EFFFFFE0000000);
 * HUGE_VAL is the retail `__infinity` object at 0x01F21730 (symbols.tsv).
 * Name string + the two HUGE doubles form the object .rdata (sidecar .map). */
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
#define SING 2
#define EDOM 33
#define ERANGE 34

#define HUGE ((double)3.40282347e+38F) /* FLT_MAX, retail 0x47EFFFFFE0000000 */

extern const int __fdlib_version;
extern union { unsigned int i[2]; double d; } __infinity;
#define HUGE_VAL (__infinity.d)
extern int *__errno(void);
#define errno (*__errno())
extern int matherr(struct exception *e);
extern int isnanf(float x);
extern float __ieee754_logf(float x);

float logf(float x) /* wrapper logf */
{
    float z;
    struct exception exc;
    z = __ieee754_logf(x);
    if (__fdlib_version == _IEEE_ || isnanf(x) || x > (float)0.0) return z;
    exc.name = "logf";
    exc.err = 0;
    exc.arg1 = exc.arg2 = (double)x;
    if (__fdlib_version == _SVID_)
        exc.retval = -HUGE;
    else
        exc.retval = -HUGE_VAL;
    if (x == (float)0.0) {
        /* logf(0) */
        exc.type = SING;
        if (__fdlib_version == _POSIX_)
            errno = ERANGE;
        else if (!matherr(&exc)) {
            errno = EDOM;
        }
    } else {
        /* logf(x<0) */
        exc.type = DOMAIN;
        if (__fdlib_version == _POSIX_)
            errno = EDOM;
        else if (!matherr(&exc)) {
            errno = EDOM;
        }
    }
    if (exc.err != 0) errno = exc.err;
    return (float)exc.retval;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_log.c
 * Retain the original SunPro permission notice below. */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
