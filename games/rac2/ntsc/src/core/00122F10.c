#include "common.h"

/* libgcc dp-bit.o: __fpcmp_parts (the static __fpcmp_parts_d of GCC's
   fp-bit.c, which keeps its own object-local name in the retail image).
   GPL version 2-or-later with the libgcc linking exception; the
   single-precision sibling of this body is __fpcmp_parts_f in the vendored
   src/core/001232F0.c. Returns a comparison against zero: a<b -> -1,
   a==b -> 0, a>b -> 1, unordered -> 1. */

typedef float DFtype __attribute__ ((mode (DF)));

typedef unsigned int UHItype __attribute__ ((mode (HI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));

typedef UDItype fractype;
typedef USItype halffractype;
typedef DFtype FLO_type;

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

static __inline__ int fp_cmp_isnan(fp_number_type *number) {
    return number->class == CLASS_SNAN || number->class == CLASS_QNAN;
}
static __inline__ int fp_cmp_isinf(fp_number_type *number) {
    return number->class == CLASS_INFINITY;
}
static __inline__ int fp_cmp_iszero(fp_number_type *number) {
    return number->class == CLASS_ZERO;
}

int func_00122F10(fp_number_type *a, fp_number_type *b) {
    if (fp_cmp_isnan(a) || fp_cmp_isnan(b)) {
        return 1;
    }
    if (fp_cmp_isinf(a) && fp_cmp_isinf(b)) {
        return b->sign - a->sign;
    }
    if (fp_cmp_isinf(a)) {
        return a->sign ? -1 : 1;
    }
    if (fp_cmp_isinf(b)) {
        return b->sign ? 1 : -1;
    }
    if (fp_cmp_iszero(a) && fp_cmp_iszero(b)) {
        return 0;
    }
    if (fp_cmp_iszero(a)) {
        return b->sign ? 1 : -1;
    }
    if (fp_cmp_iszero(b)) {
        return a->sign ? -1 : 1;
    }
    if (a->sign != b->sign) {
        return a->sign ? -1 : 1;
    }
    if (a->normal_exp > b->normal_exp) {
        return a->sign ? -1 : 1;
    }
    if (a->normal_exp < b->normal_exp) {
        return a->sign ? 1 : -1;
    }
    if (a->fraction.ll > b->fraction.ll) {
        return a->sign ? -1 : 1;
    }
    if (a->fraction.ll < b->fraction.ll) {
        return a->sign ? 1 : -1;
    }
    return 0;
}
