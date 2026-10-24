/* fpsub_LEVEL19_001CBF20 -- LEVEL19.REL 0x001CBF20, 100 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc) :
 *   "fp-bit.c", configuration FLOAT (simple precision), US_SOFTWARE_GOFAST.
 *   sub() = fpsub : comme fpadd, mais le signe du second operande est
 *   inverse avant l'addition.
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
      USItype ll;
      UHItype l[2];
    } fraction;
} fp_number_type;

typedef union
{
  SFtype value;
  USItype value_raw;
} FLO_union_type;

extern void __unpack_f (FLO_union_type *, fp_number_type *);
extern SFtype __pack_f (fp_number_type *);
extern fp_number_type *func_1CBC90 (fp_number_type *, fp_number_type *,
				    fp_number_type *);

SFtype
fpsub (SFtype arg_a, SFtype arg_b)
{
  fp_number_type a;
  fp_number_type b;
  fp_number_type tmp;
  fp_number_type *res;

  __unpack_f ((FLO_union_type *) & arg_a, &a);
  __unpack_f ((FLO_union_type *) & arg_b, &b);

  b.sign ^= 1;

  res = func_1CBC90 (&a, &b, &tmp);

  return __pack_f (res);
}
