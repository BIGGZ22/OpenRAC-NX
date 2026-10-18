/* PS2 SDK fiprintf, retail 0x01ECF490 (72 bytes): varargs front end that hands
 * the register save area to vfiprintf.
 *
 * The pinned chain has no include path, so the two lines <stdarg.h> would
 * define are expanded here exactly as gcc/ginclude/stdarg.h does.  Measured
 * (IDIOMES.md 8, 12): this chain's __builtin_next_arg yields the frame top,
 * not the register save area the ABI passes to va_list, and the area sits the
 * 6 integer argument slots (0x30 bytes) below it -- the retail literal is
 * $sp+0x50 with a 0x80 frame.  Hence the -0x30.
 */
typedef void *va_list;
#define va_start(AP, LASTARG) \
    (AP = ((va_list) __builtin_next_arg(LASTARG) - 0x30))

struct _FILE;
extern int vfiprintf(struct _FILE *fp, const char *fmt, void *args);

int fiprintf(struct _FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    return vfiprintf(fp, fmt, ap);
}
