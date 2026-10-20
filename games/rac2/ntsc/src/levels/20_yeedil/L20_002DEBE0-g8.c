#include "include_asm.h"

/* This unit is built with -G8: every body here reaches a global through $gp.
   The small-data symbols are declared with the sda attribute, which makes the
   compiler emit the bare GPREL16 form; the type needs a size at most the -G8
   threshold, and the typedefs have to come before the externs. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

/* 0x1AEFF0 - 0x388 : the float func_L20_0039BB18 reads through $gp. */
extern float D_L20_001AEC68 __attribute__((sda));

/* The calls func_L20_0039BB18 makes. */
extern int func_00282948(void *);
extern float func_00282E80(void *, void *);
extern void func_002AA2E8(float *, float, float);
extern void func_0029F338(void *);
extern float func_002A7878(float, float);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(void *, void *, void *);
extern int func_002A77E0(int);
extern void func_002C3150(void *, void *, void *, void *, int, int, float);
extern void func_L00_002E6CF0(void *, void *);

INCLUDE_ASM("asm/levels", func_L20_0039BB18);

INCLUDE_ASM("asm/levels", func_L20_00310E38);

INCLUDE_ASM("asm/levels", func_L20_0039BEC8);

INCLUDE_ASM("asm/levels", func_L20_002DEBE0);
