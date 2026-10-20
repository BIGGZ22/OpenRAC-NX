/* func_L02_00411530 (80 bytes, 7 places, belongs in the -g8 sibling) -- NEAR MISS.
   state: 68 bytes against 80; the twelve bytes are the re-materialisation of 1.0f
   before the subtraction, and they follow from the FPR assignment.
   retail: 1.0f in $f0, the clamped value in $f1 -> the second clamp loads 0.0f into
           $f0 (destroying it), so `1.0f - f` re-materialises 1.0f (lui $at,0x3F80 ;
           mtc1 $at,$f0 ; nop ; sub.s $f0,$f0,$f1).
   ours:   the mirror image (1.0f in $f1, the value in $f0, the zero in $f2), so the
           constant stays live and the subtraction is one instruction.
   Both clamps, the store to D_L02_001B2FE0 through $gp ($gp+0x3FF0) and the
   c.olt.s operand order (constant on the left, as the recipe says) are right.
   Nine shapes tried (constant on either side of both clamps, ternary, nested
   ternary, complement in a local, bounds as named locals, negation tests): 64-84
   bytes, the same FPR split. */
