#include "include_asm.h"

/* The 128-bit unit four of these bodies copy whole. */
typedef int Rac2TI128 __attribute__((mode(TI)));

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_86f665335d9cb905_AnalysisOwner {
    unsigned char prefix_00[0x68];
    void *context_68;
} Rac2Native_86f665335d9cb905_AnalysisOwner;

extern void func_L00_003B9310(Rac2Native_86f665335d9cb905_AnalysisOwner *owner, void *context);

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_15(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

extern char D_L00_001BF240[];

extern void func_00282F20_2(void *out, void *in, float scale);

extern void func_00282CC0_6(void *out, void *in1, void *in2);

extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);

extern void func_00282D30_6(void *out, void *in, float scale);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner {
    unsigned char prefix_00[0x68];
    void *context_68;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner;

typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext {
    unsigned char prefix_00[0x10];
    float vector_10[4];
    unsigned int field_20;
    unsigned int active_24;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext;

extern void func_002FDBD8_2(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern void func_002A77E0_1(int a);

extern float func_002A7878_2(float a, float b);

extern unsigned int func_002A7820_1(int a, int b);

extern void func_002A7A00(char *p, float a, float b);

extern char *func_002BDCE8(float fa, float fb, float fc, float fd, char *p, int z,
                     char *local, unsigned int v, int one);

extern char D_00189E20_13[];

/* Pass the owner and its pointer at offset 0x68 to the explicitly bound helper; original game role is unknown. */
void func_L00_003B9CB8(Rac2Native_86f665335d9cb905_AnalysisOwner *owner)
{
    func_L00_003B9310(owner, owner->context_68);
}

extern void func_003182C0(void);

/* Calls the measured no-argument callee. */
void func_L00_003BAAD0(void)
{
    func_003182C0();
}

INCLUDE_ASM("asm/levels", func_L00_003BAC30);

INCLUDE_ASM("asm/levels", func_L00_003BB080);

INCLUDE_ASM("asm/levels", func_L00_003BB1F8);

extern void func_002FC560(unsigned int reg, unsigned long value);

extern float func_002A7940(void);

extern void func_00281EB8(char *a, int b, unsigned int c, int d, float e, float f,
                          float g);

extern void func_L00_003BBF48(char *a0);

extern void func_L00_003BCDC0(char *a0);

/* Opens the block, lays seven oriented sprites over it from two helper draws and the shared
   colour word, then runs the two slot-table passes and closes the block. */
void func_L00_003BB668(char *a0)
{
    char *ctx;

    ctx = *(char **)(a0 + 0x68);
    func_002FC560(0x47, 0x00053001);

    func_00281EB8(a0 + 0x10, 0x2C,
                  (func_002A7820_1(0x80, 0xC8) << 24) | 0x72FF, 1,
                  func_002A7878_2(0.38500002f, 0.415f) * *(float *)(ctx + 0x4C),
                  -0.01f, func_002A7940());
    func_00281EB8(a0 + 0x10, 0x2D,
                  (func_002A7820_1(4, 0x28) << 24) | 0x00DDB973, 1,
                  *(float *)(ctx + 0x4C) * 1.225f, 0.0f, func_002A7940());
    func_00281EB8(a0 + 0x10, 0x29,
                  (func_002A7820_1(4, 0x28) << 24) | 0x00DDB973, 1,
                  *(float *)(ctx + 0x4C) * 0.6f, 0.0f, func_002A7940());
    func_00281EB8(a0 + 0x10, 0x2A,
                  (func_002A7820_1(4, 0x2D) << 24) | 0x00DDB973, 1,
                  func_002A7878_2(0.975f, 1.025f) * *(float *)(ctx + 0x4C),
                  0.0f, func_002A7940());
    func_00281EB8(a0 + 0x10, 0x2A,
                  (func_002A7820_1(4, 0x2D) << 24) | 0x00DDB973, 1,
                  func_002A7878_2(0.975f, 1.025f) * *(float *)(ctx + 0x4C),
                  0.0f, func_002A7940());
    func_L00_003BBF48(a0);
    func_L00_003BCDC0(a0);
    func_002FC560(0x47, 0x0005360B);
}

INCLUDE_ASM("asm/levels", func_L00_003BB8B8);

INCLUDE_ASM("asm/levels", func_L00_003BBB68);

INCLUDE_ASM("asm/levels", func_L00_003BBF48);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003BC080(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_003BC108);

INCLUDE_ASM("asm/levels", func_L00_003BC228);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003BC400(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L00_003BC468);

INCLUDE_ASM("asm/levels", func_L00_003BC998);

INCLUDE_ASM("asm/levels", func_L00_003BCC10);

INCLUDE_ASM("asm/levels", func_L00_003BCDC0);

INCLUDE_ASM("asm/levels", func_L00_003BCE28);

INCLUDE_ASM("asm/levels", func_L00_003BCF88);

INCLUDE_ASM("asm/levels", func_L00_003BD510);

INCLUDE_ASM("asm/levels", func_L00_003BD7E0);

/* Normalises and combines two temporary vectors from a state block, dispatches on the combined result and conditionally scales the output vector. */
void func_L00_003BFDE8(char *self, char *p1, int p2)
{
    float a[4];
    float b[4];

    func_00282F20_2(a, self + 224, 0.1f);
    func_00282F20_2(b, self + 224, -3.0f);
    func_00282CC0_6(a, a, self + 16);
    func_00282CC0_6(b, b, self + 16);

    if (func_002761F8(a, b, 2, self, 0))
        func_00282F20_2(p1, D_L00_001BF240, 1.0f);
    else
        func_00282F20_2(p1, self + 224, -1.0f);

    if (p2 == 0)
        func_00282D30_6(p1, p1, -1.0f);
}

/* Read the word at offset 0x 18 through the pointer stored at owner offset 0x 68. */
unsigned int func_L00_003BFF58(unsigned int param_1)
{
    unsigned int inner_ptr;
    unsigned int result;

    inner_ptr = *(unsigned int *)(param_1 + 0x68);
    result = *(unsigned int *)(inner_ptr + 0x18);
    return result;
}

INCLUDE_ASM("asm/levels", func_L00_003BFF68);

/* shared native body of family a04928c6 */
void func_L00_003C1D30(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

INCLUDE_ASM("asm/levels", func_L00_003C1FC8);

INCLUDE_ASM("asm/levels", func_L00_003C27A0);

extern void func_L00_003263D0(char *a, char *b, float h, int c, int d, char *e, float i,
                               int f, int g, float j, float k);

/* Twenty rounds of four bound helpers on three 16-byte scratch vectors, the first of which
   starts as the second argument's record; each round ends with the eleven-argument emitter. */
void func_L00_003C2DE8(char *a0, char *a1, float fa, float fb)
{
    char v0[16];
    char v1[16];
    char v2[16];
    char *pv0 = v0;
    int i;

    *(Rac2TI128 *)pv0 = *(Rac2TI128 *)a1;
    i = 0x13;
    do {
        func_002A7A00(v1, fa, fa);
        func_00282F20_2(v2, v1, func_002A7878_2(fb * 0.75f, fb * 1.25f));
        func_00282CC0_6(v1, v1, pv0);
        func_L00_003263D0(a0, v1, 0.0f, 1, 0, v2, 0.0f, 0x50804040, 0, 2.0f, 5.0f);
        i--;
    } while (i >= 0);
}

INCLUDE_ASM("asm/levels", func_L00_003C2F10);

INCLUDE_ASM("asm/levels", func_L00_003C2FC8);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_003C33E0(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_2(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003C3418);

/* The 0x68 context block starts with two words and a halfword that this body clears. */
void func_L00_003C37A0(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(unsigned char *)(p + 0x20) == 0) {
        *(int *)(q + 0x8) = 0;
        *(short *)(q + 0xE) = 0;
        *(int *)(q + 0x0) = 0;
        *(unsigned char *)(p + 0x20) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003C37C8);

/* Runs 20 local iterations; the first two calls each pass integer 2 to the same bound target; on each non-null final result, stores a short at +10, an int at +36, and bytes at +42 and +43. */
void func_L00_003C3920(char *unused, char *arg)
{
    char local[16];
    float f;
    int i;
    unsigned int v;
    unsigned int w;
    unsigned int x;
    char *r;

    i = 19;
    do {
        func_002A77E0_1(2);
        i = i - 1;
        func_002A77E0_1(2);
        f = func_002A7878_2(70000.0f, 150000.0f);
        v = func_002A7820_1(10, 127);
        w = (v << 8) | 0x7f000000U;
        x = (v << 16) | w;
        v = x | v;
        func_002A7A00(local, 1.6666668670950457453728e-05f,
                6.6666672937572002410889e-03f);
        r = func_002BDCE8(0.3f, 1.0f, 1.01f, f, arg, 0, local, v, 1);
        if (r != 0) {
            char *q = r + 32;

            *(short *)(r + 10) = 120;
            *(int *)(q + 4) = 2;
            *(char *)(q + 10) = 127;
            *(char *)(q + 11) = *(unsigned char *)(r + 10);
        }
    } while (i >= 0);
}

INCLUDE_ASM("asm/levels", func_L00_003C3A70);

INCLUDE_ASM("asm/levels", func_L00_003C3D90);

INCLUDE_ASM("asm/levels", func_L00_003C48D0);

INCLUDE_ASM("asm/levels", func_L00_003C4B98);

INCLUDE_ASM("asm/levels", func_L00_003C4F78);

INCLUDE_ASM("asm/levels", func_L00_003C5310);

INCLUDE_ASM("asm/levels", func_L00_003C58E8);

/* one float gate followed by five integer gates; every failing gate */
int func_L00_003C5B18(float f12)
{
    char *b;

    if (f12 <= 0.0f)
        goto fail;
    b = D_00189E20_13;
    if (*(short *)(*(int *)(b + 8848) + 170) != 0)
        goto fail;
    if (*(unsigned char *)(b + 8884) != 0)
        goto fail;
    if (*(int *)(b + 8852) == 49)
        goto fail;
    if (*(int *)(b + 8860) == 20)
        goto fail;
    if (*(int *)(b + 9420) > 0)
        goto ok;
fail:
    return 0;
ok:
    return 1;
}
