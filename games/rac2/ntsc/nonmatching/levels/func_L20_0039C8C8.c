/* Not matched (worker 06, level-20 wave of 2026-10-20): not attempted
   in depth.

   The body has no 128-bit copy and no `$gp` access, so neither of the
   two walls of this level is in the way; it is 836 bytes with a frame
   of 0xD0, ten callee-saved registers and eight saved float registers
   (f21..f29), and it calls func_00282CF0, func_00282920, func_00282CC0
   and others.

   This body belongs in the `-G8` unit?  No: it reaches no global
   through `$gp`. */
