/* 0 of 2 places match - no C can be placed over the whole extent.
   The extent starts 0x10 early: its first four words are sw $t3,0x40($v1); nop;
   addiu $sp,$sp,0x20; nop - a tail plus padding whose registers ($t3, $v1) are never
   set here. The real body runs from 0x0036DAC0 (lui $a1,0x14 ...) and is a
   0x10-byte-shorter function; the same 16-byte lead-in was reported for other waves'
   catalogue splits. Recorded for the maintainers. */

/* body to place starts at +0x10 */