#include "common.h"

/*
 * Linker fill (0xCDCDCDCD) between libgcc's fixunsdfdi and floatdidf modules. Not a function; kept as bytes so the gap is reproduced exactly.
 */

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011E7C4);
