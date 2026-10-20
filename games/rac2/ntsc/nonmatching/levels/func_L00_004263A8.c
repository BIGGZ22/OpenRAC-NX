/* Not a C row: the 16-byte extent is the previous function's stack restore plus an 8-byte body.
   Retail at 0x4263A8, level 00 (all 27 places): `addiu $sp,$sp,0x10; nop; jr $ra;
   daddu $v0,$zero,$zero`. The first 8 bytes are the delay-slot tail of the previous row (which ends
   at 0x4263A8 with `jr $ra; addiu $sp,$sp,0x20` before it); the last 8 bytes ARE a complete body
   (`return 0`), and the next row func_L00_004263B8 starts a real function at 0x4263B8 whose own
   prologue is `addiu $sp,$sp,-0x30` at +8. So the true 8-byte body is at 0x4263B0, and a C form
   would be `int f(void) { return 0; }` - but the extent as catalogued cannot be produced by any C.
   Maintainer data: re-cut the row to 0x4263B0..0x4263B8. */
