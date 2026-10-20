/* Not a C row: the extent mixes a dead tail with a stack restore and an 8-byte body.
   Retail at 0x428EE0, level 00 (15 places): `sw $zero,0xC($v1)` (with $v1 never set); `nop;
   addiu $sp,$sp,0x10; nop; sw $zero,0x0($v0)` (again $v0 never set); `nop; jr $ra;
   daddu $v0,$zero,$zero`. The previous row ends at 0x428EE0 (`jr $ra; addiu $sp,$sp,0x10`) and the
   next row func_L00_00428F00 at 0x428F00 is a real function with `addiu $sp,$sp,-0x20`.
   The only complete body inside is the last 8 bytes at 0x428EF8 (`return 0`).
   Maintainer data: re-cut to 0x428EF8..0x428F00, or drop the row. */
