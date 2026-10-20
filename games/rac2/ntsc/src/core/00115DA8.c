#include "common.h"

/* The unit saves $ra in a TImode slot (`sq`), so it is built with the
   -mastra-quad-saves profile. Variadic formatted print into a caller buffer:
   a local stdio-like FILE is filled with the buffer, INT_MAX as the write
   count and capacity, the 0x208 flag word and the object at +0x54, handed to
   func_00118CC8 with the format and the register save area (two named
   arguments, so 0x30 of area sits below the stack arguments), and the
   advanced buffer pointer is terminated. */
typedef struct File_00115DA8 {
    char *p;                       /* 0x00 */
    int r;                         /* 0x04 */
    int w;                         /* 0x08 */
    short flags;                   /* 0x0C */
    short file;                    /* 0x0E */
    char *base;                    /* 0x10 */
    int size;                      /* 0x14 */
    char pad0[0x54 - 0x18];
    void *data;                    /* 0x54 */
} File_00115DA8;

extern void *D_00133E74;

extern int func_00118CC8(void *arg0, char *arg1, void *arg2);

int func_00115DA8(char *arg0, char *arg1, ...) {
    File_00115DA8 f;
    int result;

    f.p = arg0;
    f.w = 0x7FFFFFFF;
    f.flags = 0x208;
    f.base = arg0;
    f.size = 0x7FFFFFFF;
    f.data = D_00133E74;
    result = func_00118CC8(&f, arg1, (char *)__builtin_next_arg(arg1) - 0x30);
    *f.p = '\0';
    return result;
}
