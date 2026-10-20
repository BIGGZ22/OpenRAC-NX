/* 0 of 2 places match - last line: 22_dobbo_orbit: instruction at +0x0 differs
   Not a function: twelve bytes, `addiu $sp,$sp,0x70; nop; addiu $sp,$sp,0x30`, no
   `jr $ra`, sets nothing it reads, and the twelve bytes before it are the epilogue
   of the previous function (`ld $s1; ld $ra; jr $ra; addiu $sp,$sp,0x20`). A
   split-off remnant between two entries, not C. Fix is in tools/levels.py. */
