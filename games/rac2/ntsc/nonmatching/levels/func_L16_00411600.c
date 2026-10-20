/* func_L16_00411600 (196 bytes, 1 place, level 16) -- Not a C row: MMI.

   0x7A020000 and 0x7C620000 are `lq $s0,0($v0)` / `sq $v1,0($v0)` (opcodes 0x1E
   and 0x1F), the 128-bit pair this cc1 never emits from C - the same reason
   func_L22_003A2988.c is filed as an ASM_FUNC candidate. Everything else is a
   two-iteration loop over the object's two 4-byte handles at state + 0x238, with
   three calls per iteration (func_0029FC20, func_00283678 at handle + 0xC0,
   func_002A0F48) and a 16-byte copy of the sub-object at handle + 0x10 through
   the lq/sq pair; the head prologue also copies two floats from the object
   (+0xF8, +0x48) into the state at +0x1F0 and calls func_0029FCC0 and
   func_002A0F48 once before the loop. */
