/* sqrt -- newlib libm/math/w_sqrt.c (public Newlib 1.9.0, adapted 2026-10-17).
 *
 * The retail wrapper inlines the exception block (it does not call
 * __kernel_standard): the name string and the quiet NaN it returns live in the
 * shared .rodata pool, so this body needs the `.rodata` link directive in its
 * sidecar (see sqrt.map).
 *
 * `struct exception` and the _LIB_VERSION macros follow public newlib
 * libc/include/math.h; _LIB_VERSION is the retail `__fdlib_version` symbol. */
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
extern int isnan(double x);
extern double __ieee754_sqrt(double x);

double sqrt(double x) /* wrapper sqrt */
{
    struct exception exc;
    double z;
    z = __ieee754_sqrt(x);
    if (__fdlib_version == _IEEE_ || isnan(x)) return z;
    if (x < 0.0) {
        exc.type = DOMAIN;
        exc.name = "sqrt";
        exc.err = 0;
        exc.arg1 = exc.arg2 = x;
        if (__fdlib_version == _SVID_)
            exc.retval = 0.0;
        else
            exc.retval = 0.0 / 0.0;
        if (__fdlib_version == _POSIX_)
            errno = EDOM;
        else if (!matherr(&exc)) {
            errno = EDOM;
        }
        if (exc.err != 0) errno = exc.err;
        return exc.retval;
    } else
        return z;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/w_sqrt.c
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
