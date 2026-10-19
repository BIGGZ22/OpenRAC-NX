/* newlib sprintf, retail 0x01EC73D0 (136 bytes).
 *
 * Sets up a FILE on the stack in string-write mode (__SWR|__SSTR = 0x208),
 * hands it to vfprintf with the register save area, then terminates the string
 * at the stream's current pointer.  The file's reent pointer comes from
 * _impure_ptr, as in the kit's printf.
 *
 * Two measured details of the frame:
 *  - the unnamed arguments live in the ABI register save area at $sp+0xB0; the
 *    pinned chain's __builtin_next_arg yields the frame top ($sp+0xE0) instead
 *    (documented gap, IDIOMES.md sections 8 and 12), so the address is written
 *    out.  $sp+0xB0 is `&f + 0xB0` because f is the frame base.
 *  - the struct places `_data` at 0x54, as measured in the retail printf.
 *
 * The interval continues after the body: 0x01EC7450 is the next function
 * (return 0), already labelled.
 */
struct _reent;
struct _FILE {
    unsigned char *_p;                          /* 0x00 */
    int _r;                                     /* 0x04 */
    int _w;                                     /* 0x08 */
    short _flags;                               /* 0x0C */
    short _file;                                /* 0x0E */
    struct { unsigned char *_base; int _size; } _bf;  /* 0x10 */
    unsigned char _pad[0x3C];                   /* 0x18 */
    struct _reent *_data;                       /* 0x54 */
};

extern struct _reent *_impure_ptr;
extern int vfprintf(struct _FILE *fp, const char *fmt, void *ap);

int sprintf(char *str, const char *fmt, ...) {
    struct _FILE f;
    char *ap;
    int ret;

    f._flags = 0x208;                          /* __SWR | __SSTR */
    f._bf._base = f._p = (unsigned char *)str;
    f._bf._size = f._w = 0x7FFFFFFF;           /* INT_MAX */
    f._data = _impure_ptr;
    ap = (char *)&f + 0xB0;                    /* ABI register save area */
    ret = vfprintf(&f, fmt, ap);
    *f._p = 0;
    return ret;
}
