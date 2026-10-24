/* fpmul -- LEVEL19.REL 0x001CBF88, 500 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc) :
 *   "fp-bit.c", configuration FLOAT (simple precision), US_SOFTWARE_GOFAST.
 *   multiply() = fpmul : _fpmul_parts est "static INLINE", donc la
 *   multiplication 64 bits est developpee dans le corps ; le resultat est
 *   repackete par __pack_f.
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
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

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

#define LSHIFT(a) { a = (a & 1) | (a >> 1); }

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
 * en 0x003F3100 dans le module retail (mesure : lui 0x3F / addiu 0x3100).
 * Elle est declaree externee pour que le harnais la pose a cette adresse
 * (sidecar .map). */
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
_fpmul_parts (fp_number_type *a, fp_number_type *b, fp_number_type *tmp)
{
  fractype low = 0;
  fractype high = 0;

  if (isnan (a))
    {
      a->sign = a->sign != b->sign;
      return a;
    }
  if (isnan (b))
    {
      b->sign = a->sign != b->sign;
      return b;
    }
  if (isinf (a))
    {
      if (iszero (b))
	return nan ();
      a->sign = a->sign != b->sign;
      return a;
    }
  if (isinf (b))
    {
      if (iszero (a))
	{
	  return nan ();
	}
      b->sign = a->sign != b->sign;
      return b;
    }
  if (iszero (a))
    {
      a->sign = a->sign != b->sign;
      return a;
    }
  if (iszero (b))
    {
      b->sign = a->sign != b->sign;
      return b;
    }

  {
    DItype answer = (DItype) (a->fraction.ll) * (DItype) (b->fraction.ll);

    high = answer >> 32;
    low = answer;
  }
  tmp->normal_exp = a->normal_exp + b->normal_exp;
  tmp->sign = a->sign != b->sign;
  tmp->normal_exp += 2;
  while (high >= IMPLICIT_2)
    {
      tmp->normal_exp++;
      if (high & 1)
	{
	  low >>= 1;
	  low |= FRACHIGH;
	}
      high >>= 1;
    }
  while (high < IMPLICIT_1)
    {
      tmp->normal_exp--;

      high <<= 1;
      if (low & FRACHIGH)
	high |= 1;
      low <<= 1;
    }
  if ((high & GARDMASK) == GARDMSB)
    {
      if (high & (1 << NGARDS))
	{
	  high += GARDROUND + 1;
	}
      else if (low)
	{
	  high += GARDROUND + 1;
	}
    }
  tmp->fraction.ll = high;
  tmp->class = CLASS_NUMBER;
  return tmp;
}

SFtype
fpmul (SFtype arg_a, SFtype arg_b)
{
  fp_number_type a;
  fp_number_type b;
  fp_number_type tmp;
  fp_number_type *res;

  __unpack_f ((FLO_union_type *) & arg_a, &a);
  __unpack_f ((FLO_union_type *) & arg_b, &b);

  res = _fpmul_parts (&a, &b, &tmp);

  return __pack_f (res);
}
