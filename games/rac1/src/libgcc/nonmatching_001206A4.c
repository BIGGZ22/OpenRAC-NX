#include "common.h"
/*
 * Linker fill (0xCDCDCDCD) between libgcc's dp-bit.o, which ends with the
 * word dptofp left behind when it was dead-stripped, and fp-bit.o. Not a
 * function; kept as bytes so the gap is reproduced exactly.
 */
INCLUDE_ASM("asm/nonmatchings/core_text", func_001206A4);
