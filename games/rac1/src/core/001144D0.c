#include "common.h"
#include "structs.h"

/*
 * core_text object 0x1144D0-0x114518. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib's locale.o (the SDK's libc.a): _localeconv_r and localeconv.
 * The linker dead-stripped the unreferenced _setlocale_r and setlocale
 * down to their last word each (func_001144D0, func_001144E8). Built
 * with Sony's 2.9-ee (Makefile.sn, EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);

LINKER_REMNANT("asm/remnants/core_text", func_001144D0);

extern char D_00152470[];

/* arg0 unused: retail loads it into $a0 for this call too but the
   function body never reads it. */
void *func_001144D8(void *arg0) {
    return D_00152470;
}

LINKER_REMNANT("asm/remnants/core_text", func_001144E8);

void *func_001144F0(void) {
    return func_001144D8(D_0012F86C);
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00114514);
