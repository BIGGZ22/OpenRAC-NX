/* fp-bit.c, double precision: unpacked compare (GCC 2.9 config/fp-bit.c). */
typedef enum {
    CLASS_SNAN,
    CLASS_QNAN,
    CLASS_ZERO,
    CLASS_NUMBER,
    CLASS_INFINITY
} fp_class_type;

typedef struct {
    fp_class_type class;
    unsigned int sign;
    int normal_exp;
    union {
        unsigned long long ll;
        unsigned int l[2];
    } fraction;
} fp_number_type;

__inline__ static int isnan(fp_number_type *x) {
    return x->class == CLASS_SNAN || x->class == CLASS_QNAN;
}

__inline__ static int isinf(fp_number_type *x) {
    return x->class == CLASS_INFINITY;
}

__inline__ static int iszero(fp_number_type *x) {
    return x->class == CLASS_ZERO;
}

int __fpcmp_parts_d(fp_number_type *a, fp_number_type *b) {
    if (isnan(a) || isnan(b)) {
        return 1; /* how to indicate unordered compare? */
    }
    if (isinf(a) && isinf(b)) {
        /* +inf > -inf, but +inf != +inf */
        return b->sign - a->sign;
    }
    /* but not both... */
    if (isinf(a)) {
        return a->sign ? -1 : 1;
    }
    if (isinf(b)) {
        return b->sign ? 1 : -1;
    }
    if (iszero(a) && iszero(b)) {
        return 0;
    }
    if (iszero(a)) {
        return b->sign ? 1 : -1;
    }
    if (iszero(b)) {
        return a->sign ? -1 : 1;
    }
    /* now both are "normal". */
    if (a->sign != b->sign) {
        /* opposite signs */
        return a->sign ? -1 : 1;
    }
    /* same sign; exponents? */
    if (a->normal_exp > b->normal_exp) {
        return a->sign ? -1 : 1;
    }
    if (a->normal_exp < b->normal_exp) {
        return a->sign ? 1 : -1;
    }
    /* same exponents; check size. */
    if (a->fraction.ll > b->fraction.ll) {
        return a->sign ? -1 : 1;
    }
    if (a->fraction.ll < b->fraction.ll) {
        return a->sign ? 1 : -1;
    }
    /* after all that, they're equal. */
    return 0;
}
