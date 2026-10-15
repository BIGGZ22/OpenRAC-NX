#include "include_asm.h"

extern char D_L14_001BF980[];

extern void func_00282F20_2(void *out, void *in, float scale);

extern void func_00282CC0_6(void *out, void *in1, void *in2);

extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);

extern void func_00282D30_6(void *out, void *in, float scale);

INCLUDE_ASM("asm/levels", func_L14_003F4F98);

INCLUDE_ASM("asm/levels", func_L14_00409B20);

/* Read the word at offset 0x 18 through the pointer stored at owner offset 0x 68. */
unsigned int func_L14_004137F8(unsigned int param_1)
{
    unsigned int inner_ptr;
    unsigned int result;

    inner_ptr = *(unsigned int *)(param_1 + 0x68);
    result = *(unsigned int *)(inner_ptr + 0x18);
    return result;
}

/* Normalises and combines two temporary vectors from a state block, dispatches on the combined result and conditionally scales the output vector. */
void func_L14_00413CD8(char *self, char *p1, int p2)
{
    float a[4];
    float b[4];

    func_00282F20_2(a, self + 224, 0.1f);
    func_00282F20_2(b, self + 224, -3.0f);
    func_00282CC0_6(a, a, self + 16);
    func_00282CC0_6(b, b, self + 16);

    if (func_002761F8(a, b, 2, self, 0))
        func_00282F20_2(p1, D_L14_001BF980, 1.0f);
    else
        func_00282F20_2(p1, self + 224, -1.0f);

    if (p2 == 0)
        func_00282D30_6(p1, p1, -1.0f);
}

INCLUDE_ASM("asm/levels", func_L14_00420B10);

INCLUDE_ASM("asm/levels", func_L14_00421BC0);

INCLUDE_ASM("asm/levels", func_L14_004221D8);

INCLUDE_ASM("asm/levels", func_L14_00422498);

INCLUDE_ASM("asm/levels", func_L14_004228C0);

INCLUDE_ASM("asm/levels", func_L14_004229A0);

extern void func_423000();
extern void func_423240();
extern void func_423358();
extern void func_423B30();
extern void func_423BD8();

void func_L14_00422FA0(char *a0)
{
    if (*(unsigned char *)(a0 + 0x20) == 0) {
        func_423000(a0);
    } else {
        func_423240(a0);
        func_423358(a0);
        func_423B30(a0);
        func_423BD8(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L14_00423000);

INCLUDE_ASM("asm/levels", func_L14_00423240);

INCLUDE_ASM("asm/levels", func_L14_00423358);

extern void func_002B5278(char *object, float factor);

extern void func_002B5258(char *object);

extern void func_002B50E0(char *object, char *bone, float factor);

extern void func_002B4FF0(char *object, char *bone, float factor);

/* Dispatches on the owner's animation step and plays the matching bone animation. */
INCLUDE_ASM("asm/levels", func_L14_00423B30);

extern char D_L14_001B9500[];

extern void func_80326928(char *a0, void *a1);

extern void func_8032AA20(char *a0, void *a1, int a2, float a3, float a4);

extern float func_802F5DF8(char *a0, void *a1);

extern void func_803240F8(char *a0);

/* Restarts the owner's animation: two timed fades, the counter, then the panic state. */
void func_L14_00423BD8(char *a0)
{
    char *s1;
    float f20;
    float f21;

    s1 = *(char **)(a0 + 0x68);
    f21 = 0.03f;
    f20 = 0.3f;
    func_80326928(a0, s1 + 0x50);
    func_8032AA20(a0, s1 + 0x680, 1, f21, f20);
    func_8032AA20(a0, s1 + 0x600, 0, f21, f20);
    if (*(unsigned short *)(a0 + 0x96) <= 0xFFFE)
        *(unsigned short *)(a0 + 0x96) = *(unsigned short *)(a0 + 0x96) + 1;
    *(unsigned short *)(s1 + 0x776) = 0;
    *(unsigned short *)(s1 + 0x778) = 0;
    *(unsigned short *)(s1 + 0x77A) = 0;
    if (*(unsigned char *)(a0 + 0x31) != 0) {
        if (func_802F5DF8(a0 + 0x10, D_L14_001B9500) < 24.0f) {
            func_803240F8(a0);
            *(unsigned char *)(a0 + 0x6F) = 0x16;
        }
    }
}

extern void func_318ED0();
extern void func_2F5C68();
extern float func_2F62F0(float a, float b);
extern float func_2F5DB0(float *p);

void func_L14_00423DC0(char *a0, int a1)
{
    float v[4];
    char *s0;
    float f;

    s0 = *(char **)(a0 + 0x68);
    *(unsigned short *)(s0 + 0x776) = 1;
    func_318ED0(a0, 2, v);
    func_2F5C68(v, a1, v);
    f = func_2F62F0(v[0], v[1]);
    *(float *)(s0 + 0x744) = f;
    f = func_2F5DB0(v);
    f = -func_2F62F0(f, v[2]) * 0.5f;
    *(float *)(s0 + 0x6E4) = f;
    *(float *)(s0 + 0x664) = f;
}

INCLUDE_ASM("asm/levels", func_L14_00423E50);

INCLUDE_ASM("asm/levels", func_L14_00425C08);

INCLUDE_ASM("asm/levels", func_L14_00425CB0);

INCLUDE_ASM("asm/levels", func_L14_004265B8);

INCLUDE_ASM("asm/levels", func_L14_00426D18);

extern short D_001A6480[];

extern void func_385CB8(unsigned int reg, unsigned long value);

extern void func_2EF6D8(int, int, int, int, int);

/* Queues the level jingle for the owner with the volume its state block asks for. */
void func_L14_00426DE8(char *a0)
{
    char *v0;
    int s0;

    v0 = *(char **)(a0 + 0x68);
    s0 = ((int)((1.0f - *(float *)(v0 + 0x68)) * 128.0f)) << 24;
    func_385CB8(0x42, 9);
    func_2EF6D8(0, D_001A6480[0xA9], 0, D_001A6480[0xA8], s0);
}

/* func_L14_00426E60 lives in the -g8 sibling file (it reads two floats through $gp). */

INCLUDE_ASM("asm/levels", func_L14_00426F28);

INCLUDE_ASM("asm/levels", func_L14_00427210);

/* func_L14_00427250 lives in the -g8 sibling file (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L14_00427458);

INCLUDE_ASM("asm/levels", func_L14_004275E0);

INCLUDE_ASM("asm/levels", func_L14_004281D0);

INCLUDE_ASM("asm/levels", func_L14_004286F8);

INCLUDE_ASM("asm/levels", func_L14_00428A08);

INCLUDE_ASM("asm/levels", func_L14_00428BA0);

INCLUDE_ASM("asm/levels", func_L14_00428E68);

INCLUDE_ASM("asm/levels", func_L14_004295D0);

INCLUDE_ASM("asm/levels", func_L14_004298F0);

INCLUDE_ASM("asm/levels", func_L14_00429A18);

extern void *func_801234F0(float scale);

extern long long func_80131430(long long value);

extern long long func_80122B00(long long a, long long b);

extern void *func_80122DA8(void *a, long long b);

extern float func_80123298(long long a);

extern void func_80321F58(char *a0, int a1, int a2);

/* Fills the owner's screen slot with the width the display state reports. */
void func_L14_0042D018(char *a0)
{
    char *p;
    void *s0;
    long long v;

    p = *(char **)(a0 + 0x68);
    if (*(unsigned char *)(a0 + 0x20) != 0)
        return;
    s0 = func_801234F0(*(float *)(a0 + 0xC));
    v = func_80131430(0x4000000000000000LL);
    v = func_80122B00(v, 0x4090000000000000LL);
    v = func_80122DA8(s0, v);
    *(float *)(p + 0x10) = func_80123298(v);
    func_80321F58(a0, 1, -1);
}

INCLUDE_ASM("asm/levels", func_L14_0042FB10);

INCLUDE_ASM("asm/levels", func_L14_0042FDA8);

INCLUDE_ASM("asm/levels", func_L14_00430410);

extern float func_2F6DA0(float x);

float func_L14_00430898(float x)
{
    return func_2F6DA0(x * 0.15915494f) * 6.2831855f;
}

INCLUDE_ASM("asm/levels", func_L14_004308D0);

INCLUDE_ASM("asm/levels", func_L14_00430B70);

INCLUDE_ASM("asm/levels", func_L14_00430E08);

INCLUDE_ASM("asm/levels", func_L14_004321C0);

INCLUDE_ASM("asm/levels", func_L14_004321D0);

INCLUDE_ASM("asm/levels", func_L14_00432450);
