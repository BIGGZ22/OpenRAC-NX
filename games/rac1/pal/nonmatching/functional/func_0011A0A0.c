/* functional: medium. The kprintf core: walks fmt, sends every character through the hook D_0012FD00, handles %[0<w>][h|l] with o x d u, %e/%f (via the soft-float printer), %s ("(null)" for an empty string), %c; arguments come from 8-byte slots at args; unknown specifiers are skipped silently. */
extern void *D_0012FD00_A0A0 __asm__("D_0012FD00");
extern s32 func_0011D960_A0A0(void) __asm__("func_0011D960");
extern void func_0011D9A8_A0A0(void) __asm__("func_0011D9A8");
extern s64 func_0011E860_A0A0(s64, s64) __asm__("func_0011E860");
extern s64 func_0011DFE8_A0A0(s64, s64) __asm__("func_0011DFE8");
extern u64 func_0011F4F8_A0A0(u64, u64) __asm__("func_0011F4F8");
extern u64 func_0011EF28_A0A0(u64, u64) __asm__("func_0011EF28");
extern u64 func_00120778_A0A0(f32) __asm__("func_00120778");
extern void func_00119F38_A0A0(u64) __asm__("func_00119F38");
extern void func_0011A0A0_A0A0(const char *fmt, u8 *args) __asm__("func_0011A0A0");

void func_0011A0A0_A0A0(const char *fmt, u8 *args) {
    char buf[0x20];
    const char *s;
    const char *p;
    char *out;
    char *padstart;
    u64 v;
    s64 sv;
    s32 saved;
    s32 mod;
    s32 w;
    s32 d1;
    s32 d2;
    s32 nib;

    saved = func_0011D960_A0A0();
    s = fmt;
    while (*s != 0) {
        padstart = 0;
        mod = 0;
        if (*s != '%') {
            ((void (*)(s32))D_0012FD00_A0A0)(*s);
            s++;
            continue;
        }
        s++;
        for (;;) {
            u32 idx = (u32)(s8)(*(u8 *)s - 0x30);
            if (idx >= 0x49) {
                s++;
                break;
            }
            switch (idx) {
            case 0:
                d1 = s[1] - 0x30;
                if ((u8)d1 >= 10) {
                    s++;
                    continue;
                }
                if ((u32)(s[2] - 0x30) < 10) {
                    w = d1 * 10 - 0x30 + s[2];
                    if (w >= 0x20)
                        w = 0x1F;
                    s += 2;
                } else {
                    w = d1;
                    s += 1;
                }
                padstart = buf + 0x1F - w;
                s++;
                {
                    s32 q;
                    for (q = w; q > 0; q--)
                        buf[0x1F - q] = '0';
                }
                continue;
            case 'h' - 0x30:
                mod = 'h';
                s++;
                continue;
            case 'l' - 0x30:
                mod = 'l';
                s++;
                continue;
            case 'o' - 0x30:
            case 'x' - 0x30:
            case 'd' - 0x30:
            case 'u' - 0x30:
                out = buf + 0x1F;
                d2 = idx + 0x30;
                if (d2 == 'd') {
                    if (mod == 'l')
                        sv = *(s64 *)args;
                    else if (mod == 'h')
                        sv = *(s16 *)args;
                    else
                        sv = *(s32 *)args;
                    v = (u64)sv;
                } else {
                    if (mod == 'l')
                        v = *(u64 *)args;
                    else if (mod == 'h')
                        v = *(u16 *)args;
                    else
                        v = *(u32 *)args;
                    sv = (s64)v;
                }
                args += 8;
                buf[0x1F] = 0;
                if (v == 0) {
                    out = buf + 0x1E;
                    buf[0x1E] = '0';
                    s++;
                } else if (d2 == 'o') {
                    s++;
                    do {
                        out--;
                        *out = (char)((v & 7) + 0x30);
                        v >>= 3;
                    } while (v != 0);
                } else if (d2 == 'x') {
                    s++;
                    do {
                        nib = (s32)(v & 0xF);
                        out--;
                        *out = (char)(nib < 10 ? nib + 0x30 : nib + 0x57);
                        v >>= 4;
                    } while (v != 0);
                } else if (d2 == 'd') {
                    if (sv < 0) {
                        sv = -sv;
                        ((void (*)(s32))D_0012FD00_A0A0)(0x2D);
                    }
                    s++;
                    if (sv != 0) {
                        do {
                            out--;
                            *out = (char)(func_0011E860_A0A0(sv, 10) + 0x30);
                            sv = func_0011DFE8_A0A0(sv, 10);
                        } while (sv != 0);
                    }
                } else {
                    s++;
                    do {
                        out--;
                        *out = (char)(func_0011F4F8_A0A0(v, 10) + 0x30);
                        v = func_0011EF28_A0A0(v, 10);
                    } while (v != 0);
                }
                if (padstart != 0 && padstart < out)
                    out = padstart;
                while (*out != 0) {
                    ((void (*)(s32))D_0012FD00_A0A0)(*out);
                    out++;
                }
                break;
            case 'e' - 0x30:
            case 'f' - 0x30: {
                f32 f = *(f32 *)args;
                args += 8;
                s++;
                if (f == 0.0f)
                    ((void (*)(s32))D_0012FD00_A0A0)(0x30);
                else
                    func_00119F38_A0A0(func_00120778_A0A0(f));
                break;
            }
            case 's' - 0x30:
                p = *(const char **)args;
                args += 8;
                s++;
                if (*p == 0) {
                    ((void (*)(s32))D_0012FD00_A0A0)('(');
                    ((void (*)(s32))D_0012FD00_A0A0)('n');
                    ((void (*)(s32))D_0012FD00_A0A0)('u');
                    ((void (*)(s32))D_0012FD00_A0A0)('l');
                    ((void (*)(s32))D_0012FD00_A0A0)('l');
                    ((void (*)(s32))D_0012FD00_A0A0)(')');
                } else {
                    while (*p != 0) {
                        ((void (*)(s32))D_0012FD00_A0A0)(*p);
                        p++;
                    }
                }
                break;
            case 'c' - 0x30:
                v = (u64)(s64)*(s8 *)args;
                args += 8;
                s++;
                ((void (*)(s32))D_0012FD00_A0A0)((s32)v);
                break;
            default:
                s++;
                break;
            }
            break;
        }
    }
    if (saved != 0)
        func_0011D9A8_A0A0();
}
