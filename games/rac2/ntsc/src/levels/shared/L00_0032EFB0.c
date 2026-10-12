#include "include_asm.h"

extern int func_L00_0032EF10(int, int, int *);

extern int func_00282C30(int);

extern void func_00282CC0_2(char *dst, char *left, char *right);

extern void func_00300190(char *dst, char *src);

extern int func_00283CF0(float value);

extern void func_002B9180_2(char *object);

extern int func_00282978_11();

extern void func_002B9180_17();

typedef unsigned char u8;

extern int func_00282D30_2(void *, void *, float);

extern int func_00282CC0_3(void *, void *, void *);

extern int func_002B9180_1(void *);

/* Calls one routine twice with two different ids and a two-int scratch area, short-circuiting to a zero result if either fails, and otherwise returns whether a third routine's difference of the two ints is below a fourth incoming integer. */
int func_L00_0032EFB0(int a0, int a1, int a2, int a3)
{
	int r = 0;
	int a;
	int b;

	if (func_L00_0032EF10(a0, a2, &a)) {
		if (func_L00_0032EF10(a1, a2, &b))
			r = func_00282C30(a - b) < a3;
	}
	return r;
}

INCLUDE_ASM("asm/levels", func_L00_0032F030);

INCLUDE_ASM("asm/levels", func_L00_0032F038);

INCLUDE_ASM("asm/levels", func_L00_0032F080);

void func_L00_003318D8(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00333E20);

INCLUDE_ASM("asm/levels", func_L00_00333F90);

/* Calls a three-argument callee twice and a two-argument callee twice over fields of the first object, folds a float field through a float-to-int callee into an int field of the object, and calls a last callee when the updated field is negative. */
void func_L00_003340A8(char *a)
{
    char *p1 = a + 16;
    char *p2 = a + 32;
    char *p3 = a + 48;
    int v;

    func_00282CC0_2(p1, p1, p3);
    func_00282CC0_2(p2, p2, p3);
    func_00300190(p1, p1);
    func_00300190(p2, p2);
    v = *(int *)(a + 4) - (func_00283CF0(*(float *)(p3 + 12)) << 24);
    *(int *)(a + 4) = v;
    *(int *)(a + 12) = v;
    if (v < 0)
        func_002B9180_2(a);
}

INCLUDE_ASM("asm/levels", func_L00_00335558);

/* Calls a routine on pointer-plus-10 and, when it returns non-zero, calls a second routine with the pointer itself. */
void func_L00_00336548(char *p)
{
    if (func_00282978_11(p + 10)) {
        func_002B9180_17(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00338AE0);

INCLUDE_ASM("asm/levels", func_L00_00338D28);

/* Updates a record: calls two helpers on the sub-object at +0x20 (float argument 0.975f), advances the float at +0x0C by 14700.0f, adds the byte at +0x30 into the byte at +0x08, then adds 0xFE000000 to the word at +0x04 and calls a third helper when the result is negative. */
void func_L00_00338DF8(char *a0)
{
    char *p = a0;
    char *q = p + 32;
    int v;

    func_00282D30_2(q, q, 0.975f);
    func_00282CC0_3(p + 16, p + 16, q);
    *(float *)(p + 12) = *(float *)(p + 12) + 14700.0f;
    *(u8 *)(p + 8) = *(u8 *)(p + 8) + *(u8 *)(q + 16);
    v = *(int *)(p + 4) + 0xFE000000;
    *(int *)(p + 4) = v;
    if (v < 0) {
        func_002B9180_1(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_0033AA40);

INCLUDE_ASM("asm/levels", func_L00_0033BA68);

INCLUDE_ASM("asm/levels", func_L00_0033C0D0);

INCLUDE_ASM("asm/levels", func_L00_0033C238);

INCLUDE_ASM("asm/levels", func_L00_0033C638);

INCLUDE_ASM("asm/levels", func_L00_0033E210);

INCLUDE_ASM("asm/levels", func_L00_0033E220);

INCLUDE_ASM("asm/levels", func_L00_0033E3B8);

INCLUDE_ASM("asm/levels", func_L00_0033E888);

INCLUDE_ASM("asm/levels", func_L00_0033EA00);

INCLUDE_ASM("asm/levels", func_L00_0033EC48);

INCLUDE_ASM("asm/levels", func_L00_0033EE38);
