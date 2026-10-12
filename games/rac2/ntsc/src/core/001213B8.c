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


INCLUDE_ASM("asm/core", func_00121B20);

INCLUDE_ASM("asm/core", func_001220F0);

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

INCLUDE_ASM("asm/core", func_00122DA8);

static __inline__ int double_parts_iszero(DoubleParts *parts) {
    return parts->kind == 2;
}
static __inline__ int double_parts_isnan(DoubleParts *parts) {
    return parts->kind == 0 || parts->kind == 1;
}
static __inline__ int double_parts_isinf(DoubleParts *parts) {
    return parts->kind == 4;
}

INCLUDE_ASM("asm/core", func_00122F10);

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
