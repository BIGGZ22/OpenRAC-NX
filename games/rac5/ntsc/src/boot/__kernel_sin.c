/* newlib libm/math/k_sin.c (fdlibm, double), adapted without headers.

   ECART CONNU — 447/468, premiere difference a 0x01EA1E34.
   Le corps est structurellement identique au retail : les 14 mots qui restent
   sont exactement les 7 chargements de constantes, que le retail prend dans son
   propre pool .rodata a 0x01F21660 (`lui $at,0x1F2 ; ld $a1,<off>($at)`) alors
   que le harnais, qui ne mappe que .text, place ce pool juste apres le corps
   (`lui $at,0x1EA`).  Declarer S1..S6 `extern` et les epingler par .map fait
   deborder le corps (496 o > 472) : GCC recharge chaque constante.
   Reouverture : placer la section .rodata du pool a 0x01F21660 dans le script
   de lien du harnais, ou etendre .map a une section de pool. */

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

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

static const double
half =  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

	double __kernel_sin(double x, double y, int iy)
{
	double z,r,v;
	__int32_t ix;
	GET_HIGH_WORD(ix,x);
	ix &= 0x7fffffff;			/* high word of x */
	if(ix<0x3e400000)			/* |x| < 2**-27 */
	   {if((int)x==0) return x;}		/* generate inexact */
	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*S6)));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}
