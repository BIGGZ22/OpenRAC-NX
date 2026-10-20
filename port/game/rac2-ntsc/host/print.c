/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's formatted prints, answered by the port.
 *
 * newlib's printf (0x00115D48) and sprintf (0x00115DA8) and the kernel
 * library's kprintf (0x0011C7E8) and scePrintf (0x0011C820) are variadic:
 * their C (or their assembly) walks the console's argument save area, which a
 * host va_list is not. The port formats here, as rac1's port does
 * (rac1/host/hand.c, rac1-pal/host/kprintf.c), and the prints go to the log.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "openrac/guest.h"

/* The conversions the game's formats use (d i u x X o c s f e g %, with flags,
 * width and precision), from host arguments: a string argument is a game
 * address. At most SIZE - 1 characters go to OUT, which is terminated; the
 * count written is returned. */
static int rac2_vformat(char* out, size_t size, const char* f, va_list args) {
    size_t n = 0;

    if (size == 0) {
        return 0;
    }
    while (*f != '\0' && n + 1 < size) {
        char spec[32];
        size_t k = 0;
        char piece[512];
        int made = 0;

        if (*f != '%') {
            out[n++] = *f++;
            continue;
        }
        spec[k++] = *f++;
        while (*f != '\0' && strchr("-+ #0123456789.", *f) != NULL && k + 2 < sizeof(spec)) {
            spec[k++] = *f++;
        }
        while (*f == 'l' || *f == 'h') {
            f++; /* the game's int and long are passed as host ints here */
        }
        if (*f == '\0') {
            break;
        }
        spec[k++] = *f;
        spec[k] = '\0';
        switch (*f++) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'c':
                made = snprintf(piece, sizeof(piece), spec, va_arg(args, int));
                break;
            case 's': {
                const gaddr s = (gaddr)va_arg(args, int);
                made = snprintf(piece, sizeof(piece), spec, s != 0 ? (const char*)G(s) : "(null)");
                break;
            }
            case 'f':
            case 'e':
            case 'g':
                made = snprintf(piece, sizeof(piece), spec, va_arg(args, double));
                break;
            case '%':
                made = snprintf(piece, sizeof(piece), "%%");
                break;
            default:
                made = 0;
                break;
        }
        if (made > 0) {
            size_t take = (size_t)made < sizeof(piece) ? (size_t)made : sizeof(piece) - 1;
            if (n + take >= size) {
                take = size - 1 - n;
            }
            memcpy(out + n, piece, take);
            n += take;
        }
    }
    out[n] = '\0';
    return (int)n;
}

/* printf: to the log. */
void func_00115D48(gaddr format, ...) {
    char out[1024];
    va_list args;

    va_start(args, format);
    rac2_vformat(out, sizeof(out), (const char*)G(format), args);
    va_end(args);
    fprintf(stderr, "[game] %s", out);
}

/* sprintf: the text is written into game memory at BUFFER. */
int func_00115DA8(gaddr buffer, gaddr format, ...) {
    char out[1024];
    int n;
    va_list args;

    va_start(args, format);
    n = rac2_vformat(out, sizeof(out), (const char*)G(format), args);
    va_end(args);
    memcpy(G(buffer), out, (size_t)n + 1);
    return n;
}

/* kprintf: the kernel library's debug print. */
void func_0011C7E8(gaddr format, ...) {
    char out[1024];
    va_list args;

    va_start(args, format);
    rac2_vformat(out, sizeof(out), (const char*)G(format), args);
    va_end(args);
    fprintf(stderr, "[game] %s", out);
}

/* scePrintf: the same through the debugger's console; the count written. */
int func_0011C820(gaddr format, ...) {
    char out[1024];
    int n;
    va_list args;

    va_start(args, format);
    n = rac2_vformat(out, sizeof(out), (const char*)G(format), args);
    va_end(args);
    fprintf(stderr, "[game] %s", out);
    return n;
}
