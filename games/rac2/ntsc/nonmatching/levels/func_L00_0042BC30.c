/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x0 differs
   Not a function: twelve bytes, `addiu $sp,$sp,0x20; nop; addiu $sp,$sp,0x20` - no
   `jr $ra` and no register traffic. A split-off remnant of the function before it.
   Fix is in tools/levels.py. */
