#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x116D30-0x116FA0. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);

#define LONG_MAX 0x7fffffffffffffffL
#define LONG_MIN (-LONG_MAX - 1)
#define ERANGE 34

/* newlib's ctype table (_ctype_+1, so it can be indexed by EOF==-1 too):
   bit 0x01 upper, 0x02 lower, 0x04 digit, 0x08 space. */
extern unsigned char D_00152301[];
#define ISSPACE(c) (D_00152301[(int)(c)] & 0x08)
#define ISDIGIT(c) (D_00152301[(int)(c)] & 0x04)
#define ISALPHA(c) (D_00152301[(int)(c)] & 0x03)
#define ISUPPER(c) (D_00152301[(int)(c)] & 0x01)

/* newlib's _strtol_r (strtol.c), verbatim modulo `long` being 64-bit on
   this ABI: skip whitespace, an optional sign, an optional 0x/0X hex
   prefix (base 0 or 16), default base 8/10 from a leading '0'; compute
   the overflow cutoff (LONG_MAX or -LONG_MIN, divided/modded by base)
   and accumulate digits, clamping to LONG_MIN/MAX and setting
   errno=ERANGE on overflow; *endptr, if given, points past the last
   digit consumed (or back at nptr if none were). */
long func_00116D30(void *rptr, int nptr_, int endptr_, int base) {
    register const char *s = (const char *)nptr_;
    char **endptr = (char **)endptr_;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    do {
        c = *s++;
    } while (ISSPACE(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }

    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (ISDIGIT(c)) {
            c -= '0';
        } else if (ISALPHA(c)) {
            c -= ISUPPER(c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        *(int *)rptr = ERANGE;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr != 0) {
        *endptr = (char *)(any ? s - 1 : (const char *)nptr_);
    }
    return (acc);
}

extern long func_00116D30(void *, int, int, int);

/* newlib's strtol: _strtol_r (func_00116D30) on _REENT. */
long func_00116F68(int arg0, int arg1, int arg2) {
    return func_00116D30(D_0012F86C, arg0, arg1, arg2);
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116F9C);
