#include "include_asm.h"

/* This unit is built with -G8: every body here reaches a global through $gp.
   The small-data symbols are declared with the sda attribute, which makes the
   compiler emit the bare GPREL16 form; the type needs a size at most the -G8
   threshold, and the typedefs have to come before the externs. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef signed long long s64;
typedef float f32;

/* 0x1AEFF0 + 0xF10 : the level word func_L20_002B93E0 reads through $gp. */
extern s32 D_L20_001AF000 __attribute__((sda));

/* 0x1AEFF0 - 0x7C60 : the word the retail reads both absolutely and through
   $gp, so it needs two names: the sda one is the $gp form, the plain one the
   absolute form.  A plain extern is the absolute form here: marking it nosda
   makes cc1 keep the address in a register of its own and the load does not
   match the retail any more (see nonmatching/levels/func_L20_002B93E0.c). */
extern s32 D_001A7390 __attribute__((sda));
extern s32 D_001A7390_1;

/* 0x1AEFF0 - 0x72F8 : the float the retail copies through $gp. */
extern float D_001A7CF8 __attribute__((sda));

/* 0x1AEFF0 - 0x7660 : the word the retail reads absolutely (plain extern, see
   the note on D_001A7390_1 above). */
extern s32 D_001A79F0;

/* 0x140000 - 0x69A2 : the byte the retail reads absolutely. */
extern u8 D_0013965E __attribute__((nosda));

extern char D_00189E20[];
extern char D_00189EA0[];
extern char D_0018C958[];
extern float D_002683EC __attribute__((nosda));

/* The calls func_L20_002B93E0 makes. */
extern void func_L00_002D24E8(void);
extern void func_L18_002D0C00(void);
extern void func_L00_002ACA08(void);
extern void func_L00_002ACF88(char *, char *, char *);
extern void func_L00_002B8930(void);
extern s32 func_00282978(s32);
extern void func_L20_002D7730(s32, s32);
extern s32 func_00288E18(s32, s32);
extern void func_L20_002DEBE0(void);
extern void func_L00_002AE0C8(void);
extern void func_L00_002B9CA8(void);
extern void func_L00_002A5368(void);
extern void func_L00_002A9600(void);
extern void func_L00_002A7958(void);
extern void func_L00_002A7748(void);
extern void func_L00_002A5840(float *);
extern void func_L00_002A5CB0(void);
extern void func_L00_002A6008(void);
extern void func_002E5CF8(void *, void *, void *);
extern void func_L00_003080D0(s64);
extern void func_L18_00428100(s32);
extern void func_L00_002A9460(void);
extern void func_002A677C(s32);

INCLUDE_ASM("asm/levels", func_L20_002B93E0);

INCLUDE_ASM("asm/levels", func_L20_002C5D00);
