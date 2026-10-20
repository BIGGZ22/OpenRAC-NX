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

/* func_002A8C28 returns a value here: retail keeps $v0 live after the call,
   and only a non-void prototype gives the flag word $v1 instead of $v0. */
extern int func_002A8C28(char *, char *, int, int, float, float, float, int, int);

extern float func_00283B98(float, float);

extern float func_00283198(float);

extern void func_002A9288(char *, float);

extern float func_00283248(float, float);

extern float func_002AAA00(char *a, char *b, int c, float d, float e, float f,
                            float g);

extern float func_002AA2E8(float *, float, float);

extern void *func_0029EFD0(int);

extern void func_002B37E8(u8 *, u8 *);

extern float func_002A7940(void);

extern void func_002833F0(void *, void *);

extern float func_002A7878(float a, float b);

extern int func_00277F48(char *a, int b, char *c, int d, float f);

extern void func_L20_00444C18(char *arg0, char *arg1);

extern char D_L20_001B9FC0[] __attribute__((nosda));

void func_L20_0044AA88(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *p = arg0 + 0xF8;
    float a = *(float *)(state + 0x140) - *(float *)(arg0 + 0x10);
    float b = *(float *)(state + 0x144) - *(float *)(arg0 + 0x14);
    float c = D_L20_001B2450 * 4.84813745e-06f;
    float d = D_L20_001B2454 * 0.000290888245f;
    float r;

    r = func_00283248(a, b);
    func_002AAA00(p, state + 0x1A4, 0, r, c, c, d);
    d = d * 4.0f;
    func_002AA2E8(arg0 + 0x0F0, *(float *)(state + 0x1A8), d);
    *(float *)(state + 0x1A8) = 0.0f;
    func_002AA2E8(arg0 + 0x0F4, *(float *)(state + 0x1AC), d);
    *(float *)(state + 0x1AC) = 0.0f;
}

void func_L20_00449AE8(char *arg0) {
    TI128 v;

    func_00282CF0(&v, D_L20_001B9FC0, arg0 + 0x10);
    func_00282F20(&v, &v, 1.0f);
    func_0032DA58(arg0, arg0 + 0x10, &v, *(float *)(arg0 + 0x18) - 0.5f);
    func_002A8C28(arg0, arg0 + 0x10, 0x10000, 0, 6.0f, D_L20_001B23EC, 2.0f,
                  1, 0);
    *(int *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(arg0, 1, -1);
}

INCLUDE_ASM("asm/levels", func_L20_0044A838);

INCLUDE_ASM("asm/levels", func_L20_00449BB0);

INCLUDE_ASM("asm/levels", func_L20_00449FD0);

INCLUDE_ASM("asm/levels", func_L20_0044B100);

/* func_L20_0044A658 is a near miss: nonmatching/levels/func_L20_0044A658.c */

INCLUDE_ASM("asm/levels", func_L20_0044A658);

/* func_L20_0044A8F0 is a near miss: nonmatching/levels/func_L20_0044A8F0.c */

INCLUDE_ASM("asm/levels", func_L20_0044A8F0);

/* func_L20_00449338 is a near miss: nonmatching/levels/func_L20_00449338.c */

INCLUDE_ASM("asm/levels", func_L20_00449338);
