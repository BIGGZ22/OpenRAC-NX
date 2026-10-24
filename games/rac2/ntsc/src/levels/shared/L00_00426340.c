#include "include_asm.h"

extern void func_L00_00426248(int);

extern void func_L00_00425490_1(int);

extern void func_L00_00424F98(int, float *, float *);

extern void func_L00_00425118(int, float, float);

extern void func_L00_004243C0(int);

extern void func_L00_00425CD0(int);

extern void func_L00_00425560(int);

extern void func_00282F20_1(char *a0, char *a1, float f);

extern void func_00282D30_5(char *a0, char *a1, float f);

extern void func_00282CC0_4(char *a0, char *a1, char *a2);

extern char D_00189E20_6[];

extern char D_00189EA0[];

extern void func_L00_002DA0B8(char *first, char *second, char *third);

extern float func_00283B98_6(float value, float reference);

extern void func_00283410_5(char *out, char *source);

extern void func_00283678(char *object, char *out);

extern void func_L00_002D7940_1(char *p);

extern void func_L00_002EE0C0(char *p);

extern void func_L00_00427B40_1(char *p);

extern char D_L00_001B8C80_1[];

extern char D_00189E20_3[];

extern float func_00283248_1(float a, float b);

extern float func_00283C80(float a, float b);

extern float func_00282E48(char *a, char *b);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_6af85cabb56d3b41_SmallConditional52Owner {
    unsigned char unknown_00[0x7d];
    unsigned char value_7d;
    short value_7e;
    unsigned char unknown_80[6];
    short identity_86;
} Rac2Native_6af85cabb56d3b41_SmallConditional52Owner;

extern void ref_L00_00428EB0_00(char *);

extern void ref_L00_00428EB0_01(char *);

extern void func_L00_004276E8(char *);

extern void func_L00_002DA1E0(char *, float);

extern void func_L00_00426680(char *);

extern void func_0035F3A8(int, int, int);

extern void *func_00335FB0(void *);

extern void func_L00_00426560(char *);

#define SDA_OBJ __attribute__((section(".sdata")))

extern char *D_L00_001B1844 SDA_OBJ;

extern char *D_L00_001B8E10;

extern char D_L00_001ADD98[] SDA_OBJ;

extern char D_L00_001ADD38[] SDA_OBJ;

extern char D_L00_001ADD78[] SDA_OBJ;

extern char D_L00_001ADD58[] SDA_OBJ;

typedef struct Rac2Native_6af85cabb56d3b41_SmallConditional52Global {
    unsigned char unknown_00[0x2290];
    char *ptr_2290;
    int mode_2294;
    unsigned char unknown_2298[0x208];
    int identity_24a0;
} Rac2Native_6af85cabb56d3b41_SmallConditional52Global;

extern Rac2Native_6af85cabb56d3b41_SmallConditional52Global D_00189E20_20;

/* runs a fixed sequence of seven callees on one object handle, passing the address of two stack floats to the third callee and then their values to the fourth */
void func_L00_00426340(int object)
{
    float x;
    float y;

    func_L00_00426248(object);
    func_L00_00425490_1(object);
    func_L00_00424F98(object, &x, &y);
    func_L00_00425118(object, x, y);
    func_L00_004243C0(object);
    func_L00_00425CD0(object);
    func_L00_00425560(object);
}

INCLUDE_ASM("asm/levels", func_L00_004263A8);

INCLUDE_ASM("asm/levels", func_L00_004263B8);

void func_L00_00426478(void)
{
}

/* selects one of three scale factors from a flag byte in a fixed global table, builds two 16-byte stack values through two callees, then applies two transforms rooted at object+48 against two fixed global tables */
void func_L00_00426480(char *object)
{
    char local[16];
    char buf[16];
    char *p = D_00189E20_6;
    unsigned char v;

    func_00282F20_1(local, (char *)(*(int *)(p + 8848) + 224), 1.0f);
    v = *(unsigned char *)(p + 8884);
    if (v == 2) {
        func_00282D30_5(buf, local, 9.5f);
    } else if (v == 1) {
        func_00282D30_5(buf, local, 0.75f);
    } else {
        func_00282D30_5(buf, local, 1.6f);
    }
    func_00282CC0_4(object + 48, D_00189EA0, buf);
    func_00282CC0_4(object + 48, D_00189EA0 + 208, object + 48);
}

INCLUDE_ASM("asm/levels", func_L00_00426560);

/* clears the seven words of the sub-object at +0x70 (the last after a refresh call), then
   clears the flag word and the identity word-16 of the table entry and the owner's +0x7E. */
void func_L00_00426680(char *self)
{
    char *p = *(char **)(self + 0x70);
    char *q;

    *(int *)(p + 0x0) = 0;
    *(int *)(p + 0x8) = 0;
    *(int *)(p + 0x4) = 0;
    *(int *)(p + 0xC) = 0;
    *(int *)(p + 0x10) = 0;
    *(int *)(p + 0x14) = 0;
    *(int *)(p + 0x18) = 0;
    func_L00_00426560(self);
    q = *(char **)(D_L00_001B1844 + *(short *)(self + 0x84) * 0x20 + 0x1C);
    *(short *)(q + 0x26) = 0;
    *(short *)(self + 0x7E) = 0;
}

INCLUDE_ASM("asm/levels", func_L00_004266E8);

INCLUDE_ASM("asm/levels", func_L00_00426C78);

void func_L00_00427008(char *self)
{
    char *a = D_L00_001B8C80_1;
    char *b;
    short v = *(short *)(a + 0x290);

    if (v != 0) {
        *(short *)(a + 0x290) = 2;
    } else {
        *(short *)(a + 0x290) = 1;
    }
    b = D_L00_001B8C80_1;
    *(char *)(b + 0x293) = 0;
    *(float *)(b + 0x2B4) = 0.075f;
    *(float *)(b + 0x2A8) = 0.075f;
    func_L00_00426680(self);
    if (*(int *)(b + 0x264) != 0) {
        func_0035F3A8(0, 0, *(int *)(b + 0x264));
    }
}

/* when the mode halfword at +0x86 is 4 or 0x51, raises the flag halfword of the table entry
   selected by the owner's index halfword at +0x84. */
void func_L00_00427078(void)
{
    char *p = D_L00_001B8E10;
    short mode = *(short *)(p + 0x86);
    char *q;

    if (mode == 4 || mode == 0x51) {
        q = *(char **)(D_L00_001B1844 + *(short *)(p + 0x84) * 0x20 + 0x1C);
        *(short *)(q + 0x26) = 1;
    }
}

extern float func_00282DB0(char *a0, char *a1);

extern void func_00282D30(char *a0, char *a1, float f);

extern void func_00282CF0(char *a0, char *a1, char *a2);

extern float func_00282DF0(char *a0);

extern float func_002831B0(float f);

/* Restores the owner's half-turn state: it measures the two world vectors of the table
   entry, compares the angle they make with a fixed threshold, and writes the owner's
   half-word at +0x7E and the entry's flag half-word at +0x290. The block at the end
   resets the entry's two small floats and the owner's state; the two tests jump to it,
   and it needs its own copy of the entry's address. */
void func_L00_004270C0(char *owner)
{
    char tmp[16];
    char v1[16];
    char v2[16];
    char *base;
    char *b;
    float f1;
    float f2;
    float f20;
    float r;
    float s;

    if (D_00189E20_20.identity_24a0 == 0x51) {
        return;
    }
    if (D_00189E20_20.identity_24a0 == 4) {
        return;
    }
    base = D_L00_001B8C80_1;
    if (*(short *)(base + 0x290) == 0) {
        *(char *)(base + 0x293) = 0;
        *(short *)(owner + 0x7E) = 2;
        *(float *)(base + 0x2B4) = 0.018f;
        *(float *)(base + 0x2A8) = 0.018f;
        return;
    }
    f1 = func_00282DB0(base + 0x1D0, owner);
    func_00282D30(tmp, base + 0x1D0, f1);
    func_00282CF0(v1, owner, tmp);
    f2 = func_00282DB0(base + 0x1D0, D_00189E20_20.ptr_2290 + 0xC0);
    func_00282D30(tmp, base + 0x1D0, f2);
    func_00282CF0(v2, D_00189E20_20.ptr_2290 + 0xC0, tmp);
    f20 = func_00282DF0(v2);
    if (f20 == 0.0f) {
        goto small;
    }
    f20 = f20 * func_00282DF0(v1);
    if (f20 == 0.0f) {
        goto small;
    }
    r = func_00282DB0(v2, v1);
    s = func_002831B0(r / f20);
    if (1.3962636f <= 1.5707964f - s) {
        *(short *)(base + 0x290) = 0;
        *(short *)(owner + 0x7E) = 4;
        return;
    }
small:
    b = D_L00_001B8C80_1;

    *(char *)(b + 0x293) = 0;
    *(short *)(owner + 0x7E) = 2;
    *(float *)(b + 0x2B4) = 0.018f;
    *(float *)(b + 0x2A8) = 0.018f;
}

INCLUDE_ASM("asm/levels", func_L00_00427620);

INCLUDE_ASM("asm/levels", func_L00_004276E8);

unsigned int func_L00_00427AD8(void)
{
    return 0;
}

/* When the sub-object at +0x70 gains its flag word at +0x40, clears the byte at +0x89 and
   fades the block at +0x30 to a half-turn target. */
void func_L00_00427AE0(char *a0)
{
    char *p = *(char **)(a0 + 0x70) + 0xE0;

    func_L00_004276E8(a0);
    if (*(int *)(p + 0x40)) {
        *(char *)(a0 + 0x89) = 0;
        func_L00_002DA1E0(a0 + 0x30, 0.5f);
    }
}

void func_L00_00427B38(void)
{
}

/* Refreshes a child sub-structure through a setup helper, folds four float pairs (one measured against a fixed half-turn constant) through a shared two-float helper, and hands the resulting four-field block to a second helper. */
void func_L00_00427B40(char *object)
{
    float local[16];
    char *child = *(char **)(object + 112);

    child += 176;
    func_L00_002DA0B8(object + 48, object + 80, child);
    *(float *)(object + 80) = func_00283B98_6(*(float *)(object + 80), *(float *)(child + 24));
    *(int *)(object + 64) = 0;
    *(float *)(object + 68) = func_00283B98_6(*(float *)(object + 84), *(float *)(object + 92));
    *(float *)(object + 72) = func_00283B98_6(3.14159265f, *(float *)(object + 80));
    *(float *)(object + 72) = func_00283B98_6(*(float *)(object + 72), *(float *)(object + 96));
    *(int *)(object + 76) = 0;
    func_00283410_5((char *)local, object + 64);
    func_00283678(object, (char *)local);
}

extern void func_L00_002D7958_1(float *, void *, float);

/* 128-bit copies reach lq/sq only through TImode. */
typedef int TQ_00427BE8 __attribute__((mode(TI)));

/* func_L00_00427BE8: five spellings give the same 128-byte object, retail is 144 -
   both quadword destinations stay folded on $s0 here where retail holds each in a
   register. Deposited in nonmatching/levels/func_L00_00427BE8.c. */
INCLUDE_ASM("asm/levels", func_L00_00427BE8);

/* resets a sub-object reached through field +112 of its argument: two int zeros through a pointer 176 bytes in, twelve float constants (0.3/0.04/0.07/1.0/0.99) and three int zeros over offsets 40..172, then four calls (sub-object, +80, +144, the argument) and a 16-bit zero at +126 */
void func_L00_00427C78(char *self)
{
    char *q = (char *)*(int **)(self + 112) + 176;
    float *f;

    *(int *)(q + 64) = 0;
    f = (float *)*(int **)(self + 112);
    f[40] = 3.0000001192092895507812e-01f;
    f[41] = 3.0000001192092895507812e-01f;
    f[39] = 0.0f;
    f[42] = 0.0f;
    *(int *)(q + 68) = 0;
    f[43] = 0.0f;
    f[18] = 9.9000000953674316406250e-01f;
    f[15] = 3.9999999105930328369141e-02f;
    f[16] = 3.9999999105930328369141e-02f;
    f[17] = 3.9999999105930328369141e-02f;
    f[10] = 7.0000000298023223876953e-02f;
    f[11] = 1.0000000000000000000000e+00f;
    f[19] = 3.9999999105930328369141e-02f;
    f[12] = 7.0000000298023223876953e-02f;
    f[13] = 1.0000000000000000000000e+00f;
    f[14] = 7.0000000298023223876953e-02f;

    func_L00_002D7940_1((char *)f + 80);
    func_L00_002D7940_1((char *)f);
    func_L00_002EE0C0((char *)f + 144);
    func_L00_00427B40_1(self);

    *(short *)(self + 126) = 0;
}

INCLUDE_ASM("asm/levels", func_L00_00427D50);

INCLUDE_ASM("asm/levels", func_L00_00428000);

INCLUDE_ASM("asm/levels", func_L00_004282F0);

/* Reads two floats from a data block, differences them against two globals, calls a two-float helper with the difference pair, refines the returned float through a second helper, then selects one of two small state codes and clears a flag byte according to two float thresholds. */
void func_L00_004283C8(char *o)
{
    char *B = D_L00_001B8C80_1;
    char *D = D_00189E20_3;
    float *p = *(float **)(D + 8848);
    float r = func_00283C80(*(float *)(B + 344),
                      func_00283248_1(p[4] - *(float *)(B + 320),
                              p[5] - *(float *)(B + 324)));

    if (r < 0.5585054f) {
        r = func_00282E48(D + 128, B + 320);
        if (r < 8.0f) {
            *(char *)(B + 659) = 0;
            *(short *)(o + 126) = 2;
            *(float *)(B + 692) = 0.018f;
            *(float *)(B + 680) = 0.018f;
        } else {
            *(short *)(o + 126) = 4;
        }
    } else {
        *(short *)(o + 126) = 4;
    }
    *(char *)(o + 125) = 0;
}

int func_L00_004284A8(void) {
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_004284B0);

void func_L00_00428580(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00428588);

INCLUDE_ASM("asm/levels", func_L00_004287E8);

INCLUDE_ASM("asm/levels", func_L00_00428CE0);

/* runs a fixed pair of callees on the same object handle */
void func_L00_00428EB0(char *a0)
{
    ref_L00_00428EB0_00(a0);
    ref_L00_00428EB0_01(a0);
}

INCLUDE_ASM("asm/levels", func_L00_00428EE0);

INCLUDE_ASM("asm/levels", func_L00_00428F00);

/* When the global identity differs from the signed owner identity at offset 0x86 and the global mode is not 6, clear the owner byte at offset 0x7D and write signed short value 3 at offset 0x7E. */
void func_L00_00428FE8(Rac2Native_6af85cabb56d3b41_SmallConditional52Owner *owner)
{
    if (D_00189E20_20.identity_24a0 != owner->identity_86 &&
        D_00189E20_20.mode_2294 != 6) {
        owner->value_7d = 0;
        owner->value_7e = 3;
    }
}

#define MACRO_ADDR __attribute__((section(".sdata")))

extern char * r1_D_L10_0015F050 MACRO_ADDR;

extern char *r1_D_L10_0015F050 MACRO_ADDR;

/* rac1-decomp src/overlays/shared/vendor_00299AF0.c: func_L10_002F6E10 */
/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay_vendor_002e3de8.c, FUN_L10_002f5a50. */
void func_L00_00429020(int i) {
    unsigned char *q;
    q = *(unsigned char **)(r1_D_L10_0015F050 + i * 0x20 + 0x1C);
    q[0x38] = 1;
    q[0x39] = 0;
}

INCLUDE_ASM("asm/levels", func_L00_00429048);

INCLUDE_ASM("asm/levels", func_L00_004290C0);

INCLUDE_ASM("asm/levels", func_L00_004291A0);

INCLUDE_ASM("asm/levels", func_L00_00429568);

extern int D_0018C0BC;
extern int D_0018C0B4;
extern int func_L00_004290C0(char *, char *);

int func_L00_00429C18(char *self, char *target)
{
    char *entry = *(char **)(D_L00_001B1844 + *(short *)(self + 0x84) * 0x20 + 0x1C);

    if (*(unsigned char *)(entry + 0x39) != 0) {
        return -1;
    }
    if (*(unsigned char *)(entry + 0x38) != 0 && *(short *)(target + 0x86) != 0x13) {
        return 1;
    }
    if (target != 0 && *(short *)(target + 0x7E) == 0 &&
        *(unsigned char *)(target + 0x7C) >= *(unsigned char *)(self + 0x7C)) {
        return 0;
    }
    if (func_L00_004290C0(self, target) == 0) {
        return 0;
    }
    if (*(int *)(entry + 0x48) >= 0 && D_0018C0BC != *(int *)(entry + 0x48)) {
        return 0;
    }
    if (*(int *)(entry + 0x44) >= 0 && D_0018C0B4 != *(int *)(entry + 0x44)) {
        return 0;
    }
    return 1;
}

void func_L00_00429CE0(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00429CE8);

INCLUDE_ASM("asm/levels", func_L00_00429F68);

INCLUDE_ASM("asm/levels", func_L00_0042A460);

INCLUDE_ASM("asm/levels", func_L00_0042A4E8);

/* clears the word at +0x20 and the halfword at +0x26 of the table entry selected by the
   index argument. */
void func_L00_0042A500(int i)
{
    char *q = *(char **)(D_L00_001B1844 + i * 0x20 + 0x1C);

    *(short *)(q + 0x26) = 0;
    *(int *)(q + 0x20) = 0;
}

INCLUDE_ASM("asm/levels", func_L00_0042A520);

INCLUDE_ASM("asm/levels", func_L00_0042A698);

INCLUDE_ASM("asm/levels", func_L00_0042A908);

int func_L00_0042AB40(void) {
    return -1;
}

void func_L00_0042AB48(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_0042AC18);

INCLUDE_ASM("asm/levels", func_L00_0042AD98);

INCLUDE_ASM("asm/levels", func_L00_0042B038);

INCLUDE_ASM("asm/levels", func_L00_0042B840);

INCLUDE_ASM("asm/levels", func_L00_0042BA18);

/* refreshes the object, installs the block at +0x30 and returns the object. */
char *func_L00_0042BAD0(char *a0)
{
    func_00335FB0(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD98;
    return a0;
}

INCLUDE_ASM("asm/levels", func_L00_0042BC30);

char *func_L00_0042BCF8(char *a0)
{
    func_00335FB0(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD38;
    return a0;
}

char *func_L00_0042BFF8(char *a0)
{
    func_00335FB0(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD78;
    return a0;
}

INCLUDE_ASM("asm/levels", func_L00_0042C030);

char *func_L00_0042C290(char *a0)
{
    func_00335FB0(a0);
    *(char **)(a0 + 0x30) = D_L00_001ADD58;
    return a0;
}

INCLUDE_ASM("asm/levels", func_L00_0042C2C8);

/* Store the supplied unsigned 32-bit word at owner byte offset 0x 44. */
void func_L00_0042C358(int param_1, unsigned int param_2)
{
  *(unsigned int*)(param_1 + 0x44) = param_2;
  return;
}

/* Store the supplied unsigned 32-bit word at owner byte offset 0x 40. */
void func_L00_0042C360(int param_1, unsigned int param_2)
{
  *(unsigned int *)(param_1 + 0x40) = param_2;
  return;
}

INCLUDE_ASM("asm/levels", func_L00_0042C398);

/* Store the supplied unsigned 32-bit word at owner byte offset 0x 40. */
void func_L00_0042C4B8(int param_1, unsigned int param_2)
{
  *(unsigned int *)(param_1 + 0x40) = param_2;
  return;
}

/* Store the supplied unsigned 32-bit word at owner byte offset 0x 44. */
void func_L00_0042C608(int param_1, unsigned int param_2)
{
  *(unsigned int*)(param_1 + 0x44) = param_2;
  return;
}

void func_L00_0042C6C8(void)
{
}

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_0042C6D8(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_0042C760);
