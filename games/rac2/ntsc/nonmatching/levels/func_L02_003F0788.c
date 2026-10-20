/* 0 of 8 places match - last line: 02_maktar_nebula: instruction at +0x0 differs
   Not a function: sixteen bytes, `addiu $sp,$sp,0x50; nop; jr $ra; nop`. It does
   return, but it deallocates a frame it never allocated, so the row is the tail of
   the function before it (the brief's `addiu $sp,$sp,+N ; nop` signature).
   Fix is in tools/levels.py. */
