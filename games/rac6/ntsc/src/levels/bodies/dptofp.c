/* dptofp -- LEVEL19.REL 0x001CBA98, 84 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de bibliotheque du compilateur EE-GCC 2.9 (libgcc) :
 *   "fp-bit.c", configuration DOUBLE, US_SOFTWARE_GOFAST.
 *   df_to_sf() = dptofp : depacketter le double, reduire la mantisse a
 *   celle d'un simple precision, repackager via __make_fp.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

#define INLINE __inline__

#define NGARDS    8L
#define GARDROUND 0x7f
#define GARDMASK  0xff
#define GARDMSB   0x80
#define EXPBITS 11
#define EXPBIAS 1023
#define FRACBITS 52
#define EXPMAX (0x7ff)
#define QUIET_NAN 0x8000000000000LL
#define FRAC_NBITS 64
#define FRACHIGH  0x8000000000000000LL
#define FRACHIGH2 0xc000000000000000LL

#define F_D_BITOFF (52+8-(23+7))

typedef UDItype fractype;
typedef USItype halffractype;

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
  DFtype value;
  fractype value_raw;
  halffractype words[2];
} FLO_union_type;

extern void __unpack_d (FLO_union_type *, fp_number_type *);
extern SFtype __make_fp (fp_class_type, unsigned int, int, USItype);

SFtype
dptofp (DFtype arg_a)
{
  fp_number_type in;
  USItype sffrac;

  __unpack_d ((FLO_union_type *) & arg_a, &in);

  sffrac = in.fraction.ll >> F_D_BITOFF;

  /* We set the lowest guard bit in SFFRAC if we discarded any non
     zero bits.  */
  if ((in.fraction.ll & (((USItype) 1 << F_D_BITOFF) - 1)) != 0)
    sffrac |= 1;

  return __make_fp (in.class, in.sign, in.normal_exp, sffrac);
}
