/* fp-bit.c, single precision: pack (GCC 2.9 config/fp-bit.c). */
typedef unsigned int USItype;
typedef unsigned short UHItype;

typedef enum {
    CLASS_SNAN,
    CLASS_QNAN,
    CLASS_ZERO,
    CLASS_NUMBER,
    CLASS_INFINITY
} fp_class_type;

typedef USItype fractype;
typedef UHItype halffractype;
typedef float FLO_type;

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
        fractype fraction : 23;
        unsigned int exp : 8;
        unsigned int sign : 1;
    } bits;
} FLO_union_type;

#define NGARDS 7L
#define EXPBITS 8
#define EXPBIAS 127
#define FRACBITS 23
#define EXPMAX (0xff)
#define QUIET_NAN 0x100000L
#define FRAC_NBITS 32
#define GARDROUND 0x3f
#define GARDMASK 0x7f
#define GARDMSB 0x40
#define NORMAL_EXPMIN (-(EXPBIAS) + 1)
#define IMPLICIT_1 (1LL << (FRACBITS + NGARDS))
#define IMPLICIT_2 (1LL << (FRACBITS + 1 + NGARDS))

__inline__ static int isnan(fp_number_type *x) {
    return x->class == CLASS_SNAN || x->class == CLASS_QNAN;
}

__inline__ static int isinf(fp_number_type *x) {
    return x->class == CLASS_INFINITY;
}

__inline__ static int iszero(fp_number_type *x) {
    return x->class == CLASS_ZERO;
}

float __pack_f(fp_number_type *src) {
    FLO_union_type dst;
    fractype fraction = src->fraction.ll; /* wasn't unsigned before? */
    int sign = src->sign;
    int exp = 0;

    if (isnan(src)) {
        exp = EXPMAX;
        if (src->class == CLASS_QNAN || 1) {
            fraction |= QUIET_NAN;
        }
    } else if (isinf(src)) {
        exp = EXPMAX;
        fraction = 0;
    } else if (iszero(src)) {
        exp = 0;
        fraction = 0;
    } else if (fraction == 0) {
        exp = 0;
    } else {
        if (src->normal_exp < NORMAL_EXPMIN) {
            /* This number's exponent is too low to fit into the bits
               available in the number, so we'll store 0 in the exponent and
               shift the fraction to the right to make up for it.  */
            int shift = NORMAL_EXPMIN - src->normal_exp;

            exp = 0;

            if (shift > FRAC_NBITS - NGARDS) {
                /* No point shifting, since it's more that 64 out.  */
                fraction = 0;
            } else {
                /* Shift by the value */
                fraction >>= shift;
            }
            fraction >>= NGARDS;
        } else if (src->normal_exp > EXPBIAS) {
            exp = EXPMAX;
            fraction = 0;
        } else {
            exp = src->normal_exp + EXPBIAS;
            /* IF the gard bits are the all zero, but the first, then we're
               half way between two numbers, choose the one which makes the
               lsb of the answer 0.  */
            if ((fraction & GARDMASK) == GARDMSB) {
                if (fraction & (1 << NGARDS)) fraction += GARDROUND + 1;
            } else {
                /* Add a one to the guards to round up */
                fraction += GARDROUND;
            }
            if (fraction >= IMPLICIT_2) {
                fraction >>= 1;
                exp += 1;
            }
            fraction >>= NGARDS;
        }
    }

    /* We previously used bitfields to store the number, but this doesn't
       handle little/big endian systems conveniently, so use shifts and
       masks */
    dst.bits.fraction = fraction;
    dst.bits.exp = exp;
    dst.bits.sign = sign;

    return dst.value;
}
