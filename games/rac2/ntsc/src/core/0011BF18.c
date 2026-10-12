#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern void r1_func_00119840(char *);

extern int r1_D_0012FCFC;

extern char r1_D_00154E00[];

extern void func_0011BF18();

extern int r1_func_00119D88();

extern int r1_func_00119D88(int c);

extern void func_0011C7E8(const char *, ...);

/* The print core: walks fmt with the va_list, writing through the hook
   r1_D_0012FD00, and returns the count. */
extern int r1_func_0011A0A0(const char *, va_list);

/* rac1-decomp src/core/00119D88.c: func_00119DC0 */
/* deci2Putchar (libkernl.a:kprintf.o): the kprintf hook used for the
   deci2 debug console. Buffers characters into r1_D_00154E00 (a 0x80-byte
   line buffer) counted by r1_D_0012FCFC, flushing (r1_func_00119840) and
   resetting the count once it reaches 0x7E; on '\n' it also flushes
   immediately (appending the '\n' first) instead of just buffering it. */
void func_0011BF18(int c) {
    int count = r1_D_0012FCFC;

    if (count >= 0x7E) {
        r1_D_0012FCFC = 0;
        r1_D_00154E00[0x7F] = 0;
        r1_func_00119840(r1_D_00154E00);
        count = r1_D_0012FCFC;
    }
    if (c == 0xA) {
        r1_D_0012FCFC = 0;
        r1_D_00154E00[count] = c;
        r1_D_00154E00[count + 1] = 0;
        r1_func_00119840(r1_D_00154E00);
    } else {
        r1_D_0012FCFC = count + 1;
        r1_D_00154E00[count] = c;
    }
}

/* rac1-decomp src/core/00119D88.c: func_00119E70 */
/* serialPutchar (libkernl.a:kprintf.o): the printf hook used when going to
   the serial port. '\n' is expanded to a CR/LF pair (write '\r' then the
   original character); anything else is written as-is via kputchar. */
void func_0011BFC8(int c) {
    if (c == 0xA) {
        r1_func_00119D88(0xD);
        r1_func_00119D88(0xA);
    } else {
        r1_func_00119D88(c);
    }
}

INCLUDE_ASM("asm/core", func_0011C000);

INCLUDE_ASM("asm/core", func_0011C090);

INCLUDE_ASM("asm/core", func_0011C1F8);

/* rac1-decomp src/core/00119D88.c: func_0011A690 */
/*
 * A real varargs forwarder: va_start, hand fmt and the va_list to
 * r1_func_0011A0A0, va_end. Its frame is printf's (r1_func_00116078, also
 * 2.9-ee): $ra at 0 and the $5-$11 save area at 0x58, which is where ap
 * points. Exact under 2.9-ee; under the game's 2.95.3 a varargs
 * definition also spills $f12-$f18 (SIZE 72/56), which is why this was
 * once a hand-built `long args[16]` forwarder (13/56).
 */
void func_0011C7E8(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    r1_func_0011A0A0(fmt, ap);
    va_end(ap);
}
