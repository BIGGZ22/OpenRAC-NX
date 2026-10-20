/* NEAR MISS - candidate size 884 bytes, exact; 219 of 221 instructions
   identical, and both differences are one constant's `dli` form.

   newlib strtol.c's `_strtol_r` (build/sdk-compiler-build/source/ee/newlib/
   libc/stdlib/strtol.c), the body src/core/001175F0.c (the `strtol` shim)
   calls.  Profile -O0 -G2 -ffunction-sections -mastra-quad-saves: the four
   arguments are spilled to the frame, `$fp` addresses it and the unit saves
   $s0..$s6, $fp, $ra in `sq` slots.  `_ctype_` (0x0013A200, the old ctype.h
   macros), `_errno` at offset 0 of struct _reent, 64-bit long, so
   LONG_MAX/LONG_MIN are the 64-bit pair, `cutoff % base` and `cutoff / base`
   are __umoddi3 (func_00122 0F0) and __udivdi3 (func_00121B20) and
   `acc *= base` is __muldi3 (func_00121AB8).  LONG_MAX comes out exactly as
   retail's `addiu $s3,$zero,-1; dsrl $s3,$s3,1`.

   The two differences are the negated LONG_MIN (= 0x8000000000000000) at
   +0x150 and +0x2D0 (the `any < 0` arm), where retail has
   `addiu $x,$zero,-1; dsll32 $x,$x,31` and this cc1 has
   `ori $x,$zero,0x8000; dsll32 $x,$x,16`.

   Measured, not guessed: a probe compiled with this profile over seven
   spellings of that constant ( -(unsigned long)LONG_MIN, the 0x8000000000000000UL
   literal, a UL suffix, `(unsigned long)-LONG_MIN`, the folded `-1UL << 63`,
   and an inline -0x7FFFFFFFFFFFFFFFL - 1L ) emits `ori`/`dsll32 16` for every
   one of them, so no source form reaches retail's pair.  Same class as the
   patch 0052 (r5900-dli-retail-general) mask wall recorded for __pack_d: the
   patch's rule covers the runs of ones but not the single-bit-at-63 case.
   Fixing patch 0052 for 0x8000000000000000 should make this file match as it
   stands.  */
#include "common.h"

/* Source: newlib strtol.c, `_strtol_r`
 * (build/sdk-compiler-build/source/ee/newlib/libc/stdlib/strtol.c), the unit the
 * `strtol` shim src/core/001175F0.c calls with the reentrancy pointer - that shim
 * is -O0 and this body is its twin: every named argument is spilled to the frame
 * (rptr at 0, nptr at 4, endptr at 8, base at 0xC), `$fp` addresses it, and each
 * source statement becomes its own basic block, so the profile is
 * `-O0 -G2 -ffunction-sections -mastra-quad-saves` (the unit saves $s0..$s6, $fp
 * and $ra in `sq` slots). `long` is 64 bits here, so LONG_MAX/LONG_MIN are the
 * 64-bit pair, `cutoff % base` and `cutoff / base` are libgcc's __umoddi3
 * (func_00122 0F0) and __udivdi3 (func_00121B20), and `acc *= base` is __muldi3
 * (func_00121AB8). The character classes come from this SDK's old ctype.h, whose
 * macros index `_ctype_` (0x0013A200) at +1; `_errno` is the first int of
 * struct _reent. */

struct _reent {
    int _errno;
};

extern const char _ctype_[];

#define _U   01
#define _L   02
#define _N   04
#define _S   010

#define isalpha(c)  ((_ctype_ + 1)[(unsigned)(c)] & (_U | _L))
#define isupper(c)  ((_ctype_ + 1)[(unsigned)(c)] & _U)
#define isdigit(c)  ((_ctype_ + 1)[(unsigned)(c)] & _N)
#define isspace(c)  ((_ctype_ + 1)[(unsigned)(c)] & _S)

#define LONG_MAX 0x7FFFFFFFFFFFFFFFL
#define LONG_MIN (-LONG_MAX - 1L)

#define ERANGE 34

long func_00117278(struct _reent *rptr, const char *nptr, char **endptr, int base) {
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        rptr->_errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (acc);
}
