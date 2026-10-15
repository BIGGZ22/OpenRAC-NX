#include "include_asm.h"

/* This unit is built with -G8: every body here reaches a global through $gp. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B23E8 __attribute__((sda));

extern float D_L20_001B23EC __attribute__((sda));

extern float D_L20_001B23F0 __attribute__((sda));

extern float D_L20_001B23F8 __attribute__((sda));

extern float D_L20_001B2410 __attribute__((sda));

extern float D_L20_001B2414 __attribute__((sda));

extern float D_L20_001B241C __attribute__((sda));

extern float D_L20_001B2450 __attribute__((sda));

extern float D_L20_001B2454 __attribute__((sda));

extern int D_L20_001B2AC8 __attribute__((sda));

extern void func_002A7750(char *, int, int);

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(void *dst, void *src, float x);

extern float func_00282E20(void *vec);

extern void func_00282F70(char *a, char *b);

extern void func_002C9218(char *a, char *b, char *c, float f);

extern void func_0032DA58(char *, char *, void *, float);

extern void func_002A8C28(char *, char *, int, int, float, float, float, int, int);

extern float func_00283B98(float, float);

extern float func_00283198(float);

extern void func_002A9288(char *, float);

extern float func_00283248(float, float);

extern void func_002AAA00(char *a, char *b, int c, float d, float e, float f,
                          float g);

extern void func_002AA2E8(float *, float, float);

extern void *func_0029EFD0(int);

extern void func_002B37E8(u8 *, u8 *);

extern float func_002A7940(void);

extern void func_002833F0(void *, void *);

extern float func_002A7878(float a, float b);

extern int func_00277F48(char *a, int b, char *c, int d, float f);

extern void func_L20_00444C18(char *arg0, char *arg1);

INCLUDE_ASM("asm/levels", func_L20_00449AE8);


INCLUDE_ASM("asm/levels", func_L20_0044A838);

INCLUDE_ASM("asm/levels", func_L20_0044AA88);

INCLUDE_ASM("asm/levels", func_L20_00449BB0);

INCLUDE_ASM("asm/levels", func_L20_00449FD0);
