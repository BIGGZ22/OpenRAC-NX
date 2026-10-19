/* ef_acos.c -- math/ef_acos.c from public Newlib 1.9.0 (adapted 2026-10-17).
 * Original header kept below; fdlibm.h inlined after it. */
/* w11/r3 -- adapted from the authentic newlib source named above.
 * fdlibm.h is inlined (typedefs, word macros, exception struct, version enum);
 * the newlib source text itself is unchanged. Verified with the sdk-divpad
 * cc1 + ee-as-sdk -G0 on RAC5 boot SCUS_976.15.
 */

typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union {
    double value;
    struct {
        __uint32_t lsw;
        __uint32_t msw;
    } parts;
} ieee_double_shape_type;

typedef union {
    float value;
    __uint32_t word;
} ieee_float_shape_type;

#define EXTRACT_WORDS(ix0, ix1, d)                                             \
    do {                                                                       \
        ieee_double_shape_type ew_u;                                           \
        ew_u.value = (d);                                                      \
        (ix0) = ew_u.parts.msw;                                                \
        (ix1) = ew_u.parts.lsw;                                                \
    } while (0)

#define GET_HIGH_WORD(i, d)                                                    \
    do {                                                                       \
        ieee_double_shape_type gh_u;                                           \
        gh_u.value = (d);                                                      \
        (i) = gh_u.parts.msw;                                                  \
    } while (0)

#define GET_LOW_WORD(i, d)                                                     \
    do {                                                                       \
        ieee_double_shape_type gl_u;                                           \
        gl_u.value = (d);                                                      \
        (i) = gl_u.parts.lsw;                                                  \
    } while (0)

#define INSERT_WORDS(d, ix0, ix1)                                              \
    do {                                                                       \
        ieee_double_shape_type iw_u;                                           \
        iw_u.parts.msw = (ix0);                                                \
        iw_u.parts.lsw = (ix1);                                                \
        (d) = iw_u.value;                                                      \
    } while (0)

#define SET_HIGH_WORD(d, v)                                                    \
    do {                                                                       \
        ieee_double_shape_type sh_u;                                           \
        sh_u.value = (d);                                                      \
        sh_u.parts.msw = (v);                                                  \
        (d) = sh_u.value;                                                      \
    } while (0)

#define SET_LOW_WORD(d, v)                                                     \
    do {                                                                       \
        ieee_double_shape_type sl_u;                                           \
        sl_u.value = (d);                                                      \
        sl_u.parts.lsw = (v);                                                  \
        (d) = sl_u.value;                                                      \
    } while (0)

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

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
    int err;
};

enum __fdlibm_version {
    __fdlibm_ieee = -1,
    __fdlibm_svid,
    __fdlibm_xopen,
    __fdlibm_posix
};

extern enum __fdlibm_version __fdlib_version;
#define _LIB_VERSION __fdlib_version
#define _IEEE_ __fdlibm_ieee
#define _SVID_ __fdlibm_svid
#define _XOPEN_ __fdlibm_xopen
#define _POSIX_ __fdlibm_posix

#define DOMAIN 1
#define SING 2
#define OVERFLOW 3
#define UNDERFLOW 4
#define TLOSS 5
#define PLOSS 6

#define HUGE ((float)3.40282346638528860e+38)

extern double __ieee754_sqrt(double);
extern double __ieee754_acos(double);
extern double __ieee754_asin(double);
extern double __ieee754_atan2(double, double);
extern double __ieee754_exp(double);
extern double __ieee754_fmod(double, double);
extern double __ieee754_pow(double, double);
extern double __ieee754_log(double);
extern __int32_t __ieee754_rem_pio2(double, double *);
extern double __kernel_sin(double, double, int);
extern double __kernel_cos(double, double);
extern double __kernel_tan(double, double, int);
extern int __kernel_rem_pio2(double *, double *, int, int, int, const __int32_t *);
extern double __kernel_standard(double, double, int);

extern float __ieee754_sqrtf(float);
extern float __ieee754_acosf(float);
extern float __ieee754_asinf(float);
extern float __ieee754_atan2f(float, float);
extern float __ieee754_expf(float);
extern float __ieee754_fmodf(float, float);
extern float __ieee754_powf(float, float);
extern float __ieee754_logf(float);
extern __int32_t __ieee754_rem_pio2f(float, float *);
extern float __kernel_sinf(float, float, int);
extern float __kernel_cosf(float, float);
extern float __kernel_tanf(float, float, int);
extern int __kernel_rem_pio2f(float *, float *, int, int, int, const __int32_t *);

extern int isnan(double);
extern int isnanf(float);
extern double fabs(double);
extern float fabsf(float);
extern double scalbn(double, int);
extern float scalbnf(float, int);
extern int matherr(struct exception *);
extern int *__errno(void);
#define errno (*__errno())
#define EDOM 33
#define ERANGE 34

/* ---- authentic newlib source follows ---- */
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


#ifdef __STDC__
static const float
#else
static float
#endif
one =  1.0000000000e+00, /* 0x3F800000 */
pi =  3.1415925026e+00, /* 0x40490fda */
pio2_hi =  1.5707962513e+00, /* 0x3fc90fda */
pio2_lo =  7.5497894159e-08, /* 0x33a22168 */
pS0 =  1.6666667163e-01, /* 0x3e2aaaab */
pS1 = -3.2556581497e-01, /* 0xbea6b090 */
pS2 =  2.0121252537e-01, /* 0x3e4e0aa8 */
pS3 = -4.0055535734e-02, /* 0xbd241146 */
pS4 =  7.9153501429e-04, /* 0x3a4f7f04 */
pS5 =  3.4793309169e-05, /* 0x3811ef08 */
qS1 = -2.4033949375e+00, /* 0xc019d139 */
qS2 =  2.0209457874e+00, /* 0x4001572d */
qS3 = -6.8828397989e-01, /* 0xbf303361 */
qS4 =  7.7038154006e-02; /* 0x3d9dc62e */

#ifdef __STDC__
	float __ieee754_acosf(float x)
#else
	float __ieee754_acosf(x)
	float x;
#endif
{
	float z,p,q,r,w,s,c,df;
	__int32_t hx,ix;
	GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix==0x3f800000) {		/* |x|==1 */
	    if(hx>0) return 0.0;	/* acos(1) = 0  */
	    else return pi+(float)2.0*pio2_lo;	/* acos(-1)= pi */
	} else if(ix>0x3f800000) {	/* |x| >= 1 */
	    return (x-x)/(x-x);		/* acos(|x|>1) is NaN */
	}
	if(ix<0x3f000000) {	/* |x| < 0.5 */
	    if(ix<=0x23000000) return pio2_hi+pio2_lo;/*if|x|<2**-57*/
	    z = x*x;
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    r = p/q;
	    return pio2_hi - (x - (pio2_lo-x*r));
	} else  if (hx<0) {		/* x < -0.5 */
	    z = (one+x)*(float)0.5;
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    s = __ieee754_sqrtf(z);
	    r = p/q;
	    w = r*s-pio2_lo;
	    return pi - (float)2.0*(s+w);
	} else {			/* x > 0.5 */
	    __int32_t idf;
	    z = (one-x)*(float)0.5;
	    s = __ieee754_sqrtf(z);
	    df = s;
	    GET_FLOAT_WORD(idf,df);
	    SET_FLOAT_WORD(df,idf&0xfffff000);
	    c  = (z-df*df)/(s+df);
	    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
	    q = one+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
	    r = p/q;
	    w = r*s+c;
	    return (float)2.0*(df+w);
	}
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/ef_acos.c
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
