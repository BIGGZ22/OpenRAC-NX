/* _strtoul_r -- newlib libc/stdlib/strtoul.c (UC Berkeley), adapted to the
 * RAC5 retail: 64-bit long, ctype table at 0x01F27151 (_ctype_+1), errno 34.
 * Provenance: newlib-2000-02-17 strtoul.c; compiler measured to be SN
 * ProDG 3.01 ee-gcc2953 (GCC 2.95.3, sq saves) + Ps2EeAs 2.0, -O2 -G0.
 * `long long` is 128-bit in that cc1: use `long` for the 64-bit words. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long s64;
typedef unsigned long u64;

#define _U 01
#define _L 02
#define _D 04
#define _S 0x08

#define isspace(c) ((D_00152201[(c)] & _S) != 0)
#define isdigit(c) ((D_00152201[(c)] & _D) != 0)
#define isalpha(c) ((D_00152201[(c)] & (_U | _L)) != 0)
#define isupper(c) ((D_00152201[(c)] & _U) != 0)

typedef struct _reent {
    s32 _errno;
} _reent;

extern u8 D_00152201[];
extern u64 __umoddi3(u64, u64);
extern u64 __udivdi3(u64, u64);
extern u64 __muldi3(u64, u64);

u64 _strtoul_r(_reent *rptr, const s8 *nptr, s8 **endptr, s32 base)
{
    register const s8 *s = nptr;
    register u64 acc;
    register s32 c;
    register u64 cutoff;
    register s32 neg = 0, any, cutlim;

    do {
        c = *s++;
    } while (isspace(c));
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
    cutoff = (u64)0xFFFFFFFFFFFFFFFFUL / (u64)base;
    cutlim = (s32)((u64)0xFFFFFFFFFFFFFFFFUL % (u64)base);
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c)) {
            c -= '0';
        } else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
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
            acc = acc * (u64)base;
            acc += (u64)c;
        }
    }
    if (any < 0) {
        acc = (u64)0xFFFFFFFFFFFFFFFFUL;
        rptr->_errno = 34;
    } else if (neg) {
        acc = -acc;
    }
    if (endptr != 0) {
        *endptr = (s8 *)(any ? s - 1 : nptr);
    }
    return acc;
}
