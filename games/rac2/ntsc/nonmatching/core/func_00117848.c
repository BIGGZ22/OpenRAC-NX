/* Near miss - not attempted as C.  1108 bytes at 0x00117848, in the same libc
   pocket as func_00117650 and func_001182B8 (the `001163B0.c` marker file), and
   with the same `-mastra-quad-saves` prologue: `addiu $sp,$sp,-0x40` then
   `sq $s2,0x20; sq $s1,0x10; sq $ra,0x30; sq $s0,0x0` (descending, 16-byte
   slots), so it needs its own unit with that flag rather than the object's
   `sdk` line.

   It never leaves the double's raw bits for the first half: the two `$a`
   arguments are split with the named-`s64` pattern this repository records
   (`daddu $v0,$s1,$zero; dsll32 $t0,$v0,0; dsra32 $t0,$t0,0; dsra32 $a3,$v0,0`
   - an inline cast would fold one of them to `lw`), then `lui $v1,0x8000;
   lui $v0,0x7FFF; ori $v0,$v0,0xFFFF` gives the sign and magnitude masks and
   `(high & 0x7FFFFFFF) | low` the zero test, exactly as func_001182B8.

   Calls: func_122B00 (dpmul) twice and func_122DA8 once.  Identity is not
   established - the two dpmul calls and func_122DA8 are the cheapest
   discriminators.  Adjacent func_00117650's note already made the same point
   for this pocket: pin the magic constants against a known source before
   writing C. */
