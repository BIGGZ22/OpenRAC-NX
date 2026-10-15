#include "common.h"

/*
 * libgcc2 L_udivdi3 and L_umoddi3. One stub object, because nothing between
 * them is compiled from source yet.
 *
 * Retail's code for both is Sony's prebuilt libgcc.a (the one every EE
 * compiler directory in the toolchain mirrors ships): its _udivdi3.o and
 * _umoddi3.o match retail byte for byte. Our libgcc2.c compiles to the same
 * instructions apart from the frame: Sony's objects reserve stack that no
 * instruction touches (udivdi3 0x10 where we have none, umoddi3 0x30 where
 * we have 0x10, and __moddi3 likewise). Every libgcc2.c revision from
 * 1999-06 to 1999-11 compiles identically here, and no flag tried moves the
 * frame, so the difference is the compiler build (Sony's Linux cc1 against
 * the Windows 2.9-ee-991111b/r4 in the mirrors), not the source.
 */

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011EF28);

INCLUDE_ASM("asm/nonmatchings/core_text", func_0011F4F8);
