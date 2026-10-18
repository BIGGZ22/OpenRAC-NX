/* fp-bit.c, double precision: unpack (GCC 2.9 config/fp-bit.c), with the
   NO_DENORMALS backport the R&C1 fp-bit carried. Retail: __unpack_d
   0x01E96120..0x01E96400. Types reproduce the measured __pack_d body. */
typedef unsigned int USItype;
typedef unsigned long long UDItype;

typedef enum {
    CLASS_SNAN,
    CLASS_QNAN,
    CLASS_ZERO,
    CLASS_NUMBER,
    CLASS_INFINITY
} fp_class_type;

typedef UDItype fractype;
typedef USItype halffractype;
typedef double FLO_type;

typedef struct {
    fp_class_type class;
    unsigned int sign;
    int normal_exp;
    union {
        fractype ll;
        halffractype l[2];
    } fraction;
} fp_number_type;

typedef union {
    FLO_type value;
    fractype value_raw;
    halffractype words[2];
    struct {
        fractype fraction : 52;
        unsigned int exp : 11;
        unsigned int sign : 1;
    } bits;
} FLO_union_type;

#define NGARDS 8L
#define EXPBITS 11
#define EXPBIAS 1023
#define FRACBITS 52
#define EXPMAX (0x7ff)
#define QUIET_NAN 0x8000000000000LL
#define IMPLICIT_1 (1LL << (FRACBITS + NGARDS))

#define NO_DENORMALS 1

void __unpack_d(const FLO_union_type *src, fp_number_type *dst) {
    fractype fraction;
    int exp;
    int sign;

    fraction = src->value_raw & ((((fractype)1) << FRACBITS) - (fractype)1);
    exp = ((int)(src->value_raw >> FRACBITS)) & ((1 << EXPBITS) - 1);
    sign = ((int)(src->value_raw >> (FRACBITS + EXPBITS))) & 1;

    dst->sign = sign;
    if (exp == 0) {
        if (fraction == 0
#ifdef NO_DENORMALS
            || 1
#endif
        ) {
            /* tastes like zero */
            dst->class = CLASS_ZERO;
        } else {
            dst->normal_exp = exp - EXPBIAS + 1;
            fraction <<= NGARDS;
            dst->class = CLASS_NUMBER;
            while (fraction < IMPLICIT_1) {
                fraction <<= 1;
                dst->normal_exp--;
            }
            dst->fraction.ll = fraction;
        }
    } else if (exp == EXPMAX) {
        if (fraction == 0) {
            dst->class = CLASS_INFINITY;
        } else {
            if (fraction & QUIET_NAN) {
                dst->class = CLASS_QNAN;
            } else {
                dst->class = CLASS_SNAN;
            }
            dst->fraction.ll = fraction;
        }
    } else {
        dst->normal_exp = exp - EXPBIAS;
        dst->class = CLASS_NUMBER;
        dst->fraction.ll = (fraction << NGARDS) | IMPLICIT_1;
    }
}
