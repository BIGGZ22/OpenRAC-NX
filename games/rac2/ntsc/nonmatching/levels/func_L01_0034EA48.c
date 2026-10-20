/* func_L01_0034EA48 (12 bytes, 8 places) -- CATALOGUE DEFECT, not C.
   The 12 bytes are two `addiu $sp,$sp,0x10 ; nop` pairs. The preceding body ends at
   0x34EA44 (`jr $ra ; addiu $v0,$zero,1`) and real code resumes at 0x34EA58
   (`lui $v1,0x20 ; lw $v0,-0xCAC($v1) ; ...`). No `jr $ra` in the extent:
   an extent fix in tools/levels.py. */
