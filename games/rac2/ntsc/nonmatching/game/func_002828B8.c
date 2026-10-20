/* from src/game/0027F128-g8.c: 16-byte near miss; the candidate is 12 bytes.
   The gp-relative float constant (lwc1 $f0,-0x76E0($gp)) needs the -G8
   sibling unit and a sized sda declaration, and the multiplication
   (mul.s $f0,$f0,$f12 in the jr $ra delay slot) comes out right; what is
   missing is the nop retail leaves in the load's delay slot.  This cc1 moves
   the jr $ra into that slot instead (lwc1 ; jr $ra ; mul.s), so the body is
   four bytes short.  The same wall holds for func_002828F0 (offset
   -0x76DC), func_00282928 (-0x76D8) and func_00282938 (-0x76D4). */
typedef float f32;

extern float D_001A7910 __attribute__((sda));

f32 func_002828B8(f32 a0) { return D_001A7910 * a0; }
