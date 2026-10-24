#include "include_asm.h"

#define MACRO_ADDR __attribute__((section(".sdata")))

typedef int TI128 __attribute__((mode(TI)));

extern int func_2B3760(int);

extern unsigned char D_L03_00189E20[];

extern int r1_D_L02_0015F04C MACRO_ADDR;

extern int r1_D_L02_00169670[];

extern int func_2EEC20(void *);

extern void func_331270(void *);

extern TI128 D_001A8F20;

extern void func_2EEFB0(void *, void *, void *);

extern void func_2EF020(void *, float);

extern void func_2ECA50(unsigned char *, int, int, int);

extern int D_001AEF60 __attribute__((sda));

extern float func_2EF158(void *, void *);

extern void func_2B8300(int, int, void *);

extern int func_419B10(void *, void *, int, void *, void *, void *, float, float);

extern void func_32D0F0(void *, int, float);

extern float func_2EF670(float, float);

INCLUDE_ASM("asm/levels", func_L03_002AB5C8);

INCLUDE_ASM("asm/levels", func_L03_002AD540);

INCLUDE_ASM("asm/levels", func_L03_002ADC00);

void func_L03_002ADCA0(void) {
    unsigned char *g = D_L03_00189E20;

    if (*(short *)(g + 0x6BC) != 0 && *(int *)(g + 0x660) == 0)
        *(float *)(*(unsigned char **)(g + 0x2290) + 0xF4) = *(float *)(g + 0x6B0);
}

INCLUDE_ASM("asm/levels", func_L03_002B0958);

INCLUDE_ASM("asm/levels", func_L03_002B8D10);

INCLUDE_ASM("asm/levels", func_L03_002BC430);

/* rac1-decomp src/overlays/shared/camera_001FC308.c: func_L02_001FC308 */
/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/camera/001fc008.c, FUN_L02_001fc008. */
void func_L03_002D84F0(int x) {
    int i;

    if (r1_D_L02_0015F04C < 16) {
        for (i = 0; i < r1_D_L02_0015F04C; i++) {
            if (x == r1_D_L02_00169670[i]) {
                return;
            }
        }
        r1_D_L02_00169670[r1_D_L02_0015F04C++] = x;
    }
}

INCLUDE_ASM("asm/levels", func_L03_002EC508);

INCLUDE_ASM("asm/levels", func_L03_002ECF08);

INCLUDE_ASM("asm/levels", func_L03_002ED2F8);

INCLUDE_ASM("asm/levels", func_L03_002EFB60);

INCLUDE_ASM("asm/levels", func_L03_003245B0);

INCLUDE_ASM("asm/levels", func_L03_00326458);

INCLUDE_ASM("asm/levels", func_L03_003268C0);

INCLUDE_ASM("asm/levels", func_L03_00329EF0);

INCLUDE_ASM("asm/levels", func_L03_0032AB20);

INCLUDE_ASM("asm/levels", func_L03_00339128);

void func_L03_00339268(unsigned char *a0) {
    if (*(short *)(a0 + 0xA) == -1)
        return;
    if (func_2EEC20(a0 + 0xA) == 0)
        return;
    func_331270(a0);
}

INCLUDE_ASM("asm/levels", func_L03_003564C0);

INCLUDE_ASM("asm/levels", func_L03_0038A6D8);

INCLUDE_ASM("asm/levels", func_L03_0038AB30);

/* func_L03_0038B238 is defined in L03_002AB5C8-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L03_0038B2E8);

void func_L03_0038B390(unsigned char *a0, int a1, int a2, int a3) {
    unsigned char *p = a0;
    int v = a3;

    func_2ECA50(a0, a1, a2, 0x80);
    *(int *)(p + 0x4C) = v;
    *(int *)(p + 0x48) = v;
    *(int *)(p + 0x44) = v;
    *(int *)(p + 0x40) = v;
    *(int *)(p + 0x50) = 0;
    *(int *)(p + 0x54) = 0;
    *(int *)(p + 0x58) = 0;
    *(float *)(p + 0x5C) = 1.0f;
    *(float *)(p + 0x60) = 1.0f;
    *(int *)(p + 0x64) = 0;
    *(float *)(p + 0x68) = 1.0f;
    *(float *)(p + 0x6C) = 1.0f;
    *(long long *)(p + 0x70) = 5;
}

INCLUDE_ASM("asm/levels", func_L03_0038B408);

INCLUDE_ASM("asm/levels", func_L03_0038B4E8);

INCLUDE_ASM("asm/levels", func_L03_0038B5A8);

INCLUDE_ASM("asm/levels", func_L03_0038B5D8);

INCLUDE_ASM("asm/levels", func_L03_0038BCC0);

void func_L03_00393228(unsigned char *a0) {
    unsigned char *g = D_L03_00189E20;

    if (*(unsigned char *)(g + 0x22B4) == 0)
        if (func_2EF158(g + 0x80, a0 + 0x10) < 1.7f)
            func_2B8300(1, 0x3F, a0);
}

INCLUDE_ASM("asm/levels", func_L03_003A40A0);

int func_L03_003A4978(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003C1BF0);

INCLUDE_ASM("asm/levels", func_L03_003C3220);

float func_L03_003C5AB8(unsigned char *);

int func_L03_003C41B8(unsigned char *a0, int a1, int a2) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    float f = 0.0f;
    float out;
    int r;

    if (a2 != 0)
        f = func_L03_003C5AB8(a0);
    r = func_419B10(*(void **)(s0 + 0x220), a0 + 0x10, a1, &out, s0 + 0x368, s0 + 0x36C, f,
                    *(float *)(s0 + 0x110));
    func_32D0F0(a0, a1, out);
    return r;
}

INCLUDE_ASM("asm/levels", func_L03_003C4240);

INCLUDE_ASM("asm/levels", func_L03_003C4368);

INCLUDE_ASM("asm/levels", func_L03_003C4468);

INCLUDE_ASM("asm/levels", func_L03_003C4578);

INCLUDE_ASM("asm/levels", func_L03_003C4800);

/* func_L03_003C4A10 is defined in L03_002AB5C8-g8.c (it reads $gp data). */

INCLUDE_ASM("asm/levels", func_L03_003C4BD8);

INCLUDE_ASM("asm/levels", func_L03_003C4D40);

/* The level's entity table base, the group count and the table of group id lists (in the
   resident image: the list of group g ends after its first entry with bit 15 set). */
extern unsigned char *D_L03_001B2A1C __attribute__((sda));

extern int D_L03_001B2A38[] __attribute__((sda));

extern unsigned short *D_L03_001FB5A0[];

/* The part of the object's private state (at +0x68) that these walks read: eight group ids at
   +0x200 and the id of an entity at +0x220. */
typedef struct {
    unsigned char pad0[0x200];
    int ids[8];
    unsigned int ref220;
} L03GroupState;

extern unsigned int func_L03_004197A8(unsigned char *);

extern int func_L03_00419BD8(unsigned int, void *, void *);

extern float func_00282E80(void *, void *);

/* Returns the entity of class 0xEF9 nearest to the object, within 15.0, among its eight groups
   (func_L03_004197A8 and func_L03_00419BD8 must both accept it), or 0. */
unsigned char *func_L03_003C4F58(unsigned char *a0) {
    L03GroupState *o = *(L03GroupState **)(a0 + 0x68);
    unsigned short *q;
    unsigned char *e;
    unsigned char *best = 0;
    float bestd = 15.0f;
    float d;
    int i;

    for (i = 0; i < 8; i++) {
        for (q = D_L03_001FB5A0[o->ids[i]];
             (o->ids[i] & 0xFF) != 0xFF && o->ids[i] <= D_L03_001B2A38[0] && q != 0 &&
             ((short)q[-1] >= 0 || q == D_L03_001FB5A0[o->ids[i]]);
             q++) {
            e = D_L03_001B2A1C + ((*q & 0x7FFF) << 8);
            if (e == 0)
                break;
            if (*(short *)(e + 0xAA) == 0xEF9) {
                if (func_L03_004197A8(e)) {
                    if (func_L03_00419BD8(o->ref220, a0 + 0x10, e + 0x10)) {
                        d = func_00282E80(a0 + 0x10, e + 0x10);
                        if (d < bestd) {
                            bestd = d;
                            best = e;
                        }
                    }
                }
            }
        }
    }
    return best;
}

extern unsigned int func_L03_003CAAC8(unsigned char *, unsigned char *);

/* Returns the first entity of class 0x97A, in the object's eight groups, that
   func_L03_003CAAC8 finds near the object or near the player, or 0. The group table is reached
   through a local pointer: with its address computed once outside the loops, the walk's other
   constants (0x97A, 0xFF, the player's address) all fit loop.c's budget for moving invariants
   out of the walk, as in retail. */
unsigned char *func_L03_003C5128(unsigned char *a0) {
    L03GroupState *o = *(L03GroupState **)(a0 + 0x68);
    unsigned short **tbl = D_L03_001FB5A0;
    unsigned short *q;
    unsigned char *e;
    int i;

    for (i = 0; i < 8; i++) {
        for (q = tbl[o->ids[i]];
             (o->ids[i] & 0xFF) != 0xFF && o->ids[i] <= D_L03_001B2A38[0] && q != 0 &&
             ((short)q[-1] >= 0 || q == tbl[o->ids[i]]);
             q++) {
            e = D_L03_001B2A1C + ((*q & 0x7FFF) << 8);
            if (e == 0)
                break;
            if (*(short *)(e + 0xAA) == 0x97A) {
                if (func_L03_003CAAC8(e, a0 + 0x10))
                    return e;
                if (func_L03_003CAAC8(e, D_L03_00189E20 + 0x80))
                    return e;
            }
        }
    }
    return 0;
}

extern int func_L03_003CA138(unsigned char *);

/* Returns the entity of class 0xE87 or 0xE88 in state 1 nearest to the object, within 14.0,
   among its eight groups, or 0. */
unsigned char *func_L03_003C52D0(unsigned char *a0) {
    L03GroupState *o = *(L03GroupState **)(a0 + 0x68);
    unsigned short *q;
    unsigned char *e;
    unsigned char *best = 0;
    float bestd = 14.0f;
    float d;
    int i;

    for (i = 0; i < 8; i++) {
        for (q = D_L03_001FB5A0[o->ids[i]];
             (o->ids[i] & 0xFF) != 0xFF && o->ids[i] <= D_L03_001B2A38[0] && q != 0 &&
             ((short)q[-1] >= 0 || q == D_L03_001FB5A0[o->ids[i]]);
             q++) {
            e = D_L03_001B2A1C + ((*q & 0x7FFF) << 8);
            if (e == 0)
                break;
            if (*(unsigned short *)(e + 0xAA) == 0xE87 || *(unsigned short *)(e + 0xAA) == 0xE88) {
                if (func_L03_003CA138(e)) {
                    d = func_00282E80(a0 + 0x10, e + 0x10);
                    if (d < bestd) {
                        bestd = d;
                        best = e;
                    }
                }
            }
        }
    }
    return best;
}

INCLUDE_ASM("asm/levels", func_L03_003C5488);

INCLUDE_ASM("asm/levels", func_L03_003C5668);

/* func_L03_003C58F0 is defined in L03_002AB5C8-g8.c (it reads the group count through $gp). */

float func_L03_003C5AB8(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)(a0 + 0x68);
    float x = func_2EF670(*(float *)(p + 0x1DC), 1.5f);
    float s;

    if (x > 1.4959966f)
        x = 1.4959966f;
    x = x * 0.2f;
    s = (float)*(int *)(p + 0x360);
    return x * s;
}
