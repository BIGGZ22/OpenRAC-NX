#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
typedef int TI128 __attribute__((mode(TI)));

extern u16 D_L18_001B18FC __attribute__((sda));

extern float D_L18_001B191C __attribute__((sda));

extern void *func_0029EFD0(short a);

extern float func_00283248(float a, float b);

extern void func_002A0F48(void *a);

INCLUDE_ASM("asm/levels", func_L18_00437AA0);
