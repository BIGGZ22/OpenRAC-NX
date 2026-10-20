/* from src/game/0027F128-g8.c: 16-byte near miss; the candidate is 12 bytes.
   Same wall as func_002828B8: this cc1 moves the jr $ra into the delay slot
   of the gp-relative lwc1, retail leaves a nop there.  The constant is
   D_001A791C (the gp offset -0x76D4). */
typedef float f32;

extern float D_001A791C __attribute__((sda));

f32 func_00282938(f32 a0) { return D_001A791C * a0; }
