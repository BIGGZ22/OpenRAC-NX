/* libkernl.a:kprintf.o _printf - the print core. kprintf
 * (0x11F828's neighbour) and scePrintf both call it; it walks fmt with the
 * va_list, writes every character through the hook D_00134698, and is the
 * engine behind "%o %x %d %u %e %f %s %c" with the optional "%0N" width and
 * the l/h modifiers.
 *
 * Transcribed from Lombyte (GPL-3.0) src/assembly/sdk/debug/printf_impl.c, which
 * keeps the same body under its NON_MATCHING branch. This candidate is close
 * but NOT exact: tools/check.py func_0011C1F8 --diff reports
 *   `boot: 1512 bytes, expected 1516`
 * - the candidate carries one extra `daddu $v0,$zero,$zero` (a `return 0` the
 * upstream `void _printf` does not have; retail sets no return value) and is
 * one instruction short elsewhere, and two blocks are scheduled in the other
 * order (`andi $v0,$v0,0xFF` before `addiu $s0,$s0,-0x1` at 0x11C444, where
 * retail hoists the decrement). Everything else - the 8-byte ARG stride at
 * eleven sites, the `l`/`h` modifier in $a3 tested against 0x6C, the jump
 * table dispatch, the 64-bit digit loops calling libgcc out of the binary
 * (__divdi3 0x11FC68, and 0x121450/0x1220F0/0x121B20 for the `%` and unsigned
 * forms) - matches.
 *
 * Even when exact it could not be placed: the switch over (char)(*fmt - '0')
 * becomes a jump table and retail keeps it at 0x13A9F0, so tools/build.py
 * would refuse it with `needs .rodata at its place, which is not C`, the same
 * refusal measured for func_00115FC0. Kept here as the working starting point.
 */

/* _printf (libkernl.a:kprintf.o) - the print core: walks fmt with the
 * va_list, writing every character through the hook D_00134698, and returns
 * nothing useful. kprintf (0x11F828's neighbour) and scePrintf both call it;
 * it is the engine behind the "%o %x %d %u %e %f %s %c" set.
 *
 * Transcribed from Lombyte (GPL-3.0) src/assembly/sdk/debug/printf_impl.c,
 * sha256 e0e1d4cc7ff0e56cc2b5d22ee8be2cf5c26ce3d2f3ffa4a9a6a0f5ef5ce4b3c1
 * (upstream keeps the body under its NON_MATCHING branch). Confirmed against
 * RAC2's own bytes: the va_list advances by 8 per ARG (`addiu $s3,$s3,0x8`,
 * eleven sites), the hook is loaded per use from 0x134698 (`lw $v0,0x4698($s5)`
 * with $s5 = 0x130000), the `l`/`h` modifier lives in $a3 and is compared
 * against 0x6C, and the 64-bit digit loops call libgcc out of the binary
 * (__divdi3 at 0x11FC68 for `v / 10`, and 0x121450/0x1220F0/0x121B20 for the
 * matching `%` and unsigned forms).
 *
 * The switch over (char)(*fmt - '0') becomes a jump table at 0x13A9F0, so this
 * cannot be placed by C even when it is exact; see nonmatching/core. */

#define ARG(T) (ap += 8, *(T *)(ap - 8))

extern void (*D_00134698[])(int);

extern void func_0011C090(double);

extern int func_0011F5E0(void);

extern void func_0011F628(void);

int func_0011C1F8(const char *fmt, char *ap) {
    char buf[32];
    char *pad;
    char *p;
    int mod;
    int n;
    unsigned long d;
    unsigned char ch;
    char second;
    long long v;
    int ie;

    ie = func_0011F5E0();
    for (; *fmt != 0; fmt++) {
        pad = 0;
        mod = 0;
        if (*fmt == '%') {
        again:
            fmt++;
            switch ((char)(*fmt - '0')) {
            case 0:
                n = fmt[1] - '0';
                second = fmt[2];
                if ((unsigned char)n < 10) {
                    if ((unsigned int)(second - '0') < 10) {
                        n = n * 10 + (second - '0');
                        fmt += 2;
                        if (n >= 32) {
                            n = 31;
                        }
                    } else {
                        fmt += 1;
                    }
                    pad = &buf[31 - n];
                    while (n > 0) {
                        buf[31 - n] = '0';
                        n--;
                    }
                    goto again;
                }
                goto again;
            case 'l' - '0':
                mod = 'l';
                goto again;
            case 'h' - '0':
                mod = 'h';
                goto again;
            case 'o' - '0':
                if (mod == 'l') {
                    v = ARG(long long);
                } else if (mod == 'h') {
                    v = ARG(unsigned short);
                } else {
                    v = ARG(unsigned int);
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        *--p = (v & 7) + '0';
                        v = (unsigned long long)v >> 3;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_00134698[0](*p++);
                }
                break;
            case 'x' - '0':
                if (mod == 'l') {
                    v = ARG(long long);
                } else if (mod == 'h') {
                    v = ARG(unsigned short);
                } else {
                    v = ARG(unsigned int);
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        d = v & 0xF;
                        ch = (d < 10) ? d + '0' : d + 'a' - 10;
                        *--p = ch;
                        v = (unsigned long long)v >> 4;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_00134698[0](*p++);
                }
                break;
            case 'd' - '0':
                if (mod == 'l') {
                    v = ARG(long long);
                } else if (mod == 'h') {
                    v = ARG(short);
                } else {
                    v = ARG(int);
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    if (v < 0) {
                        v = -v;
                        D_00134698[0]('-');
                    }
                    while (v != 0) {
                        *--p = v % 10 + '0';
                        v = v / 10;
                    }
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_00134698[0](*p++);
                }
                break;
            case 'u' - '0':
                if (mod == 'l') {
                    v = ARG(long long);
                } else if (mod == 'h') {
                    v = ARG(unsigned short);
                } else {
                    v = ARG(unsigned int);
                }
                p = &buf[31];
                *p = 0;
                if (v == 0) {
                    *--p = '0';
                } else {
                    do {
                        *--p = (unsigned long long)v % 10 + '0';
                        v = (unsigned long long)v / 10;
                    } while (v != 0);
                }
                if (pad != 0 && pad < p) {
                    p = pad;
                }
                while (*p != 0) {
                    D_00134698[0](*p++);
                }
                break;
            case 'e' - '0':
            case 'f' - '0': {
                float f = ARG(float);
                if (f == 0.0f) {
                    D_00134698[0]('0');
                } else {
                    func_0011C090(f);
                }
            } break;
            case 's' - '0':
                p = ARG(char *);
                if (*p == 0) {
                    D_00134698[0]('(');
                    D_00134698[0]('n');
                    D_00134698[0]('u');
                    D_00134698[0]('l');
                    D_00134698[0]('l');
                    D_00134698[0](')');
                } else {
                    if (*p != 0) {
                        do {
                            D_00134698[0](*p++);
                        } while (*p != 0);
                    }
                }
                break;
            case 'c' - '0':
                v = ARG(char);
                D_00134698[0](v);
                break;
            default:
                break;
            }
        } else {
            D_00134698[0](*fmt);
        }
    }
    if (ie != 0) {
        func_0011F628();
    }
    return 0;
}
