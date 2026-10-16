#include <stdarg.h>
#include "common.h"
#include "structs.h"

/*
 * core_text object 0x116248-0x1162B8: the C library's sprintf.c. Retail
 * leaves no linker fill after it (it ends 8-byte aligned), so the object
 * boundary at 0x1162B8 comes from the code: sprintf matches only under
 * Sony's 2.9-ee (Makefile.sn, EE29_CORE), and the stdio functions after
 * it (001162B8.c) only under the game's 2.95.3 (2.9-ee turns __sclose
 * into a tail call, which retail's is not).
 */

extern void *D_0012F86C NOT_SDA; /* _impure_ptr */

/* newlib's FILE (struct __sFILE), 0x58 bytes. */
typedef struct {
    unsigned char *_p;          /* 0x00 */
    int _r;                     /* 0x04 */
    int _w;                     /* 0x08 */
    short _flags;               /* 0x0C */
    short _file;                /* 0x0E */
    unsigned char *_bf_base;    /* 0x10 */
    int _bf_size;               /* 0x14 */
    char _pad18[0x54 - 0x18];
    void *_data;                /* 0x54 */
} NewlibFile;
extern int func_001170A0(void *, const char *, va_list);

/* sprintf: vfprintf into a string FILE of unbounded size, then terminate
   the string. */
int func_00116248(char *str, const char *fmt, ...) {
    int ret;
    va_list ap;
    NewlibFile f;

    f._flags = 0x208;
    f._bf_base = f._p = (unsigned char *)str;
    f._bf_size = f._w = 0x7FFFFFFF;
    f._data = D_0012F86C;
    va_start(ap, fmt);
    ret = func_001170A0(&f, fmt, ap);
    va_end(ap);
    *f._p = 0;
    return ret;
}
