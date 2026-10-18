/* fmodf -- newlib libm/math/wf_fmod.c (public Newlib 1.9.0, adapted 2026-10-17).
 * Name string and quiet NaN form the object .rdata (sidecar .map). */
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
#define EDOM 33

extern const int __fdlib_version;
extern int *__errno(void);
#define errno (*__errno())
extern int matherr(struct exception *e);
extern int isnanf(float x);
extern float __ieee754_fmodf(float x, float y);

float fmodf(float x, float y) /* wrapper fmodf */
{
    float z;
    struct exception exc;
    z = __ieee754_fmodf(x, y);
    if (__fdlib_version == _IEEE_ || isnanf(y) || isnanf(x)) return z;
    if (y == (float)0.0) {
        /* fmodf(x,0) */
        exc.type = DOMAIN;
        exc.name = "fmodf";
        exc.err = 0;
        exc.arg1 = (double)x;
        exc.arg2 = (double)y;
        if (__fdlib_version == _SVID_)
            exc.retval = x;
        else
            exc.retval = 0.0 / 0.0;
        if (__fdlib_version == _POSIX_)
            errno = EDOM;
        else if (!matherr(&exc)) {
            errno = EDOM;
        }
        if (exc.err != 0) errno = exc.err;
        return (float)exc.retval;
    } else
        return z;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_fmod.c
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
