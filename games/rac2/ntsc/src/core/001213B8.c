#include "common.h"
#include "structs.h"

extern void r1_func_0011D4A0(void);

extern double func_00123078(int);
extern double func_00122B00(double, double);
extern double func_00122A40(double, double);

double func_001213B8(long value) {
    double high;
    double low;
    unsigned long lower;
    int word;
    high = func_00123078((int)(value >> 32));
    high = func_00122B00(high, 65536.0);
    high = func_00122B00(high, 65536.0);
    lower = (unsigned long)value & 0xFFFFFFFFUL;
    word = (int)lower;
    low = func_00123078(word);
    if (word < 0)
        low = func_00122A40(low, 4294967296.0);
    return func_00122A40(high, low);
}

INCLUDE_ASM("asm/core", func_00121450);

INCLUDE_ASM("asm/core", func_00121AB8);

extern void func_00121B18(void);
extern void r1_func_0011D4A0_00121B18(void);

/* rac1-decomp src/core/0011DD68.c: func_0011DD98 */
/* Tail call: retail is `j r1_func_0011D4A0_00121B18` + nop, with no frame at all.
   2.9-ee emits it natively for a void function that ends in a call. */
void func_00121B18(void) {
    r1_func_0011D4A0_00121B18();
}

/* func_00121B20 and func_001220F0 (libgcc2.c's L_udivdi3 and L_umoddi3,
   `__udivdi3`/`__umoddi3`) are compiled C now: src/core/00121B20.c and
   src/core/001220F0.c. */

INCLUDE_ASM("asm/core", func_00122630);

INCLUDE_ASM("asm/core", func_00122760);

INCLUDE_ASM("asm/core", func_00122800);

typedef struct {
    int kind;
    unsigned int sign;
    int exponent;
    unsigned long fraction;
} DoubleParts;
extern void func_00122760(double *, DoubleParts *);
extern DoubleParts *func_00122800(DoubleParts *, DoubleParts *, DoubleParts *);
extern double func_00122630(DoubleParts *);
extern int func_00122F10(DoubleParts *, DoubleParts *);

double func_00122A40(double first, double second) {
    DoubleParts a, b, result;
    func_00122760(&first, &a);
    func_00122760(&second, &b);
    return func_00122630(func_00122800(&a, &b, &result));
}

double func_00122A98(double first, double second) {
    DoubleParts a, b, result;
    func_00122760(&first, &a);
    func_00122760(&second, &b);
    b.sign ^= 1;
    return func_00122630(func_00122800(&a, &b, &result));
}

INCLUDE_ASM("asm/core", func_00122B00);

/* libgcc dp-bit.o: dpdiv (GCC's fp-bit.c `divide`, whose static `_fpdiv_parts`
   this cc1 inlines because it has a single caller). GPL version 2-or-later
   with the libgcc linking exception. The single-precision sibling is fpdiv in
   the vendored src/core/001232F0.c; the source is rac1-decomp
   src/libgcc/fp-bit.c built with -DNO_DENORMALS -DUS_SOFTWARE_GOFAST.

   The 64-bit shift-subtract loop over the fraction, and the rounding block's
   `ANDI $v1,$a3,0xFF` / `0x80` against GARDMASK/GARDMSB, are the double
   constants: NGARDS 8, GARDROUND 0x7f, GARDMASK 0xff, GARDMSB 0x80,
   FRACBITS 52, so IMPLICIT_1 is 1LL<<60 -- which is retail's
   `ori $v0,$zero,0x8000; dsll32 $v0,$v0,13`. */

#define DP_FRACBITS 52
#define DP_NGARDS 8
#define DP_IMPLICIT_1 (1LL << (DP_FRACBITS + DP_NGARDS))
#define DP_GARDROUND 0x7f
#define DP_GARDMASK 0xff
#define DP_GARDMSB 0x80

extern DoubleParts __thenan_df;

static __inline__ DoubleParts *dp_div_nan(void) {
    return &__thenan_df;
}

/* fp-bit.c's own static INLINE predicates, spelled as they are there:
   the `== CLASS_INFINITY` form is what makes retail compare with
   `xori`/`beqz` rather than `addiu`/`beq`. */
static __inline__ int dp_div_isnan(DoubleParts *number) {
    return number->kind == 0 || number->kind == 1;
}
static __inline__ int dp_div_isinf(DoubleParts *number) {
    return number->kind == 4;
}
static __inline__ int dp_div_iszero(DoubleParts *number) {
    return number->kind == 2;
}

static __inline__ DoubleParts *dp_div_parts(DoubleParts *a, DoubleParts *b) {
    unsigned long bit;
    unsigned long numerator;
    unsigned long denominator;
    unsigned long quotient;

    if (dp_div_isnan(a)) {
        return a;
    }
    if (dp_div_isnan(b)) {
        return b;
    }

    a->sign = a->sign ^ b->sign;

    if (dp_div_isinf(a) || dp_div_iszero(a)) {
        if (a->kind == b->kind) {
            return dp_div_nan();
        }
        return a;
    }
    if (dp_div_isinf(b)) {
        a->fraction = 0;
        a->exponent = 0;
        return a;
    }
    if (dp_div_iszero(b)) {
        a->kind = 4;
        return a;
    }

    a->exponent = a->exponent - b->exponent;
    numerator = a->fraction;
    denominator = b->fraction;

    if (numerator < denominator) {
        numerator *= 2;
        a->exponent--;
    }
    bit = DP_IMPLICIT_1;
    quotient = 0;
    while (bit) {
        if (numerator >= denominator) {
            quotient |= bit;
            numerator -= denominator;
        }
        bit >>= 1;
        numerator *= 2;
    }

    if ((quotient & DP_GARDMASK) == DP_GARDMSB) {
        if (quotient & (1 << DP_NGARDS)) {
            quotient += DP_GARDROUND + 1;
        } else if (numerator) {
            quotient += DP_GARDROUND + 1;
        }
    }

    a->fraction = quotient;
    return a;
}

double func_00122DA8(double first, double second) {
    DoubleParts a, b;
    DoubleParts *res;

    func_00122760(&first, &a);
    func_00122760(&second, &b);

    res = dp_div_parts(&a, &b);

    return func_00122630(res);
}

static __inline__ int double_parts_iszero(DoubleParts *parts) {
    return parts->kind == 2;
}
static __inline__ int double_parts_isnan(DoubleParts *parts) {
    return parts->kind == 0 || parts->kind == 1;
}
static __inline__ int double_parts_isinf(DoubleParts *parts) {
    return parts->kind == 4;
}

int func_00123028(double first, double second) {
    DoubleParts a, b;
    func_00122760(&first, &a);
    func_00122760(&second, &b);
    return func_00122F10(&a, &b);
}

double func_00123078(int value) {
    DoubleParts parts;
    parts.kind = 3;
    parts.sign = value < 0;
    if (value == 0)
        parts.kind = 2;
    else {
        parts.exponent = 60;
        if (parts.sign != 0) {
            if (value == (int)0x80000000)
                return -2147483648.0;
            parts.fraction = -value;
        } else
            parts.fraction = value;
        while (parts.fraction < (1UL << 60)) {
            parts.fraction <<= 1;
            --parts.exponent;
        }
    }
    return func_00122630(&parts);
}

int func_00123130(double value) {
    DoubleParts parts;
    int result;
    func_00122760(&value, &parts);
    if (double_parts_iszero(&parts))
        return 0;
    if (double_parts_isnan(&parts))
        return 0;
    if (double_parts_isinf(&parts))
        return parts.sign != 0 ? (int)0x80000000 : 0x7FFFFFFF;
    if (parts.exponent < 0)
        return 0;
    if (parts.exponent > 30)
        return parts.sign != 0 ? (int)0x80000000 : 0x7FFFFFFF;
    result = (int)(parts.fraction >> (60 - parts.exponent));
    return parts.sign != 0 ? -result : result;
}

unsigned int func_001231C8(double value) {
    DoubleParts parts;
    func_00122760(&value, &parts);
    if (double_parts_iszero(&parts))
        return 0;
    if (double_parts_isnan(&parts))
        return 0;
    if (parts.sign != 0)
        return 0;
    if (double_parts_isinf(&parts))
        return 0xFFFFFFFFU;
    if (parts.exponent < 0)
        return 0;
    if (parts.exponent > 31)
        return 0xFFFFFFFFU;
    if (parts.exponent > 60)
        return parts.fraction << (parts.exponent - 60);
    return parts.fraction >> (60 - parts.exponent);
}
