#include "common.h"
#include "structs.h"

/*
 * core_text object 0x112468-0x1138A8. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib (the SDK's libc.a): closer.o (_close_r) and dtoa.o (quorem,
 * _dtoa_r), back to back. Built with Sony's 2.9-ee (Makefile.sn,
 * EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);

extern int D_0015ED10;

int func_00112468(int *errOut, int arg1) {
    int r;
    D_0015ED10 = 0;
    r = func_00119100(arg1);
    if (r == -1 && D_0015ED10 != 0) {
        *errOut = D_0015ED10;
    }
    return r;
}

/* newlib dtoa.c quorem(b,S): one digit of b/S for dtoa's digit-generation
 * loop -- estimate the digit q from the top limbs, subtract q*S from b
 * (trimming b's leading zero limbs), then bump q by one more if b is
 * still >= S (the estimate can undershoot by 1), subtracting S again. */
extern int func_00115CE8(void *, void *); /* cmp */

typedef struct Bigint_1154D0 {
    struct Bigint_1154D0 *next;
    int k, maxwds, sign, wds;
    unsigned int x[1];
} Bigint_1154D0;

#define STOREINC(xc, hi, lo) \
    (((unsigned short *)(xc))[1] = (unsigned short)(hi), \
     ((unsigned short *)(xc))[0] = (unsigned short)(lo), \
     (xc)++)

int func_001124C0(Bigint_1154D0 *b, Bigint_1154D0 *S) {
    int n;
    int borrow, y, z;
    unsigned int carry, q, ys, si, zs;
    unsigned int *bx, *bxe, *sx, *sxe;

    n = S->wds;
    if (b->wds < n)
        return 0;
    sx = S->x;
    sxe = sx + --n;
    bx = b->x;
    bxe = bx + n;
    q = *bxe / (*sxe + 1);
    if (q) {
        borrow = 0;
        carry = 0;
        do {
            si = *sx++;
            ys = (si & 0xffff) * q + carry;
            zs = (si >> 16) * q + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            STOREINC(bx, z, y);
        } while (sx <= sxe);
        if (!*bxe) {
            bx = b->x;
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    if (func_00115CE8(b, S) >= 0) {
        q++;
        borrow = 0;
        carry = 0;
        bx = b->x;
        sx = S->x;
        do {
            si = *sx++;
            ys = (si & 0xffff) + carry;
            zs = (si >> 16) + (ys >> 16);
            carry = zs >> 16;
            y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
            borrow = y >> 16;
            z = (*bx >> 16) - (zs & 0xffff) + borrow;
            borrow = z >> 16;
            STOREINC(bx, z, y);
        } while (sx <= sxe);
        bx = b->x;
        bxe = bx + n;
        if (!*bxe) {
            while (--bxe > bx && !*bxe)
                --n;
            b->wds = n;
        }
    }
    return q;
}

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/
typedef struct _Bigint {
    struct _Bigint *_next;
    s32 _k, _maxwds, _sign, _wds;
    u32 _x[1];
} _Bigint;
struct _reent {
    u8 prefix[0x40];
    _Bigint *_result;
    s32 _result_k;
};
typedef s32 __Long;
typedef u32 __ULong;
union double_union { f64 d; u32 i[2]; };
extern const f64 D_001524C8[23];
extern const f64 D_00152590[5];
extern const f64 D_001525B0[];
extern char D_00152410[];
extern char D_00152420[];
extern char D_00152428[];
extern _Bigint *func_001154D0(struct _reent *, s32);
extern void func_00115578(struct _reent *, _Bigint *);
extern _Bigint *func_00115EE8(struct _reent *, f64, s32 *, s32 *);
extern s32 func_001156C0(u32);
extern _Bigint *func_00115808(struct _reent *, s32);
extern _Bigint *func_00115840(struct _reent *, _Bigint *, _Bigint *);
extern _Bigint *func_00115A70(struct _reent *, _Bigint *, s32);
extern _Bigint *func_00115B70(struct _reent *, _Bigint *, s32);
extern s32 func_00115CE8_126D8(_Bigint *, _Bigint *) __asm__("func_00115CE8");
extern _Bigint *func_00115D50(struct _reent *, _Bigint *, _Bigint *);
extern _Bigint *func_001155A8(struct _reent *, _Bigint *, s32, s32);
extern s32 func_001124C0_126D8(_Bigint *, _Bigint *) __asm__("func_001124C0");
extern void *func_00115248(void *, const void *, u32);

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string. Inspired by "How to Print Floating-Point Numbers Accurately" by Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101]. Modifications: 1. Rather than iterating, we use a simple numeric overestimate to determine k = floor(log10(d)). We scale relevant quantities using O(log2(k)) rather than O(k) multiplications. 2. For some modes > 2 (corresponding to ecvt and fcvt), we don't try to generate digits strictly left to right. Instead, we compute with fewer bits and propagate the carry if necessary when rounding the final digit up. This is often faster. 3. Under the assumption that input will be rounded nearest, mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22. That is, we allow equality in stopping tests when the round-nearest rule will give the same floating-point value as would satisfaction of the stopping test with strict inequality. 4. We remove common factors of powers of 2 from relevant quantities. 5. When converting floating-point integers less than 1e16, we use floating-point arithmetic rather than resorting to multiple-precision integers. 6. When asked to produce fewer than 15 digits, we first try to get by with floating-point arithmetic; we resort to multiple-precision integer arithmetic only if we cannot guarantee that the floating-point calculation has given the correctly rounded result. For k requested digits and "uniformly" distributed input, the probability is something like 10^(k-15) that we must resort to the long calculation.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/sdk/library/_dtoa_r.c, _dtoa_r. */
char *func_001126D8(struct _reent *ptr, f64 _d, s32 mode, s32 ndigits, s32 *decpt, s32 *sign, char **rve)
{
  /*	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4-9 should give the same return values as 2-3, i.e.,
			4 <= mode <= 9 ==> same return as mode
			2 + (mode & 1).  These modes are mainly for
			debugging; often they run slower but sometimes
			faster than modes 2-3.
		4,5,8,9 ==> left-to-right digit generation.
		6-9 ==> don't try fast floating-point estimate
			(if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

  s32 bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1, j, j1, k, k0,
    k_check, leftright, m2, m5, s2, s5, spec_case, try_quick;
  union double_union d, d2, eps;
  __Long L;

  s32 denorm;
  __ULong x;

  _Bigint *b, *b1, *delta, *mlo, *mhi, *S;
  f64 ds;
  char *s, *s0;

  d.d = _d;

  if (ptr->_result)
    {
      ptr->_result->_k = ptr->_result_k;
      ptr->_result->_maxwds = 1 << ptr->_result_k;
      func_00115578 (ptr, ptr->_result);
      ptr->_result = 0;
    }

  if (((d).i[1]) & 0x80000000U)
    {
      /* set sign for everything, including 0's and NaNs */
      *sign = 1;
      ((d).i[1]) &= ~0x80000000U; /* clear sign bit */
    }
  else
    *sign = 0;

  if ((((d).i[1]) & 0x7ff00000U) == 0x7ff00000U)

    {
      /* Infinity or NaN */
      *decpt = 9999;
      s =

 !((d).i[0]) && !(((d).i[1]) & 0xfffff) ? D_00152410 :

 D_00152420;
      if (rve)
 *rve =

   s[3] ? s + 8 :

   s + 3;
      return s;
    }

  if (!d.d)
    {
      *decpt = 1;
      s = D_00152428;
      if (rve)
 *rve = s + 1;
      return s;
    }

  b = func_00115EE8 (ptr, d.d, &be, &bbits);

  if ((i = (s32) (((d).i[1]) >> 20 & (0x7ff00000U >> 20))))
    {

      d2.d = d.d;
      ((d2).i[1]) &= 0xfffffU;
      ((d2).i[1]) |= 0x3ff00000U;

      /* log(x)	~=~ log(1.5) + (x-1.5)/1.5
		 * log10(x)	 =  log(x) / log(10)
		 *		~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
		 * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
		 *
		 * This suggests computing an approximation k to log10(d) by
		 *
		 * k = (i - Bias)*0.301029995663981
		 *	+ ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
		 *
		 * We want k to be too large rather than too small.
		 * The error in the first-order Taylor series approximation
		 * is in our favor, so we just round up the constant enough
		 * to compensate for any error in the multiplication of
		 * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
		 * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
		 * adding 1e-13 to the constant term more than suffices.
		 * Hence we adjust the constant term to 0.1760912590558.
		 * (We could get a more accurate k by invoking log10,
		 *  but this is probably not worthwhile.)
		 */

      i -= 1023;

      denorm = 0;
    }
  else
    {
      /* d is denormalized */

      i = bbits + be + (1023 + (53 - 1) - 1);
      x = i > 32 ? ((d).i[1]) << 64 - i | ((d).i[0]) >> i - 32
 : ((d).i[0]) << 32 - i;
      d2.d = x;
      ((d2).i[1]) -= 31 * 0x100000U; /* adjust exponent */
      i -= (1023 + (53 - 1) - 1) + 1;
      denorm = 1;
    }

  ds = (d2.d - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
  k = (s32) ds;
  if (ds < 0. && ds != k)
    k--; /* want k = floor(ds) */
  k_check = 1;
  if (k >= 0 && k <= 22)
    {
      if (d.d < D_001524C8[k])
 k--;
      k_check = 0;
    }
  j = bbits - i - 1;
  if (j >= 0)
    {
      b2 = 0;
      s2 = j;
    }
  else
    {
      b2 = -j;
      s2 = 0;
    }
  if (k >= 0)
    {
      b5 = 0;
      s5 = k;
      s2 += k;
    }
  else
    {
      b2 -= k;
      b5 = -k;
      s5 = 0;
    }
  if (mode < 0 || mode > 9)
    mode = 0;
  try_quick = 1;
  if (mode > 5)
    {
      mode -= 4;
      try_quick = 0;
    }
  leftright = 1;
  switch (mode)
    {
    case 0:
    case 1:
      ilim = ilim1 = -1;
      i = 18;
      ndigits = 0;
      break;
    case 2:
      leftright = 0;
      /* no break */
    case 4:
      if (ndigits <= 0)
 ndigits = 1;
      ilim = ilim1 = i = ndigits;
      break;
    case 3:
      leftright = 0;
      /* no break */
    case 5:
      i = ndigits + k + 1;
      ilim = i;
      ilim1 = i - 1;
      if (i <= 0)
 i = 1;
    }
  j = sizeof (__ULong);
  for (ptr->_result_k = 0; sizeof (_Bigint) - sizeof (__ULong) + j <= i;
       j <<= 1)
    ptr->_result_k++;
  ptr->_result = func_001154D0 (ptr, ptr->_result_k);
  s = s0 = (char *) ptr->_result;

  if (ilim >= 0 && ilim <= 14 && try_quick)
    {
      /* Try to get by with floating-point arithmetic. */

      i = 0;
      d2.d = d.d;
      k0 = k;
      ilim0 = ilim;
      ieps = 2; /* conservative */
      if (k > 0)
 {
   ds = D_001524C8[k & 0xf];
   j = k >> 4;
   if (j & 0x10)
     {
       /* prevent overflows */
       j &= 0x10 - 1;
       d.d /= D_001525B0[0];
       ieps++;
     }
   for (; j; j >>= 1, i++)
     if (j & 1)
       {
  ieps++;
  ds *= D_00152590[i];
       }
   d.d /= ds;
 }
      else if (j1 = -k)
 {
   d.d *= D_001524C8[j1 & 0xf];
   for (j = j1 >> 4; j; j >>= 1, i++)
     if (j & 1)
       {
  ieps++;
  d.d *= D_00152590[i];
       }
 }
      if (k_check && d.d < 1. && ilim > 0)
 {
   if (ilim1 <= 0)
     goto fast_failed;
   ilim = ilim1;
   k--;
   d.d *= 10.;
   ieps++;
 }
      eps.d = ieps * d.d + 7.;
      ((eps).i[1]) -= (53 - 1) * 0x100000U;
      if (ilim == 0)
 {
   S = mhi = 0;
   d.d -= 5.;
   if (d.d > eps.d)
     goto one_digit;
   if (d.d < -eps.d)
     goto no_digits;
   goto fast_failed;
 }

      if (leftright)
 {
   /* Use Steele & White method of only
	   * generating digits needed.
	   */
   eps.d = 0.5 / D_001524C8[ilim - 1] - eps.d;
   for (i = 0;;)
     {
       L = d.d;
       d.d -= L;
       *s++ = '0' + (s32) L;
       if (d.d < eps.d)
  goto ret1;
       if (1. - d.d < eps.d)
  goto bump_up;
       if (++i >= ilim)
  break;
       eps.d *= 10.;
       d.d *= 10.;
     }
 }
      else
 {

   /* Generate ilim digits, then fix them up. */
   eps.d *= D_001524C8[ilim - 1];
   for (i = 1;; i++, d.d *= 10.)
     {
       L = d.d;
       d.d -= L;
       *s++ = '0' + (s32) L;
       if (i == ilim)
  {
    if (d.d > 0.5 + eps.d)
      goto bump_up;
    else if (d.d < 0.5 - eps.d)
      {
        while (*--s == '0');
        s++;
        goto ret1;
      }
    break;
  }
     }

 }

    fast_failed:
      s = s0;
      d.d = d2.d;
      k = k0;
      ilim = ilim0;
    }

  /* Do we have a "small" integer? */

  if (be >= 0 && k <= 14)
    {
      /* Yes. */
      ds = D_001524C8[k];
      if (ndigits < 0 && ilim <= 0)
 {
   S = mhi = 0;
   if (ilim < 0 || d.d <= 5 * ds)
     goto no_digits;
   goto one_digit;
 }
      for (i = 1;; i++)
 {
   L = d.d / ds;
   d.d -= L * ds;
# 505 "/Users/flavy/Projects/OpenRAC/games/rac1/ntsc/src/sdk/library/_dtoa_r.c"
   *s++ = '0' + (s32) L;
   if (i == ilim)
     {
       d.d += d.d;
       if (d.d > ds || d.d == ds && L & 1)
  {
  bump_up:
    while (*--s == '9')
      if (s == s0)
        {
   k++;
   *s = '0';
   break;
        }
    ++*s++;
  }
       break;
     }
   if (!(d.d *= 10.))
     break;
 }
      goto ret1;
    }

  m2 = b2;
  m5 = b5;
  mhi = mlo = 0;
  if (leftright)
    {
      if (mode < 2)
 {
   i =

     denorm ? be + (1023 + (53 - 1) - 1 + 1) :

     1 + 53 - bbits;

 }
      else
 {
   j = ilim - 1;
   if (m5 >= j)
     m5 -= j;
   else
     {
       s5 += j -= m5;
       b5 += j;
       m5 = 0;
     }
   if ((i = ilim) < 0)
     {
       m2 -= i;
       i = 0;
     }
 }
      b2 += i;
      s2 += i;
      mhi = func_00115808 (ptr, 1);
    }
  if (m2 > 0 && s2 > 0)
    {
      i = m2 < s2 ? m2 : s2;
      b2 -= i;
      m2 -= i;
      s2 -= i;
    }
  if (b5 > 0)
    {
      if (leftright)
 {
   if (m5 > 0)
     {
       mhi = func_00115A70 (ptr, mhi, m5);
       b1 = func_00115840 (ptr, mhi, b);
       func_00115578 (ptr, b);
       b = b1;
     }
   if (j = b5 - m5)
     b = func_00115A70 (ptr, b, j);
 }
      else
 b = func_00115A70 (ptr, b, b5);
    }
  S = func_00115808 (ptr, 1);
  if (s5 > 0)
    S = func_00115A70 (ptr, S, s5);

  /* Check for special case that d is a normalized power of 2. */

  if (mode < 2)
    {
      if (!((d).i[0]) && !(((d).i[1]) & 0xfffffU)

   && ((d).i[1]) & 0x7ff00000U

 )
 {
   /* The special case */
   b2 += 1;
   s2 += 1;
   spec_case = 1;
 }
      else
 spec_case = 0;
    }

  /* Arrange for convenient computation of quotients:
   * shift left if necessary so divisor has 4 leading 0 bits.
   *
   * Perhaps we should just compute leading 28 bits of S once
   * and for all and pass them and a shift to quorem, so it
   * can do shifts and ors to compute the numerator for q.
   */

  if (i = ((s5 ? 32 - func_001156C0 (S->_x[S->_wds - 1]) : 1) + s2) & 0x1f)
    i = 32 - i;

  if (i > 4)
    {
      i -= 4;
      b2 += i;
      m2 += i;
      s2 += i;
    }
  else if (i < 4)
    {
      i += 28;
      b2 += i;
      m2 += i;
      s2 += i;
    }
  if (b2 > 0)
    b = func_00115B70 (ptr, b, b2);
  if (s2 > 0)
    S = func_00115B70 (ptr, S, s2);
  if (k_check)
    {
      if (func_00115CE8_126D8 (b, S) < 0)
 {
   k--;
   b = func_001155A8 (ptr, b, 10, 0); /* we botched the k estimate */
   if (leftright)
     mhi = func_001155A8 (ptr, mhi, 10, 0);
   ilim = ilim1;
 }
    }
  if (ilim <= 0 && mode > 2)
    {
      if (ilim < 0 || func_00115CE8_126D8 (b, S = func_001155A8 (ptr, S, 5, 0)) <= 0)
 {
   /* no digits, fcvt style */
 no_digits:
   k = -1 - ndigits;
   goto ret;
 }
    one_digit:
      *s++ = '1';
      k++;
      goto ret;
    }
  if (leftright)
    {
      if (m2 > 0)
 mhi = func_00115B70 (ptr, mhi, m2);

      /* Compute mlo -- check for special case
       * that d is a normalized power of 2.
       */

      mlo = mhi;
      if (spec_case)
 {
   mhi = func_001154D0 (ptr, mhi->_k);
   func_00115248(&(mhi)->_sign, &(mlo)->_sign, (mlo)->_wds * sizeof(u32) + 2*sizeof(s32));
   mhi = func_00115B70 (ptr, mhi, 1);
 }

      for (i = 1;; i++)
 {
   dig = func_001124C0_126D8 (b, S) + '0';
   /* Do we yet have the shortest decimal string
	   * that will round to d?
	   */
   j = func_00115CE8_126D8 (b, mlo);
   delta = func_00115D50 (ptr, S, mhi);
   j1 = delta->_sign ? 1 : func_00115CE8_126D8 (b, delta);
   func_00115578 (ptr, delta);

   if (j1 == 0 && !mode && !(((d).i[0]) & 1))
     {
       if (dig == '9')
  goto round_9_up;
       if (j > 0)
  dig++;
       *s++ = dig;
       goto ret;
     }

   if (j < 0 || j == 0 && !mode

       && !(((d).i[0]) & 1)

     )
     {
       if (j1 > 0)
  {
    b = func_00115B70 (ptr, b, 1);
    j1 = func_00115CE8_126D8 (b, S);
    if ((j1 > 0 || j1 == 0 && dig & 1)
        && ++dig == '9' + 1)
      goto round_9_up;
  }
       *s++ = dig;
       goto ret;
     }
   if (j1 > 0)
     {
       if (dig == '9')
  { /* possible if i == 1 */
  round_9_up:
    *s++ = '9';
    goto roundoff;
  }
       *s++ = dig + 1;
       goto ret;
     }
   *s++ = dig;
   if (i == ilim)
     break;
   b = func_001155A8 (ptr, b, 10, 0);
   if (mlo == mhi)
     mlo = mhi = func_001155A8 (ptr, mhi, 10, 0);
   else
     {
       mlo = func_001155A8 (ptr, mlo, 10, 0);
       mhi = func_001155A8 (ptr, mhi, 10, 0);
     }
 }
    }
  else
    for (i = 1;; i++)
      {
 *s++ = dig = func_001124C0_126D8 (b, S) + '0';
 if (i >= ilim)
   break;
 b = func_001155A8 (ptr, b, 10, 0);
      }

  /* Round off last digit */

  b = func_00115B70 (ptr, b, 1);
  j = func_00115CE8_126D8 (b, S);
  if (j > 0 || j == 0 && dig & 1)
    {
    roundoff:
      while (*--s == '9')
 if (s == s0)
   {
     k++;
     *s++ = '1';
     goto ret;
   }
      ++*s++;
    }
  else
    {
      while (*--s == '0');
      s++;
    }
ret:
  func_00115578 (ptr, S);
  if (mhi)
    {
      if (mlo && mlo != mhi)
 func_00115578 (ptr, mlo);
      func_00115578 (ptr, mhi);
    }
ret1:
  func_00115578 (ptr, b);
  *s = 0;
  *decpt = k + 1;
  if (rve)
    *rve = s;
  return s0;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_001138A4);
