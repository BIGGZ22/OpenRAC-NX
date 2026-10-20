#include "common.h"

/* libgcc libgcc2.c: __fixunsdfdi, the L_fixunsdfdi member (GPL version
   2-or-later with the libgcc linking exception; the source is the public
   archive the qualified compiler is built from, gnu-ee-binutils-gcc-1.1,
   gcc/libgcc2.c).

   The division is by the integer constant 1 << 32, which this compiler folds
   into a multiplication by its reciprocal: retail materialises 2^-32
   (0x3DF0000000000000, as ori 0xF7C0 + dsll32 14) before the call. The
   helper entry points keep the names the compiler's own optabs carry
   (config/gofast.h): dpcmp, dpmul, dpsub, dptoul, and __floatundidf for the
   UDItype -> DFtype cast, which the retail image holds at 0x001213B8. */

typedef float DFtype __attribute__ ((mode (DF)));
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned long long UDItype __attribute__ ((mode (DI)));

extern int dpcmp(double, double) __asm__("func_00123028");
extern double dpmul(double, double) __asm__("func_00122B00");
extern double dpsub(double, double) __asm__("func_00122A98");
extern unsigned int dptoul(double) __asm__("func_001231C8");
extern double __floatdidf(long long) __asm__("func_001213B8");
extern double __floatundidf(unsigned long long) __asm__("func_001213B8");

#define WORD_SIZE (sizeof (SItype) * 8)
#define HIGH_WORD_COEFF (((UDItype) 1) << WORD_SIZE)

UDItype func_001212C8(DFtype a) {
    DFtype b;
    UDItype v;

    if (a < 0)
        return 0;

    /* Compute high word of result, as a flonum.  */
    b = (a / HIGH_WORD_COEFF);
    /* Convert that to fixed (but not to DItype!), and shift it into the high
       word.  */
    v = (USItype) b;
    v <<= WORD_SIZE;
    /* Remove high part from the DFtype, leaving the low part as flonum.  */
    a -= (DFtype) v;
    /* Convert that to fixed (but not to DItype!) and add it in. Sometimes A
       comes out negative. This is significant, since A has more bits than a
       long int does.  */
    if (a < 0)
        v -= (USItype) (- a);
    else
        v += (USItype) a;
    return v;
}

INCLUDE_ASM("asm/core", func_001213B4);
