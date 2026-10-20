/* from src/game/0027F128.c: 16-byte near miss; 2 instructions differ.
   Retail converts in place (cvt.w.s $f12,$f12) and reads the word back from
   the same register; this cc1 always converts into a fresh FPR and moves the
   word through a GPR (cvt.w.s $f0,$f12 ; mfc1 $v0,$f0). */
int func_00283CF0(f32 a0) { f32 b = a0; return (int)b; }
