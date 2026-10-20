/* from src/game/0027F128.c: 16-byte near miss; the candidate is 32 bytes.
   Retail keeps the truncated integer in the FPU (cvt.w.s $f0,$f12 ;
   cvt.s.w $f0,$f0) and subtracts it from the argument; this cc1 moves the
   integer through a GPR (mfc1 $v0,$f0 ; mtc1 $v0,$f0) before cvt.s.w. */
f32 func_00283CB8(f32 a0) { f32 b = (f32)(int)a0; return a0 - b; }
