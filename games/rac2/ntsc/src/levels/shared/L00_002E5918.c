#include "include_asm.h"

extern void func_00284EA8(void);

typedef long s64;

extern void func_002CBC58(s64);

extern float func_00283BE0_6(float b, float a);

extern float func_00282C48_8(float t);

extern void func_00283B98_10(float a, float c);

/* calls the measured no-argument callee */
void func_L00_002E5918(void) {
 func_00284EA8();
}


INCLUDE_ASM("asm/levels", func_L00_002E6440);

/* calls the measured callee with zero */
void func_L00_002E6448(void) {
 func_002CBC58(0);
}

#define MACRO_ADDR __attribute__((section(".sdata")))

extern int r1_D_0015F564_002E6CF0 MACRO_ADDR;

 /* GetEffectTex(int, int) */

/*
 * Four parallel callback lists, each a (function, argument) pair of
 * arrays with its own count, plus a "register" and a "run them all"
 * function per list. The counts are reached with retail's one-register
 * macro form, so they are MACRO_ADDR.
 */
typedef void (*DrawCallback_r002E6CF0)(void *);

extern DrawCallback_r002E6CF0 r1_D_0018DC40_002E6CF0[];

extern void *r1_D_0018DD40_002E6CF0[];

/* rac1-decomp src/game/draw.c: func_001F49B0 */
void func_L00_002E6CF0(DrawCallback_r002E6CF0 fn, void *arg) {
    int count = r1_D_0015F564_002E6CF0;
    if (count < 0x40) {
        r1_D_0018DC40_002E6CF0[count] = fn;
        r1_D_0018DD40_002E6CF0[count] = arg;
        r1_D_0015F564_002E6CF0 = count + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002E6D40);

#define MACRO_ADDR __attribute__((section(".sdata")))

extern int r1_D_0015F564_002E6DB8 MACRO_ADDR;

 /* GetEffectTex(int, int) */

/*
 * Four parallel callback lists, each a (function, argument) pair of
 * arrays with its own count, plus a "register" and a "run them all"
 * function per list. The counts are reached with retail's one-register
 * macro form, so they are MACRO_ADDR.
 */
typedef void (*DrawCallback_r002E6DB8)(void *);

extern DrawCallback_r002E6DB8 r1_D_0018DC40_002E6DB8[];

extern void *r1_D_0018DD40_002E6DB8[];

/* rac1-decomp src/game/draw.c: func_001F49B0 */
void func_L00_002E6DB8(DrawCallback_r002E6DB8 fn, void *arg) {
    int count = r1_D_0015F564_002E6DB8;
    if (count < 0x40) {
        r1_D_0018DC40_002E6DB8[count] = fn;
        r1_D_0018DD40_002E6DB8[count] = arg;
        r1_D_0015F564_002E6DB8 = count + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002E6E08);

INCLUDE_ASM("asm/levels", func_L00_002E6E80);

#define MACRO_ADDR __attribute__((section(".sdata")))

extern int r1_D_0015F564_002E6EF8 MACRO_ADDR;

 /* GetEffectTex(int, int) */

/*
 * Four parallel callback lists, each a (function, argument) pair of
 * arrays with its own count, plus a "register" and a "run them all"
 * function per list. The counts are reached with retail's one-register
 * macro form, so they are MACRO_ADDR.
 */
typedef void (*DrawCallback_r002E6EF8)(void *);

extern DrawCallback_r002E6EF8 r1_D_0018DC40_002E6EF8[];

extern void *r1_D_0018DD40_002E6EF8[];

/* rac1-decomp src/game/draw.c: func_001F49B0 */
void func_L00_002E6EF8(DrawCallback_r002E6EF8 fn, void *arg) {
    int count = r1_D_0015F564_002E6EF8;
    if (count < 0x40) {
        r1_D_0018DC40_002E6EF8[count] = fn;
        r1_D_0018DD40_002E6EF8[count] = arg;
        r1_D_0015F564_002E6EF8 = count + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002E6F48);

INCLUDE_ASM("asm/levels", func_L00_002E73E0);

INCLUDE_ASM("asm/levels", func_L00_002E7F98);

INCLUDE_ASM("asm/levels", func_L00_002E8120);

INCLUDE_ASM("asm/levels", func_L00_002E8428);

INCLUDE_ASM("asm/levels", func_L00_002E8DD8);

extern void func_0027F128(void *a0, void *a1, void *a2);
extern char D_L00_0028C590[];

void func_L00_002E9100(void *a0, void *a1) {
 func_0027F128(a0, a1, D_L00_0028C590);
}

extern char D_L00_0028C930[];

void func_L00_002E9120(void *a0, void *a1) {
 func_0027F128(a0, a1, D_L00_0028C930);
}

extern char D_L00_0028CCD0[];

void func_L00_002E9140(void *a0, void *a1) {
 func_0027F128(a0, a1, D_L00_0028CCD0);
}

extern void func_0027F1D8(void *a0, void *a1, void *a2);
extern char D_L00_0028C590[];

void func_L00_002E9210(void *a0, void *a1) {
 func_0027F1D8(a0, a1, D_L00_0028C590);
}

INCLUDE_ASM("asm/levels", func_L00_002E94B8);

extern int func_0027C878(int);

extern void func_0027F2A8(void *, void *, void *, void *, void *, int, void *);

void func_L00_002E95C0(void *a0, void *a1, void *a2, void *a3, void *a4) {
 int t = func_0027C878(3);
 func_0027F2A8(a0, a1, a2, a3, a4, t, D_L00_0028CCD0);
}

INCLUDE_ASM("asm/levels", func_L00_002E9AA0);

INCLUDE_ASM("asm/levels", func_L00_002EA318);

extern void func_0027A9F0(void);

extern void func_0027AB40(void);

void func_L00_002EA600(void) {
 func_00284EA8();
 func_0027A9F0();
 func_0027AB40();
}

INCLUDE_ASM("asm/levels", func_L00_002EA660);

INCLUDE_ASM("asm/levels", func_L00_002EA668);

INCLUDE_ASM("asm/levels", func_L00_002EA708);

INCLUDE_ASM("asm/levels", func_L00_002EBE88);

INCLUDE_ASM("asm/levels", func_L00_002EC230);

INCLUDE_ASM("asm/levels", func_L00_002EC248);

INCLUDE_ASM("asm/levels", func_L00_002EC608);

INCLUDE_ASM("asm/levels", func_L00_002EC970);

INCLUDE_ASM("asm/levels", func_L00_002ECFE8);

INCLUDE_ASM("asm/levels", func_L00_002ED3F0);

void func_L00_002ED420(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002ED7F8);

INCLUDE_ASM("asm/levels", func_L00_002ED808);

INCLUDE_ASM("asm/levels", func_L00_002EDC90);

INCLUDE_ASM("asm/levels", func_L00_002EDCB8);

INCLUDE_ASM("asm/levels", func_L00_002EDCC8);

INCLUDE_ASM("asm/levels", func_L00_002EDCF0);

INCLUDE_ASM("asm/levels", func_L00_002EDD00);

INCLUDE_ASM("asm/levels", func_L00_002EDD28);

INCLUDE_ASM("asm/levels", func_L00_002EDD38);

INCLUDE_ASM("asm/levels", func_L00_002EE050);

INCLUDE_ASM("asm/levels", func_L00_002EE058);

INCLUDE_ASM("asm/levels", func_L00_002EE078);

INCLUDE_ASM("asm/levels", func_L00_002EE0C0);

INCLUDE_ASM("asm/levels", func_L00_002EE290);

INCLUDE_ASM("asm/levels", func_L00_002EE340);

INCLUDE_ASM("asm/levels", func_L00_002EE4D8);

INCLUDE_ASM("asm/levels", func_L00_002EE660);

/* Takes three floats, computes a value from the second and first, negates the third when that value is not positive, clamps the third against that value and its negation (recomputing it in each branch), then calls a helper with the first and the clamped third. */
void func_L00_002EF548(float a, float b, float c)
{
    float t = func_00283BE0_6(b, a);

    if (t <= 0.0f)
        c = -c;
    if (func_00282C48_8(t) < c)
        c = func_00282C48_8(t);
    else if (c < -func_00282C48_8(t))
        c = -func_00282C48_8(t);
    func_00283B98_10(a, c);
}

INCLUDE_ASM("asm/levels", func_L00_002EF688);

INCLUDE_ASM("asm/levels", func_L00_002F0498);
