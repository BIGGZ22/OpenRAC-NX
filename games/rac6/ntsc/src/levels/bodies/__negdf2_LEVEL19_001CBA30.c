/* __negdf2_LEVEL19_001CBA30 -- LEVEL19.REL 0x001CBA30, 56 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc) :
 *   "fp-bit.c", configuration DOUBLE, US_SOFTWARE_GOFAST.
 *   negate() = __negdf2 : depacketter le double, flipping le signe,
 *   repacketter.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef float DFtype __attribute__ ((mode (DF)));
typedef float SFtype __attribute__ ((mode (SF)));
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

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
      UDItype ll;
      USItype l[2];
    } fraction;
} fp_number_type;

typedef union
{
  DFtype value;
  UDItype value_raw;
  USItype words[2];
} FLO_union_type;

extern void __unpack_d (FLO_union_type *, fp_number_type *);
extern DFtype __pack_d (fp_number_type *);

DFtype
__negdf2 (DFtype arg_a)
{
  fp_number_type a;

  __unpack_d ((FLO_union_type *) & arg_a, &a);
  a.sign = !a.sign;
  return __pack_d (&a);
}
