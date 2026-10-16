#include "common.h"
#include "records.h"
#include "structs.h"

/*
 * cmecpu.cpp in the original source; text 0x1EE9F8-0x1EFD70.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];

ASM_FUNC("asm/handwritten/text", func_001EE9F8);
