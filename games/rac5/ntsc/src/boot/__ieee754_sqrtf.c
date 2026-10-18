/* math/ef_sqrt.c (newlib libm, fdlibm), adapted without headers.
   Source: public Newlib 1.9.0; RAC5 addresses remeasured.
   Provenance 2026-10-17, wave 11 lot r1. */
/* ---- fdlibm.h (newlib), verbatim types and accessors, little-endian EE ---- */
typedef int __int32_t;
typedef unsigned int __uint32_t;

typedef union
{
  double value;
  struct
    {
      __uint32_t lsw;
      __uint32_t msw;
    } parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

#define GET_LOW_WORD(i,d)					\
do {								\
  ieee_double_shape_type gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

#define SET_LOW_WORD(d,v)					\
do {								\
  ieee_double_shape_type sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)

typedef union
{
  float value;
  __uint32_t word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)
/* ---- declarations the body calls (fdlibm.h + math.h) ---- */
extern double fabs(double);
extern float fabsf(float);
extern double scalbn(double, int);
extern float scalbnf(float, int);
extern double floor(double);
extern float floorf(float);
extern double copysign(double, double);
extern float copysignf(float, float);
extern float __ieee754_sqrtf(float);
extern double __kernel_sin(double, double, int);
extern double __kernel_cos(double, double);
extern int __kernel_rem_pio2(double *, double *, int, int, int, const __int32_t *);
extern float __kernel_sinf(float, float, int);
extern float __kernel_cosf(float, float);
extern int __kernel_rem_pio2f(float *, float *, int, int, int, const __int32_t *);
extern __int32_t __ieee754_rem_pio2f(float, float *);
extern __int32_t __ieee754_rem_pio2(double, double *);
extern float __ieee754_expf(float);
extern double __ieee754_sqrt(double);
extern double __ieee754_log(double);
extern float __ieee754_logf(float);
extern float __ieee754_expf(float);
extern float __ieee754_powf(float, float);
extern float __ieee754_fmodf(float, float);
extern float __ieee754_acosf(float);
extern float __ieee754_asinf(float);
extern float __ieee754_atan2f(float, float);
struct exception
{
  int type;
  char *name;
  double arg1;
  double arg2;
  double retval;
  int err;
};
#define DOMAIN 1
#define SING 2
#define OVERFLOW 3
#define UNDERFLOW 4
#define TLOSS 5
#define PLOSS 6
enum __fdlibm_version
{
  __fdlibm_ieee = -1,
  __fdlibm_svid,
  __fdlibm_xopen,
  __fdlibm_posix
};
extern enum __fdlibm_version __fdlib_version;
#define _LIB_VERSION __fdlib_version
#define _IEEE_  __fdlibm_ieee
#define _SVID_  __fdlibm_svid
#define _XOPEN_ __fdlibm_xopen
#define _POSIX_ __fdlibm_posix
extern int matherr(struct exception *);
union __dmath
{
  __uint32_t i[2];
  double d;
};
extern const union __dmath __infinity;
#define HUGE_VAL (__infinity.d)
#define HUGE ((float)3.40282346638528860e+38)
#define ERANGE 34
extern int isnan(double);
extern int isnanf(float);
extern int *__errno(void);
#define errno (*__errno())
#define EDOM 33
/* ---- end shim ---- */
/* ef_sqrtf.c -- float version of e_sqrt.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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

static	const float	one	= 1.0, tiny=1.0e-30;

	float __ieee754_sqrtf(float x)
{
	float z;
	__int32_t 	sign = (__int32_t)0x80000000;
	__uint32_t r;
	__int32_t ix,s,q,m,t,i;

	GET_FLOAT_WORD(ix,x);

    /* take care of Inf and NaN */
	if((ix&0x7f800000L)==0x7f800000L) {
	    return x*x+x;		/* sqrt(NaN)=NaN, sqrt(+inf)=+inf
					   sqrt(-inf)=sNaN */
	}
    /* take care of zero */
	if(ix<=0) {
	    if((ix&(~sign))==0) return x;/* sqrt(+-0) = +-0 */
	    else if(ix<0)
		return (x-x)/(x-x);		/* sqrt(-ve) = sNaN */
	}
    /* normalize x */
	m = (ix>>23);
	if(m==0) {				/* subnormal x */
	    for(i=0;(ix&0x00800000L)==0;i++) ix<<=1;
	    m -= i-1;
	}
	m -= 127;	/* unbias exponent */
	ix = (ix&0x007fffffL)|0x00800000L;
	if(m&1)	/* odd m, double x to make it even */
	    ix += ix;
	m >>= 1;	/* m = [m/2] */

    /* generate sqrt(x) bit by bit */
	ix += ix;
	q = s = 0;		/* q = sqrt(x) */
	r = 0x01000000L;		/* r = moving bit from right to left */

	while(r!=0) {
	    t = s+r;
	    if(t<=ix) {
		s    = t+r;
		ix  -= t;
		q   += r;
	    }
	    ix += ix;
	    r>>=1;
	}

    /* use floating add to find out rounding direction */
	if(ix!=0) {
	    z = one-tiny; /* trigger inexact flag */
	    if (z>=one) {
	        z = one+tiny;
		if (z>one)
		    q += 2;
		else
		    q += (q&1);
	    }
	}
	ix = (q>>1)+0x3f000000L;
	ix += (m <<23);
	SET_FLOAT_WORD(z,ix);
	return z;
}

/* Public upstream: https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/ef_sqrt.c
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
