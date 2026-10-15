#include "common.h"

/*
 * libgcc2 L_moddi3 (__moddi3). Not yet matching from libgcc2.c: 7/410 words, right size. Retail's frame is 0x20 larger (0x40 vs 0x20), i.e. one more stack local than this libgcc2.c revision has. Same residual as udivdi3/umoddi3.
 */

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011E860);
