#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

typedef float f32;

extern void func_0026FF08(f32, f32, f32, f32, s32, s32);

/* clears object byte then calls the measured four-float helper when the field is nonnegative */
void func_L00_002D7140(u8 *object) {
 object[0x1d]=0;
 if (*(f32 *)(object+0x10)>=0.0f)
  func_0026FF08(1.1243411302566528f,0.005f,0.2f,0.0f,0,3);
}

INCLUDE_ASM("asm/levels", func_L00_002D71A0);

INCLUDE_ASM("asm/levels", func_L00_002D75F0);
