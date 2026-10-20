/* from src/core/00122800.c: 608 bytes against retail's 576, sixteen bytes
 * long, and every instruction but two sequences is retail's.
 *
 * libgcc dp-bit.o: _fpadd_parts, transcribed from GCC's fp-bit.c in its
 * double configuration (the source the qualified compiler is built from,
 * gnu-ee-binutils-gcc-1.1, gcc/config/fp-bit.c). The isnan/isinf/iszero
 * predicates and nan() are the file's own static inlines, and the two
 * constants the body needs are materialised exactly as retail: IMPLICIT_1's
 * range test arrives as ((-1 << 5) >> 4) = IMPLICIT_1 - 2 for
 * `fraction < IMPLICIT_1 && fraction != 0` at +412, IMPLICIT_2's as
 * (-1 >> 3) = IMPLICIT_2 - 1 at +520. `nan()` is the object at 0x00141810
 * (the lui/addiu pair at +76/+84 is the whole of it), declared D_00141810.
 *
 * What blocks it: the mask 0x0FFFFFFFFFFFFFFE (IMPLICIT_1 - 2). It appears
 * twice (the loop test at +412 and the copy at +444) and each time the
 * qualified assembler expands it differently from retail:
 *
 *   retail   addiu $v0,$zero,-1 ; dsll $v0,$v0,5 ; dsrl $v0,$v0,4   (3)
 *   ours     ori $v0,$zero,0xffff ; dsll 16 ; ori 0xffff ; dsll 16 ;
 *            ori 0xffff ; dsll 12 ; ori 0xffe                      (7)
 *
 * Same value, four instructions more each time, which is the whole 32-byte
 * difference. This is gas's load_register, not cc1: the unpatched path below
 * patch 0052 has a mask case ("Optimize if the shifted value is a (power of
 * 2) - 1", which handles `lo == 0xffffffff && ((hi + 1) & hi) == 0` for
 * exactly this value) that emits retail's addiu/dsll/dsrl. Patch
 * 0052-r5900-dli-retail-general intercepts it first: its guard excludes only
 * a plain 2^n - 1 (`wide & (wide + 1) == 0`) and a high word of all ones, and
 * 0x0FFFFFFFFFFFFFFE is neither.
 *
 * What would unblock it: widening patch 0052's guard so the all-ones-run
 * shapes reach the mask path below it (the same narrow exception the
 * __pack_d near miss, func_00122630, asks for from the other direction).
 * Both profiles (-O2 -G0 -ffunction-sections and sdk -O2 -G2
 * -ffunction-sections) produce the same 608 bytes here, so the profile is not
 * the lever.
 *
 * Verified: tools/check.py func_00122800 --diff: boot: 608 bytes, expected
 * 576; the only differences are the two constants above and the D_00141810
 * relocation.
 */

/* libgcc dp-bit.o: _fpadd_parts, transcribed from GCC's fp-bit.c in its
   double configuration (GPL version 2-or-later with the libgcc linking
   exception; the source is the public archive the qualified compiler is
   built from, gnu-ee-binutils-gcc-1.1, gcc/config/fp-bit.c). The
   single-precision build of the same body is vendored in src/core/001232F0.c;
   the body-writing rules here are NGARDS 8, GARDROUND 0x7f, GARDMASK 0xff,
   GARDMSB 0x80, EXPBITS 11, EXPBIAS 1023, EXPMAX 0x7ff, FRACBITS 52,
   FRAC_NBITS 64, IMPLICIT_1 1<<60, IMPLICIT_2 1<<61.

   The two constants the body needs are materialised exactly as retail, by the
   inverse forms the compiler folds them into: IMPLICIT_1's range test arrives
   as ((-1 << 5) >> 4) = IMPLICIT_1 - 2 for `fraction < IMPLICIT_1 && fraction
   != 0` at +412, IMPLICIT_2's as (-1 >> 3) = IMPLICIT_2 - 1 at +520.

   `nan()` is fp-bit.c's static `thenan`, the object the retail image keeps at
   0x00141810 (the lui/addiu pair at +76/+84 is the whole of it). */

typedef float DFtype __attribute__ ((mode (DF)));

typedef int DItype __attribute__ ((mode (DI)));

typedef unsigned int UHItype __attribute__ ((mode (HI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

typedef UDItype fractype;
typedef USItype halffractype;
typedef DItype intfrac;

#define NGARDS 8
#define FRACBITS 52
#define FRAC_NBITS 64

#define IMPLICIT_1 (1LL << (FRACBITS + NGARDS))
#define IMPLICIT_2 (1LL << (FRACBITS + 1 + NGARDS))

/* Preserve the sticky-bit when shifting fractions to the right.  */
#define LSHIFT(a) { a = (a & 1) | (a >> 1); }

typedef enum
{
  CLASS_SNAN,
  CLASS_QNAN,
  CLASS_ZERO,
  CLASS_NUMBER,
  CLASS_INFINITY
} fp_class_type;

typedef struct
{
  fp_class_type class;
  unsigned int sign;
  int normal_exp;

  union
    {
      fractype ll;
      halffractype l[2];
    } fraction;
} fp_number_type;

extern fp_number_type D_00141810;

static __inline__ fp_number_type *nan_parts(void) {
    return &D_00141810;
}

static __inline__ int fp_isnan(fp_number_type *number) {
    return number->class == CLASS_SNAN || number->class == CLASS_QNAN;
}

static __inline__ int fp_isinf(fp_number_type *number) {
    return number->class == CLASS_INFINITY;
}

static __inline__ int fp_iszero(fp_number_type *number) {
    return number->class == CLASS_ZERO;
}

fp_number_type *func_00122800(fp_number_type *a, fp_number_type *b,
                              fp_number_type *tmp) {
  intfrac tfraction;

  /* Put commonly used fields in local variables.  */
  int a_normal_exp;
  int b_normal_exp;
  fractype a_fraction;
  fractype b_fraction;

  if (fp_isnan (a))
    {
      return a;
    }
  if (fp_isnan (b))
    {
      return b;
    }
  if (fp_isinf (a))
    {
      /* Adding infinities with opposite signs yields a NaN.  */
      if (fp_isinf (b) && a->sign != b->sign)
	return nan_parts ();
      return a;
    }
  if (fp_isinf (b))
    {
      return b;
    }
  if (fp_iszero (b))
    {
      if (fp_iszero (a))
	{
	  *tmp = *a;
	  tmp->sign = a->sign & b->sign;
	  return tmp;
	}
      return a;
    }
  if (fp_iszero (a))
    {
      return b;
    }

  /* Got two numbers. shift the smaller and increment the exponent till
     they're the same */
  {
    int diff;

    a_normal_exp = a->normal_exp;
    b_normal_exp = b->normal_exp;
    a_fraction = a->fraction.ll;
    b_fraction = b->fraction.ll;

    diff = a_normal_exp - b_normal_exp;

    if (diff < 0)
      diff = -diff;
    if (diff < FRAC_NBITS)
      {
	/* ??? This does shifts one bit at a time.  Optimize.  */
	while (a_normal_exp > b_normal_exp)
	  {
	    b_normal_exp++;
	    LSHIFT (b_fraction);
	  }
	while (b_normal_exp > a_normal_exp)
	  {
	    a_normal_exp++;
	    LSHIFT (a_fraction);
	  }
      }
    else
      {
	/* Somethings's up.. choose the biggest */
	if (a_normal_exp > b_normal_exp)
	  {
	    b_normal_exp = a_normal_exp;
	    b_fraction = 0;
	  }
	else
	  {
	    a_normal_exp = b_normal_exp;
	    a_fraction = 0;
	  }
      }
  }

  if (a->sign != b->sign)
    {
      if (a->sign)
	{
	  tfraction = -a_fraction + b_fraction;
	}
      else
	{
	  tfraction = a_fraction - b_fraction;
	}
      if (tfraction >= 0)
	{
	  tmp->sign = 0;
	  tmp->normal_exp = a_normal_exp;
	  tmp->fraction.ll = tfraction;
	}
      else
	{
	  tmp->sign = 1;
	  tmp->normal_exp = a_normal_exp;
	  tmp->fraction.ll = -tfraction;
	}
      /* and renormalize it */

      while (tmp->fraction.ll < IMPLICIT_1 && tmp->fraction.ll)
	{
	  tmp->fraction.ll <<= 1;
	  tmp->normal_exp--;
	}
    }
  else
    {
      tmp->sign = a->sign;
      tmp->normal_exp = a_normal_exp;
      tmp->fraction.ll = a_fraction + b_fraction;
    }
  tmp->class = CLASS_NUMBER;
  /* Now the fraction is added, we have to shift down to renormalize the
     number */

  if (tmp->fraction.ll >= IMPLICIT_2)
    {
      LSHIFT (tmp->fraction.ll);
      tmp->normal_exp++;
    }
  return tmp;

}
