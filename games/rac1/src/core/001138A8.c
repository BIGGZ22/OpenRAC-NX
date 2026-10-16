#include "common.h"
#include "structs.h"

/*
 * core_text object 0x1138A8-0x1138B8. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib's errno.o (__errno; the SDK's libc.a). Built with Sony's 2.9-ee
 * (Makefile.sn, EE29_CORE), like libc.a.
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;

extern void *D_0012F86C NOT_SDA;

int func_001138A8(void) {
    return (int)D_0012F86C;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_001138B4);
