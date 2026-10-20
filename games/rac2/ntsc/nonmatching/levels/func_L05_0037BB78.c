/* 0 of 1 places match - last line: 05_feltzin_system: instruction at +0x0 differs
   Not a function: twenty bytes, `addiu $sp,$sp,0x30; nop; lw $v0,0x8($v1); nop;
   addiu $sp,$sp,0x20` - no `jr $ra`, and `$v1` is read before anything sets it, so
   this is the tail of its neighbour rather than an entry. Fix is in tools/levels.py. */
