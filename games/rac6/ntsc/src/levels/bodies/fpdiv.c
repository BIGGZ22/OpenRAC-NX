/* fpdiv -- LEVEL19.REL 0x001CC180, 352 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc) :
 *   "fp-bit.c", configuration FLOAT (simple precision), US_SOFTWARE_GOFAST.
 *   divide() = fpdiv : _fpdiv_parts est "static INLINE", la division
 *   bit a bit est donc developpee dans le corps.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111-sdkas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef float SFtype __attribute__ ((mode (SF)));
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UHItype __attribute__ ((mode (HI)));

#define INLINE __inline__

#define NGARDS    7L
#define GARDROUND 0x3f
#define GARDMASK  0x7f
#define GARDMSB   0x40
#define EXPBITS 8
#define EXPBIAS 127
#define FRACBITS 23
#define EXPMAX (0xff)
#define QUIET_NAN 0x100000L
#define FRAC_NBITS 32
#define FRACHIGH  0x80000000L
#define FRACHIGH2 0xc0000000L

#define NORMAL_EXPMIN (-(EXPBIAS)+1)
#define IMPLICIT_1 (1LL<<(FRACBITS+NGARDS))
#define IMPLICIT_2 (1LL<<(FRACBITS+1+NGARDS))

typedef USItype fractype;
typedef UHItype halffractype;

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

typedef union
{
  SFtype value;
  fractype value_raw;
} FLO_union_type;

/* La constante "thenan" du nan() de fp-bit.c est un objet statique place
 * en 0x003F3100 dans le module retail ; elle est declaree externee pour
 * que le harnais la pose a cette adresse (sidecar .map). */
extern fp_number_type thenan;

INLINE
static fp_number_type *
nan ()
{
  return &thenan;
}

INLINE
static int
isnan (fp_number_type *x)
{
  return x->class == CLASS_SNAN || x->class == CLASS_QNAN;
}

INLINE
static int
isinf (fp_number_type *x)
{
  return x->class == CLASS_INFINITY;
}

INLINE
static int
iszero (fp_number_type *x)
{
  return x->class == CLASS_ZERO;
}

extern void __unpack_f (FLO_union_type *, fp_number_type *);
extern SFtype __pack_f (fp_number_type *);

static INLINE fp_number_type *
_fpdiv_parts (fp_number_type *a, fp_number_type *b)
{
  fractype bit;
  fractype numerator;
  fractype denominator;
  fractype quotient;

  if (isnan (a))
    {
      return a;
    }
  if (isnan (b))
    {
      return b;
    }

  a->sign = a->sign ^ b->sign;

  if (isinf (a) || iszero (a))
    {
      if (a->class == b->class)
	return nan ();
      return a;
    }

  if (isinf (b))
    {
      a->fraction.ll = 0;
      a->normal_exp = 0;
      return a;
    }
  if (iszero (b))
    {
      a->class = CLASS_INFINITY;
      return a;
    }

  {
    a->normal_exp = a->normal_exp - b->normal_exp;
    numerator = a->fraction.ll;
    denominator = b->fraction.ll;

    if (numerator < denominator)
      {
	numerator *= 2;
	a->normal_exp--;
      }
    bit = IMPLICIT_1;
    quotient = 0;
    while (bit)
      {
	if (numerator >= denominator)
	  {
	    quotient |= bit;
	    numerator -= denominator;
	  }
	bit >>= 1;
	numerator *= 2;
      }

    if ((quotient & GARDMASK) == GARDMSB)
      {
	if (quotient & (1 << NGARDS))
	  {
	    quotient += GARDROUND + 1;
	  }
	else if (numerator)
	  {
	    quotient += GARDROUND + 1;
	  }
      }

    a->fraction.ll = quotient;
    return (a);
  }
}

SFtype
fpdiv (SFtype arg_a, SFtype arg_b)
{
  fp_number_type a;
  fp_number_type b;
  fp_number_type *res;

  __unpack_f ((FLO_union_type *) & arg_a, &a);
  __unpack_f ((FLO_union_type *) & arg_b, &b);

  res = _fpdiv_parts (&a, &b);

  return __pack_f (res);
}
