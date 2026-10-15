#include "include_asm.h"

extern int func_00282978_1(char *p);

extern void func_002B9180_1(char *p);

extern void func_00282CC0_1(char *p0, char *p1, char *p2);

extern int func_00282978_11();

extern void func_002B9180_17();

extern float func_00283CE0(int a);

extern int func_00283CF0_2(float f);

extern int func_00283D38_1(int a, int b, float f);

extern float func_002A7878_3(float a, float b);

extern void func_00282CC0_13(char *a, char *b, char *c);

extern void func_00282D30_13(char *a, char *b, float f);

extern void func_00300190_1(char *a, char *b);

extern void func_00300248(char *a, char *b);

extern int func_00282978_3(char *p);

extern void func_002B9180_6(char *p);

typedef int s32;

#ifndef RAC2_T_V4_F61E33AD9
#define RAC2_T_V4_F61E33AD9
typedef int V4_F61e33ad9 __attribute__((mode(TI)));
#endif

extern float func_00282C48_4(float f);

extern void func_00282E80(void *a, void *b);

extern void func_00282E48_3(void *a, void *b);

/*
 * Disassembled at 0x002cd220
 * Signature: int FUN_002cd220(void)
 * Behavior:
 *   - Saves RA on stack
 *   - Calls FUN_0027C540(0)
 *   - Calls FUN_0029C138()
 *   - Calls FUN_0027C660()
 *   - Restores RA from stack
 *   - Returns 0
 */
extern void func_0027C540_2(long);

extern void func_L00_0030D210(void);

extern void func_0027C660_2(void);

unsigned long long func_L00_00346A80(void);

extern void func_0027C540_9(long);

extern void func_L00_0030D2B0(void);

extern void func_0027C660_9(void);

/* Function at 0x002cd8c0: FUN_002CD8C0
 * 
 * Disassembly summary:
 * - 12 instructions, 48 bytes total
 * - Prologue: save ra, allocate 16-byte stack frame
 * - Three calls to external functions:
 *   1. FUN_0027C540(0)
 *   2. FUN_0029C450()
 *   3. FUN_0027C660()
 * - Epilogue: restore ra, return 0, deallocate stack
 * - No local variables used
 * - Return type is void-like; returns zero in v0
 * - Delay slots are_NOP/unused in this case
 */

/* External function declarations (exact addresses from task metadata) */
extern void func_0027C540_7(long);

extern void func_L00_0030D528(void);

extern void func_0027C660_7(void);

extern int D_L00_001A904C_3 __attribute__((sda));

extern int func_00341558(int x);

extern void func_0027C540_6(long);

extern void func_L00_0030D758(void);

extern void func_0027C660_6(void);

void func_0027C540_3(long);

void func_0027C660_3(void);

void func_L00_0030D808(void);

/* 
 * Function: FUN_002D2EE0
 * 
 * Disassembly summary:
 * - Saves return address on stack
 * - Calls FUN_0027C540(0) (with zero argument)
 * - Calls FUN_0029C7A0()
 * - Calls FUN_0027C660()
 * - Restores return address
 * - Returns 0
 *
 * Notes:
 * - The function follows standard MIPS ABI prologue/epilogue.
 * - Delay slots are filled with NOP or argument setup as seen in disassembly.
 * - No local variables used.
 * - All external function addresses come from the assigned externals list.
 */

extern void func_0027C540_4(long arg0);

extern void func_L00_0030D878(void);

extern void func_0027C660_4(void);

/* External function declarations as per target specification */
extern void func_0027C540_5(long);

extern void func_0027C660_5(void);

extern void func_L00_0030D9C8(void);

/* External functions declared with exact aliases from task.externals */
extern void func_0027C540_8(long param_1);

extern void func_0027C660_8(void);

extern void func_L00_0030DA38(void);

/* Target: FUN_002d5540 on MIPS (SCUS_972.68, -O2 -G0 -ffunction-sections)
 * Calls three externals: FUN_0027C540(int), FUN_0029CED8(void), FUN_0027C660(void)
 * Returns 0. Stack frame size 16 bytes. Delay slots filled.
 */
extern void func_0027C540_1(long);

extern void func_L00_0030DFE0(void);

extern void func_0027C660_1(void);

/* Frozen Batch A A03-counter-mod50-80; acknowledged claim b5e8cf533fd3cee3cb2dcdd0bf7be9549f6cc39cd932c589176487a8b170abee. */
typedef int BatchA_A03_s32;

extern BatchA_A03_s32 D_L00_001A8FE0[] __attribute__((sda));

extern BatchA_A03_s32 D_001A7BC0[] __attribute__((sda));

extern BatchA_A03_s32 D_001A7B90_1[] __attribute__((sda));

/* Guarded child-descriptor update: when the guard call returns non-zero the child is released, otherwise a packed colour byte is folded into a parent word and a four-float vector is assembled and handed to a spawn helper before three counters are advanced. */
void func_L00_0033F678(char *a0)
{
    char local[16];
    char *s1 = a0;
    char *s0;
    int a1;
    int v1;
    char *p;

    if (func_00282978_1(a0 + 10) != 0) {
        func_002B9180_1(a0);
        return;
    }
    s0 = a0 + 32;
    a1 = *(int *)(s1 + 4);
    v1 = (a1 >> 24) + *(unsigned char *)(s0 + 20);
    *(int *)(s1 + 4) = (a1 & 0x00ffffff) | (v1 << 24);
    p = s1 + 16;
    *(float *)(local + 0) = *(float *)(s1 + 32);
    *(float *)(local + 4) = *(float *)(s0 + 4);
    *(float *)(local + 8) = *(float *)(s0 + 8);
    *(int *)(local + 12) = 0;
    func_00282CC0_1(p, p, local);
    *(float *)(s0 + 8) = *(float *)(s0 + 8) - *(float *)(s0 + 12);
    *(unsigned char *)(s1 + 8) = *(unsigned char *)(s1 + 8) + *(unsigned char *)(s0 + 21);
    *(float *)(s1 + 12) = *(float *)(s1 + 12) + *(float *)(s0 + 16);
}

INCLUDE_ASM("asm/levels", func_L00_0033FFB8);

/* Calls a routine on pointer-plus-10 and, when it returns non-zero, calls a second routine with the pointer itself. */
void func_L00_003402B8(char *p)
{
    if (func_00282978_11(p + 10)) {
        func_002B9180_17(p);
    }
}

/* Guarded child-descriptor update: when the guard call returns non-zero the child is released, otherwise a packed colour byte is folded into a parent word and a four-float vector is assembled and handed to a spawn helper before three counters are advanced. */
void func_L00_00340490(char *a0)
{
    char local[16];
    char *s1 = a0;
    char *s0;
    int a1;
    int v1;
    char *p;

    if (func_00282978_1(a0 + 10) != 0) {
        func_002B9180_1(a0);
        return;
    }
    s0 = a0 + 32;
    a1 = *(int *)(s1 + 4);
    v1 = (a1 >> 24) + *(unsigned char *)(s0 + 20);
    *(int *)(s1 + 4) = (a1 & 0x00ffffff) | (v1 << 24);
    p = s1 + 16;
    *(float *)(local + 0) = *(float *)(s1 + 32);
    *(float *)(local + 4) = *(float *)(s0 + 4);
    *(float *)(local + 8) = *(float *)(s0 + 8);
    *(int *)(local + 12) = 0;
    func_00282CC0_1(p, p, local);
    *(float *)(s0 + 8) = *(float *)(s0 + 8) - *(float *)(s0 + 12);
    *(unsigned char *)(s1 + 8) = *(unsigned char *)(s1 + 8) + *(unsigned char *)(s0 + 21);
    *(float *)(s1 + 12) = *(float *)(s1 + 12) + *(float *)(s0 + 16);
}

INCLUDE_ASM("asm/levels", func_L00_00340560);

INCLUDE_ASM("asm/levels", func_L00_003406E8);

INCLUDE_ASM("asm/levels", func_L00_00340898);

INCLUDE_ASM("asm/levels", func_L00_00340CF8);

INCLUDE_ASM("asm/levels", func_L00_003411A0);

INCLUDE_ASM("asm/levels", func_L00_00341410);

/* divides one helper result by a second, multiplies a third by the quotient, quantises that product to a byte lane and the half quotient through two more helpers, bumps a byte counter, adds a scaled helper result to a float field, forwards two 16-byte sub-objects to three helpers, stores the merged byte lanes over a low halfword and calls a trailing helper when a probe on the parameter returns non-zero */
void func_L00_00341658(char *p)
{
    char *s0 = p + 32;
    char *s3 = p + 16;
    float r1, r2, r3, r4;
    int x, y;

    r1 = func_00283CE0(*(short *)(p + 10));
    r2 = func_00283CE0(*(int *)(s0 + 24));
    r1 = r1 / r2;
    r3 = func_00283CE0(*(int *)(s0 + 16));
    x = func_00283CF0_2(r3 * r1);
    y = func_00283D38_1(0, *(int *)(s0 + 20), r1 * 0.5f);
    *(char *)(p + 8) = *(unsigned char *)(p + 8) + 1;
    r4 = func_002A7878_3(0.004f, 0.012f);
    *(float *)(p + 12) = *(float *)(p + 12) + r4 * 210000.0f;
    func_00282CC0_13(s3, s3, s0);
    func_00282D30_13(s0, s0, 0.99f);
    func_00300190_1(s3, s3);
    func_00300248(s0, s0);
    *(int *)(p + 4) = y | (x << 24);
    if (func_00282978_3(p + 10) != 0) {
        func_002B9180_6(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00341798);

/* advances an index with signed delta and either wraps by count or clamps the applicable endpoint */
/* Advances a signed index, wrapping or clamping to the measured count. */
s32 func_L00_003417B8(const s32 *count, s32 index, s32 delta, s32 wrap) {
    if (wrap != 0) {
        index = (index + delta + *count) % *count;
    } else {
        index += delta;
        if (delta < 0) {
            if (index < 0) index = 0;
        } else if (index > *count - 1) {
            index = *count - 1;
        }
    }
    return index;
}

INCLUDE_ASM("asm/levels", func_L00_00341820);

/* copies two 16-byte records into locals; when the float threshold is positive it passes the difference of their third components through a float leaf returning the absolute value and, if that is below the threshold, calls one leaf with both records, otherwise the shared alternative leaf */
void func_L00_00341A88(void *a0, void *a1, float scale)
{
    V4_F61e33ad9 l0;
    V4_F61e33ad9 l1;
    float *p0;
    float *p1;
    float t;
    float u;

    l0 = *(V4_F61e33ad9 *)a0;
    l1 = *(V4_F61e33ad9 *)a1;
    p0 = (float *)&l0;
    p1 = (float *)&l1;
    if (0.0f < scale) {
        t = p0[2];
        u = p1[2];
        if (func_00282C48_4(t - u) < scale) {
            func_00282E80(p0, p1);
            return;
        }
    }
    func_00282E48_3(p0, p1);
}

INCLUDE_ASM("asm/levels", func_L00_00341B18);

INCLUDE_ASM("asm/levels", func_L00_00341D08);

INCLUDE_ASM("asm/levels", func_L00_00341FA8);

INCLUDE_ASM("asm/levels", func_L00_003425F8);

INCLUDE_ASM("asm/levels", func_L00_00342C80);

#define NOT_SDA __attribute__((section(".data")))

extern void *r1_D_0012F86C NOT_SDA;

/* rac1-decomp src/core/001138A8.c: func_001138A8 */
int func_L00_00342EB0(void) {
    return (int)r1_D_0012F86C;
}

unsigned int func_L00_00345AF0(void)
{
    return 0;
}

int func_L00_00345AF8(void) {
    return 0;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_00346680(void)
{
  func_0027C540_2(0);
  func_L00_0030D210();
  func_0027C660_2();
  return 0;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
unsigned long long func_L00_00346A80(void)
{
  func_0027C540_9(0);
  func_L00_0030D2B0();
  func_0027C660_9();
  return 0;
}

int func_L00_00346AB0(void) {
    return 0;
}

int func_L00_00346AB8(void) {
    return 0;
}

int func_L00_00346B50(void)
{
    func_0027C540(0);
    func_L00_0030DCF8();
    func_0027C660();
    return 0;
}

extern int func_00344378(int x);

/* When the global is set, derives a pointer from it plus a fixed offset, runs it through a callee and stores the result into a field of the argument. */
int func_L00_00346CD8(int *obj)
{
    if (D_L00_001A904C_3 != 0)
        obj[13] = func_00344378(D_L00_001A904C_3 + 0x39F38);
    return 0;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
long long func_L00_00346D20(void)
{
    func_0027C540_7(0LL);
    func_L00_0030D528();
    func_0027C660_7();
    return 0LL;
}

INCLUDE_ASM("asm/levels", func_L00_00346DB0);

int func_L00_00347A28(void) {
    return 0;
}

/* When a global flag is set, derives a pointer from that global plus a fixed offset, runs it through one callee and stores the callee result into a field of the argument. */
int func_L00_003485E8(int *obj)
{
    if (D_L00_001A904C_3 != 0)
        obj[13] = func_00341558(D_L00_001A904C_3 + 0x3BE08);
    return 0;
}

int func_L00_00349040(void) {
    return 0;
}

int func_L00_00349628(void) {
    return 0;
}

int func_L00_0034B470(void) {
    return 0;
}

int func_L00_0034BC90(void) {
    return 0;
}

extern int D_00138344;

extern void func_L00_0030D718(int a);

extern void func_L00_0030E010(int a);

extern void func_0027C540(int a);

extern void func_0027C660(void);

extern void func_L00_0030DCF8(void);

extern void func_L00_0030D6E8(void);

extern void func_L00_0030D8E8(void);


extern void func_L00_0030D478(int a);

int func_L00_0034C038(void)
{
    func_L00_0030D718(D_00138344);
    return 0;
}

int func_L00_0034C060(void)
{
    func_0027C540(0);
    func_L00_0030D6E8();
    func_0027C660();
    return 0;
}

/* Callback state read by the guarded update below: a source pointer the value is taken from, the
   value itself and the fallback word that decides the -1 result. */
typedef struct {
    char reserved00[0x14];
    char *source;
    int value;
    char reserved1c[0x118];
    int fallback;
} CallbackStateL001FEC40;

extern CallbackStateL001FEC40 D_L00_001FEC40;

extern void func_L00_0030D788(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C090(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030D788(flags);
    return result;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034C118(void)
{
    func_0027C540_6(0);
    func_L00_0030D758();
    func_0027C660_6();
    return 0;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
unsigned long long func_L00_0034C288(void)
{
    func_0027C540_3(0);
    func_L00_0030D808();
    func_0027C660_3();
    return 0;
}

extern void func_L00_0030D838(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C2B8(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030D838(flags);
    return result;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
unsigned long long func_L00_0034C340(void)
{
    func_0027C540_4(0);
    func_L00_0030D878();
    func_0027C660_4();
    return 0;
}

extern void func_L00_0030D8A8(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C370(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030D8A8(flags);
    return result;
}
int func_L00_0034C3F8(void)
{
    func_0027C540(0);
    func_L00_0030D8E8();
    func_0027C660();
    return 0;
}

extern void func_L00_0030DAD8(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C428(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030DAD8(flags);
    return result;
}
extern void func_L00_0030DB18(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034C4B0(void)
{
    func_0027C540(0);
    func_L00_0030DB18();
    func_0027C660();
    return 0;
}

extern void func_L00_0030DA68(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C4E0(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030DA68(flags);
    return result;
}
extern void func_L00_0030DAA8(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034C568(void)
{
    func_0027C540(0);
    func_L00_0030DAA8();
    func_0027C660();
    return 0;
}

extern void func_L00_0030D958(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034C638(void)
{
    func_0027C540(0);
    func_L00_0030D958();
    func_0027C660();
    return 0;
}

extern void func_L00_0030D988(int);

/* Guarded callback update: with the pending-input bit set, refreshes the value from its source and reports it (or the fallback's -1), otherwise reports a configured input or runs the setup call. */
int func_L00_0034C668(void)
{
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_L00_001FEC40.source + 0xE0);
        if (value != 0)
            D_L00_001FEC40.value = value;
        else if (D_L00_001FEC40.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_L00_0030D988(flags);
    return result;
}
/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
/* Implementation matching decompiled output */
long func_L00_0034C6F0(void)
{
  func_0027C540_5(0);
  func_L00_0030D9C8();
  func_0027C660_5();
  return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0034C720);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
/* Candidate function: FUN_002d3390 */
unsigned long long func_L00_0034C7F0(void)
{
    func_0027C540_8(0);
    func_L00_0030DA38();
    func_0027C660_8();
    return 0;
}

extern void func_L00_0030D678(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034C8E8(void)
{
    func_0027C540(0);
    func_L00_0030D678();
    func_0027C660();
    return 0;
}

/* When a global flag is set, derives a pointer from that global plus a fixed offset, runs it through one callee and stores the callee result into a field of the argument. */
int func_L00_0034E6F0(int *obj)
{
    if (D_L00_001A904C_3 != 0)
        obj[13] = func_00341558(D_L00_001A904C_3 + 0x3BE08);
    return 0;
}

int func_L00_0034E950(void)
{
    func_L00_0030E010(D_00138344);
    return 0;
}

int func_L00_0034E978(void)
{
    func_L00_0030E010(D_00138344);
    return 0;
}

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
long long func_L00_0034E9A0(void)
{
    func_0027C540_1(0);
    func_L00_0030DFE0();
    func_0027C660_1();
    return 0;
}

int func_L00_0034EA38(void) {
    return 0;
}

extern void func_L00_0030DBF8(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034EB78(void)
{
    func_0027C540(0);
    func_L00_0030DBF8();
    func_0027C660();
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0034EC00);

extern void func_L00_0030DB88(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034ECD0(void)
{
    func_0027C540(0);
    func_L00_0030DB88();
    func_0027C660();
    return 0;
}

int func_L00_0034ED58(void)
{
    func_L00_0030D478(D_00138344);
    return 0;
}

extern void func_L00_0030D448(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034ED80(void)
{
    func_0027C540(0);
    func_L00_0030D448();
    func_0027C660();
    return 0;
}

extern int D_L00_001C95D4;

int func_L00_0034EDB0(void) {
    D_L00_001C95D4 = -1;
    return 0;
}

int func_L00_0034EDC8(void) {
    return 0;
}

extern void func_L00_0030D4B8(void);

/* Run the three measured setup calls with an explicit long zero, then return the defined integer zero. */
int func_L00_0034EF68(void)
{
    func_0027C540(0);
    func_L00_0030D4B8();
    func_0027C660();
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_00350F20);

extern int D_L00_001C95D4;

int func_L00_00350F28(void) {
    D_L00_001C95D4 = -1;
    return 0;
}

unsigned int func_L00_00351608(void)
{
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_00357CB0);

/* A03-counter-mod50-80 freshly exact-verified under family reservation */
void func_L00_003584B0(void)
{
    if (D_L00_001A8FE0[0] == 0) {
        BatchA_A03_s32 n = D_001A7BC0[0] + 1;
        D_001A7BC0[0] = n;
        if (n % 50 == 0) {
            if (D_001A7B90_1[0]) {
                D_001A7BC0[0] = n + 10;
            }
        }
    }
}

/* Return the constant signed integer one. */
/* Returns the constant integer 1. Matches FUN_0012F940 disassembly and decompilation. */
int func_L00_00358668(void) {
    return 1;
}

INCLUDE_ASM("asm/levels", func_L00_00358670);

INCLUDE_ASM("asm/levels", func_L00_003586A0);

extern void func_00282A60(unsigned int *dst, unsigned int val, int size);

extern char D_L00_0021D140[];
extern char D_L00_0021DDB0[];
extern char D_L00_001B2370[] __attribute__((sda));

/* Zeroes the three measured work areas, the two 0xC70-byte ones and a 0x40-byte one. */
void func_L00_003588A8(void)
{
    func_00282A60((unsigned int *)D_L00_0021D140, 0, 0xC70);
    func_00282A60((unsigned int *)D_L00_0021DDB0, 0, 0xC70);
    func_00282A60((unsigned int *)D_L00_001B2370, 0, 0x40);
}

INCLUDE_ASM("asm/levels", func_L00_00358B30);

extern char *D_L00_001B1E9C __attribute__((sda));
extern char *D_L00_001B1EA0 __attribute__((sda));

/* Walks the 0x100-byte records between the two list bounds and returns the first whose 16-bit key at 0xB2 matches, or null. */
char *func_L00_00358D50(int a0, int key)
{
    char *cur;
    for (cur = D_L00_001B1E9C; cur != D_L00_001B1EA0; cur += 0x100) {
        if (*(short *)(cur + 0xB2) == key)
            return cur;
    }
    return 0;
}
