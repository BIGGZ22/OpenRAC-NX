#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x1162B8-0x116CC0: the C library's stdio internals
 * (__sread, __swrite, __sseek, __sclose, ...), built with the game's
 * 2.95.3. The object starts where sprintf.c (00116248.c) ends; see the
 * note there.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern int func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern int func_00114518(int *errOut, void *a, void *b, void *c);
extern int func_00116108(int *errOut, void *a, void *b, void *c);


/* Reclaimed from a banked revert. The old note blamed retail's
   dsll32/dsra32 on a sign-extension this compiler "does not emit for an
   int-returning callee" -- right observation, wrong conclusion. The pair is
   not an extension of an int, it is the DImode->SImode narrowing GCC emits
   when a 64-bit return value is assigned to an int (`long` is 64 bits for
   this target, though word_mode is still SI -- which is why a 64-bit
   *compare* is rejected outright with "unsupported wide integer operation").
   So the callee was prototyped `long` where it is called and defined `int`
   where it is defined: an inconsistency retail could have, because these two
   functions lived in different translation units and nothing cross-checks
   them at link time. We merge the whole segment into one file, so the two
   views are reconciled with an asm-labelled alias -- func_00116108 keeps its
   int definition (it stays byte-exact; widening it is impossible anyway,
   since its own `r == -1` would become an unsupported 64-bit compare) and
   the call site goes through a declaration that spells the return type the
   way retail's header did. Emits a plain `jal func_00116108`. */
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");

int func_001162B8(void *arg0, void *arg1, void *arg2) {
    Node1E4 *s = (Node1E4 *)arg0;
    int r = func_00116108_wide((int *)s->owner,
                               (void *)(int)s->handle, arg1, arg2);
    if (r >= 0) {
        s->pos += r;
    } else {
        s->flags = (unsigned short)s->flags & 0xEFFF;
    }
    return r;
}

/* newlib's __swrite. Same cross-TU `long` prototype as func_001162B8
   above -- see that note. Here the narrowing lands on the return value
   itself: retail's trailing dsll32/dsra32 pair is func_001188C8's 64-bit
   result being handed back through this function's int return type. */
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");

int func_00116320(void *arg0, void *arg1, void *arg2) {
    char *s = (char *)arg0;
    if (*(unsigned short *)(s + 0xC) & 0x100) {
        func_00114518(*(int **)(s + 0x54), (void *)(int)*(short *)(s + 0xE),
                      (void *)0, (void *)2);
    }
    *(unsigned short *)(s + 0xC) &= 0xEFFF;
    return func_001188C8_wide(*(int **)(s + 0x54),
                              (void *)(int)*(short *)(s + 0xE), arg1, arg2);
}

/* Third of the family. This one keeps the 64-bit type all the way out --
   the return is the raw call result, and the only place retail narrows it
   is the 32-bit store to +0x50. Writing the `== -1` arm first is what puts
   the mask branch on the fallthrough path, as retail has it. */
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");

long func_001163A0(void *arg0, void *arg1, void *arg2) {
    Node1E4 *s = (Node1E4 *)arg0;
    long r = func_00114518_wide((int *)s->owner,
                                (void *)(int)s->handle, arg1, arg2);
    if (r == -1) {
        s->flags = (unsigned short)s->flags & 0xEFFF;
    } else {
        s->pos = r;
        s->flags = (unsigned short)s->flags | 0x1000;
    }
    return r;
}

extern int func_00112468(int *errOut, int arg1);

/* newlib's __sclose. It returns the close result: 2.9-ee tail-calls a
   void function that ends in a call, which retail does not have here. */
int func_00116408(void *arg0) {
    char *self = (char *)arg0;
    return func_00112468(*(int **)(self + 0x54), *(short *)(self + 0xE));
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116428);

INCLUDE_ASM("asm/nonmatchings/core_text", func_001165B8);

INCLUDE_ASM("asm/nonmatchings/core_text", func_001166FC);

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116810);

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116948);

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116B00);

INCLUDE_ASM("asm/nonmatchings/core_text", func_00116CBC);
