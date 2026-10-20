/* func_L01_00337B80 (12 bytes, 5 places) -- CATALOGUE DEFECT, not C.
   0x337B80 opens on `addiu $sp,$sp,0x80 ; nop`: the previous body's epilogue runs to
   0x337B7C (`jr $ra ; addiu $sp,$sp,0x70`), and the next bytes `addiu $sp,$sp,0x20 ; nop`
   are the same padding shape while real code resumes at 0x337B90
   (`daddu $v0,$a0,$zero ; beqz $a3,...`). No `jr $ra` in the extent: an extent fix in
   tools/levels.py, not a C function. */
