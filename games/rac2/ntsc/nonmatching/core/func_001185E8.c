/* from src/core/001163B0.c: C that compiles to 1508 bytes where retail has 1496.
 *
 * The SDK's vfprintf.obj float-to-string helper.  config/datamine's table names
 * strrev at 0x1184D0 in that same object, and the siblings next door are the
 * rounding counter 0x118460 and the itoa 0x118548, so this is the object's cvt.
 * It is NOT newlib's cvt: it returns a *string* (the digits are built in the
 * static buffer 0x13C080 and the exponent is written with the itoa), VfprintfFloat
 * takes `size` from strlen of the result (0x119180 -> 0x1191F8) and its four
 * special-value strings are "Inf" 0x13A490, "-Inf" 0x13A498, "NaN" 0x13A4A0 and
 * "Ind" 0x13A4A8.  The newlib vfprintf.c in the SDK build tree has none of that
 * (its cvt takes eight arguments and returns a digit pointer), so the body below
 * was reconstructed from the retail disassembly, not transcribed.
 *
 * What the draft gets right, measured: the six-way special-value dispatch is
 * instruction for instruction retail's - same tree, pivot on 0xFFFC0000, signed
 * compares (`slt`, `lui 0xFFF0` sign-extended) - once the source is a switch on
 * `u.i[0] & ~0xFFFF` over `union { double d; int i[2]; }`.  `~0xFFFF` is what
 * makes the tree signed (the same mask written 0xFFFF0000 gives `sltu` and
 * orders the cases differently), and the union member is i[0], the *low* half
 * on this little-endian target, even though the masks are high-word exponent
 * patterns - the source reads its word0 the big-endian way.
 *
 * What is left is twelve bytes of register allocation: the masked value here
 * lands in $v1 against retail's $s2, which also shifts the spill of $a2 by one
 * slot, and the exponent tail keeps one extra live value in $s2 (three
 * instructions).  Every block, call and constant in between agrees.
 *
 * Profile: -O2 -G2 -ffunction-sections -mastra-quad-saves (frame 0xB0, ten `sq`
 * saves, $s0..$s3 at 0x10/0x20/0x30/0x40).  Helpers called, all already named
 * in symbol_addrs: dpcmp 0x123028, dpsub 0x122A98, dpmul 0x122B00, litodp
 * 0x123078, dptoli 0x123130, and the object's own 0x117650 (double round) and
 * 0x117848 (dpdiv). */

extern char D_0013C080[];

extern char D_0013A490[];
extern char D_0013A498[];
extern char D_0013A4A0[];
extern char D_0013A4A8[];

extern int func_00118460(char *buf, int len);

extern char *func_00118548(unsigned int value, char *buf, unsigned int base);

extern double func_00117650(double);

extern double func_00117848(double, double, int);

char *func_001185E8(double value, int ndigits, char ch, int dot) {
    union {
        double d;
        int i[2];
    } u;
    char *p;
    char *q;
    char c;
    double w;
    double f;
    double eps;
    int e;
    int n;
    int i;
    int j;
    int digit;
    int len;

    u.d = value;
    switch (u.i[0] & ~0xFFFF) {
    case 0x7FF00000:
        return D_0013A490;
    case 0xFFF00000:
        return D_0013A498;
    case 0x7FF80000:
    case 0x7FFC0000:
    case 0xFFFC0000:
        return D_0013A4A0;
    case 0xFFF80000:
        return D_0013A4A8;
    }

    e = 0;
    n = 0;
    if (value < 0) {
        D_0013C080[0] = '-';
        p = D_0013C080 + 1;
        value = -value;
    } else {
        p = D_0013C080;
    }

    if (value >= 1.0) {
        f = func_00117650(value);
        value = value - f;
        for (e = 1; e < 163; e++) {
            digit = (int)func_00117848(f, 10.0, n - 1);
            p[n] = digit + '0';
            n++;
            f = (f - (double)digit) * 0.1;
            if (f < 1.0)
                break;
        }
        j = n - 1;
        if (j > 0) {
            i = 0;
            q = p + j;
            do {
                c = *q;
                *q = *p;
                *p = c;
                i++;
                p++;
                j--;
                q--;
            } while (i < j);
            p = D_0013C080;
        }
        e = n;
    }

    if (e == 0) {
        if (ch == 'f') {
            p[0] = '0';
            n = 1;
        } else {
            if (value != 0) {
                while (value < 1.0 && e > -1020) {
                    value = value * 10.0;
                    e--;
                }
            }
            if (value >= 1.0) {
                value = value * 0.1;
                ndigits = ndigits + 1;
            }
        }
    }

    if (e >= 0) {
        if (ch != 'f')
            len = ndigits + 1;
        else
            len = ndigits + (e > 0 ? e : 1);
    } else {
        len = ndigits;
    }

    eps = 5.5511151231257827e-17;
    len = len + 1;
    w = 10.0;
    for (;;) {
        f = value * w;
        digit = (int)f;
        value = f - (double)digit;
        w = w * 10.0;
        eps = eps * 10.0;
        if (n >= len)
            break;
        if (value < eps)
            break;
        if (value > 1.0 - eps)
            break;
        p[n] = digit + '0';
        n++;
    }
    if (value >= 0.5)
        digit = digit + 1;
    p[n] = digit + '0';
    n++;
    while (n <= len) {
        p[n] = '0';
        n++;
    }
    if (func_00118460(p, len + 1) == 0) {
        p[0] = '1';
        e = e + 1;
        len = len + 1;
    }
    if (ndigits == 0) {
        len = len - 1;
    } else {
        i = 1;
        if (ch == 'f' && e > 0)
            i = e;
        if (i < len) {
            q = p + len;
            j = len;
            do {
                j--;
                *q = q[-1];
                q--;
            } while (i < j);
        }
        p[i] = '.';
    }
    if (dot == 0)
        goto skip;
    if (len == 0) {
        c = p[len];
    } else {
        c = p[len];
        if (c == '0') {
            do {
                len--;
                if (len == 0)
                    break;
            } while (p[len] == '0');
            c = p[len];
        }
    }
    if (c == '.')
        len--;
skip:
    if (ch == 'f') {
        p[len + 1] = 0;
    } else {
        p[len + 1] = ch;
        if (e < 0) {
            i = len + 2;
            p[i] = '-';
            if (e > -10) {
                i = len + 3;
                p[i] = '0';
            }
            e = -e;
        } else {
            i = len + 2;
            if (e != 0)
                e = e - 1;
            p[i] = '+';
            if (e < 10) {
                i = len + 3;
                p[i] = '0';
            }
        }
        func_00118548(e, p + i + 1, 10);
    }
    return D_0013C080;
}
