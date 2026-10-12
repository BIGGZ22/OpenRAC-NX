#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

extern void func_00335870(void);
extern void func_002A7750();
extern char D_L02_001BE200[];
extern void func_L02_003DAE60(void *, int);
extern void func_L02_003DB158(void *, int);
extern void func_L02_003DAF90(void *);
extern void func_L02_003DB030(void *);
extern void func_002AB8C0(void *, void *);
extern void func_002A0488(void *, void *, void *, void *);
extern void func_002B0990(void *, void *, int, int, float);
extern void func_002A8C28(void *, void *, int, float, float, float, int, int, int);
extern void func_L00_003D7250(void *, void *, float, int, int, int);
extern void func_002B60B0(float, float, float, float);
extern void func_002A7470(void *, int, int, int);
extern float func_002A7878(float, float);
extern float func_00283248(float, float);
extern float func_00283180(float);
extern float func_00283198(float);

typedef int TI128 __attribute__((mode(TI)));

/* Quadword copy, as the game's own macro wrote it (lq/sq through a materialised address). */
static __inline__ void qcopy(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}

INCLUDE_ASM("asm/levels", func_L02_0040A4A8);

INCLUDE_ASM("asm/levels", func_L02_0040B0F8);

/* arms one actor: raises its counter, then two setup calls */
void func_L02_0040B420(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) <= 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_002B0990(a0, q + 0x210, 5, 1, 2.5f);
    func_002A8C28(a0, a0 + 0x10, 0x4000000, 1.3f, 1.0f, 1.0f, 0, 1, 0);
}

/* starts the three effect tracks of the actor */
void func_L02_0040B4B0(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (a0[0x95] != 0) {
        func_L00_003D7250(a0, s1 + 0x290, 0.125f, 0x36F6, 0x20000000, 0x20000000);
        func_L00_003D7250(a0, s1 + 0x2A0, 0.15f, 0x1050B0, 0x30000000, 0x20000000);
        func_L00_003D7250(a0, s1 + 0x2B0, 0.15f, 0x1050B0, 0x10000000, 0x30000000);
    }
}

/* brings the actor into its armed state */
void func_L02_0040B868(u8 *a0, int a1) {
    u8 *s0 = *(u8 **)(a0 + 0x68);
    int one = 1;
    u32 v;

    *(float *)(s0 + 0x20) = (float)*(s16 *)(s0 + 0x24);
    *(u16 *)(s0 + 0x2E2) = one;
    func_002B60B0(0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    func_002A7750(a0, 3, 1);
    if (*(u8 *)(a0 + 0x43) != 2)
        func_002A7470(a0, 2, 0, 0);
    *(u32 *)(a0 + 0x98) = *(u32 *)(*(u32 *)(a0 + 0x24) + 0x10);
    *(u8 *)(s0 + 0x6A) = 0;
    *(u8 *)(s0 + 0x2E) = one;
    *(u8 *)(a0 + 0x31) = one;
    v = *(u16 *)(a0 + 0x34);
    *(u8 *)(a0 + 0x7F) = 0;
    *(u16 *)(a0 + 0x34) = (v | 0x1000) & 0xFFB8;
    *(u16 *)(s0 + 0x2E8) = a1;
    *(u16 *)(s0 + 0x88) = 0;
    if (*(s16 *)(s0 + 0x50) != 0) {
        *(u16 *)(s0 + 0x50) = 0;
        func_002A0488(a0, *(u8 *)(s0 + 0x54), *(u8 *)(s0 + 0x55), *(u8 *)(s0 + 0x56));
        *(u16 *)(s0 + 0x52) = one;
    }
}

INCLUDE_ASM("asm/levels", func_L02_0040B980);

/* sweeps the two effect tracks of the actor relative to a head position */
void func_L02_0040BAB8(u8 *a0, float *a1) {
    float v, t, h, k;
    u8 *q;

    v = func_00283248(*(float *)(a0 + 0x10) - a1[0], *(float *)(a0 + 0x14) - a1[1]);
    v = v + func_002A7878(-0.7853982448577881f, 0.7853982448577881f);
    t = func_002A7878(-2.0f, 2.0f) + 8.0f;
    q = *(u8 **)(a0 + 0x68);
    qcopy(q + 0x2C0, a1);
    h = t * func_00283180(v);
    *(float *)(q + 0x2C0) = *(float *)(q + 0x2C0) + h;
    k = func_00283198(v);
    *(float *)(q + 0x2C4) = *(float *)(q + 0x2C4) + t * k;
}

INCLUDE_ASM("asm/levels", func_L02_0040BBA0);

INCLUDE_ASM("asm/levels", func_L02_0040BBD8);

INCLUDE_ASM("asm/levels", func_L02_0040BE08);

INCLUDE_ASM("asm/levels", func_L02_0040CAF0);

/* steps the actor's state machine, then raises its counter */
void func_L02_0040CEE8(u8 *a0) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    func_002AB8C0(a0, s0 + 0x50);
    if (*(s16 *)(s0 + 0x50) == 0) {
        if (*(u32 *)(s0 + 0x1AC) == 0) {
            func_002A0488(a0, *(u32 *)(s0 + 0x1A0), *(u32 *)(s0 + 0x1A4),
                          *(u32 *)(s0 + 0x1A8));
            *(u32 *)(s0 + 0x1AC) = 1;
        }
    } else {
        *(u32 *)(s0 + 0x1AC) = 0;
    }
    if (*(u16 *)(a0 + 0x96) <= 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
}

INCLUDE_ASM("asm/levels", func_L02_0040CF68);

/* resets the actor's aim: three records and the two shared limits */
void func_L02_0040D080(u8 *a0, int a1) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    *(u16 *)(s0 + 0x1B8) = 0;
    *(float *)(s0 + 0x20) = (float)*(s16 *)(s0 + 0x24);
    func_002A7750(a0, 3, 1);
    if (*(u8 *)(a0 + 0x43) != 0)
        func_002A7470(a0, 0, 0, 10);
    *(u32 *)(a0 + 0x98) = *(u32 *)(*(u32 *)(a0 + 0x24) + 0x10);
    *(u8 *)(s0 + 0x2E) = 1;
    *(u8 *)(a0 + 0x7F) = 0;
    *(u8 *)(a0 + 0x31) = 1;
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) | 0x1000) & 0xFFB8;
    *(float *)(s0 + 0x18C) = func_002A7878(0.0f, 90.0f) * 0.01745329424738884f;
    *(u16 *)(s0 + 0x1BA) = a1;
    *(u16 *)(s0 + 0x3C) = 0;
    *(u32 *)(s0 + 0x38) = 0;
    if (*(s16 *)(s0 + 0x50) != 0) {
        *(u16 *)(s0 + 0x50) = 0;
        func_002A0488(a0, *(u32 *)(s0 + 0x1A0), *(u32 *)(s0 + 0x1A4),
                      *(u32 *)(s0 + 0x1A8));
        *(u16 *)(s0 + 0x52) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L02_0040D180);

INCLUDE_ASM("asm/levels", func_L02_0040FB80);

INCLUDE_ASM("asm/levels", func_L02_00411750);

INCLUDE_ASM("asm/levels", func_L02_00411D18);

INCLUDE_ASM("asm/levels", func_L02_004124A0);

INCLUDE_ASM("asm/levels", func_L02_00412808);

INCLUDE_ASM("asm/levels", func_L02_004129D0);

INCLUDE_ASM("asm/levels", func_L02_00412D50);

INCLUDE_ASM("asm/levels", func_L02_00413730);

INCLUDE_ASM("asm/levels", func_L02_004216F0);

INCLUDE_ASM("asm/levels", func_L02_00423520);

INCLUDE_ASM("asm/levels", func_L02_00423838);

INCLUDE_ASM("asm/levels", func_L02_0042E2F8);

INCLUDE_ASM("asm/levels", func_L02_0042E680);

INCLUDE_ASM("asm/levels", func_L02_0042E988);

INCLUDE_ASM("asm/levels", func_L02_004388A0);

/* restarts the shared effect object the level keeps at 0x1BE200 */
void func_L02_00438B28(void) {
    char *p = D_L02_001BE200;

    func_L02_003DAE60(*(void **)(p + 0x198), 0);
    func_L02_003DAE60(*(void **)(p + 0x198), 1);
    func_L02_003DB158(*(void **)(p + 0x198), 5);
    func_L02_003DB158(*(void **)(p + 0x198), 6);
    func_L02_003DAF90(*(void **)(p + 0x198));
    func_L02_003DB030(*(void **)(p + 0x198));
}

INCLUDE_ASM("asm/levels", func_L02_00438B90);

INCLUDE_ASM("asm/levels", func_L02_00439CD8);

INCLUDE_ASM("asm/levels", func_L02_0043E150);

INCLUDE_ASM("asm/levels", func_L02_00445200);

/* rac1-decomp src/core/00118A40.c: func_00119100 */
/* Takes an argument its callers pass (e.g. r1_func_00112468) and ignores
   it; an unused parameter costs no codegen. */
int func_L02_00456258(int arg0) {
    return -1;
}

extern void func_00335870(void);

void func_L02_00456AC0(void) {
    func_00335870();
}
