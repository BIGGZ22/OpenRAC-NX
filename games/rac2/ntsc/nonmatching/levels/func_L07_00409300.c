/* 0 of 4 places match - catalogue defect, not a source problem: no C can be placed here.
   The catalogue row is 172 bytes at 0x00409300, but the bytes there open with
   `addiu $sp,$sp,0x190` / `nop` - the tail of the previous function's frame teardown, the
   signature the seventh-wave brief lists as a catalogue defect.
   The real body starts 8 bytes later, at 0x00409308, and is 164 bytes:
   `addiu $sp,$sp,-0x10 ; sd $ra,0x0($sp) ; lbu $v1,0x20($a0) ...` up to
   `jr $ra` / `addiu $sp,$sp,0x10` at 0x004093A4 / 0x004093A8.
   What it does (from the bytes at +8): switches on arg0[0x20] - case 0 calls
   func_002A7750(arg0, 1, -1); case 1 takes the 0x68 state pointer and, when it is non-null,
   calls func_L00_00326048 with the state (or state+0xC when the word at $gp-0x60F0 is 2),
   the two packed colour words 0x80AABED2/0x80466464 and 0.0553f, or 0x8096AAAA/0x80466464
   and 0.0378f in the other arm.
   What unblocks it: a catalogue fix moving the row to 0x00409308 with size 164 (four places). */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L07_00409300);
