#include "include_asm.h"

extern int D_L00_001A8FD8 __attribute__((sda));

extern int D_L00_001A8FDC __attribute__((sda));

extern void func_0033AE20(int *p, int flag);

extern void func_0033AF48_1(int *p);

extern void func_00336320(int *p);

extern void func_L00_0042BAD0_1(char *object);

extern void func_L00_0042C290_1(char *object);

extern void F8ad956d6_FUN_00336320(char *target);

extern void F8ad956d6_FUN_002FC560(int marker, int code);

extern void F8ad956d6_FUN_00295F60(int first, int second);

extern void F8ad956d6_FUN_002DAF68(char *target);

extern void F8ad956d6_FUN_00336A78(char *target);

extern void func_L00_0042BAD0_5(void *);

extern void func_L00_0042BFF8_4(void *);

extern void func_L00_0042C290_5(void *);

typedef float f32;

typedef unsigned char u8;

typedef int s32;

extern int func_0033AB50(int);

extern void F7d9c8805_FUN_003361A8(int);

extern void F7d9c8805_FUN_00336968(int);

extern void func_0033AE18(unsigned char *owner, int value);

/* Lazily initialises a sub-object of the argument behind two global flags and a field, runs one guarded setup step on it, then hands it to two callees. */
void func_003399B0(int *obj)
{
    if (D_L00_001A8FD8 != 0 || D_L00_001A8FDC != 0 || obj[192] != 0)
        func_0033AE20(obj + 2, 0);
    if (obj[191] == 1)
        func_0033AE20(obj + 2, 1);
    func_0033AF48_1(obj + 2);
    func_00336320(obj + 170);
}

/* calls two one-argument functions on a sequence of sub-objects of the argument (four at 8..236 stride 76, nine at 312..888 stride 72) and returns the argument */
char *func_00339A38(char *object)
{
    func_L00_0042BAD0_1(object + 8);
    func_L00_0042BAD0_1(object + 84);
    func_L00_0042BAD0_1(object + 160);
    func_L00_0042BAD0_1(object + 236);
    func_L00_0042C290_1(object + 312);
    func_L00_0042C290_1(object + 384);
    func_L00_0042C290_1(object + 456);
    func_L00_0042C290_1(object + 528);
    func_L00_0042C290_1(object + 600);
    func_L00_0042C290_1(object + 672);
    func_L00_0042C290_1(object + 744);
    func_L00_0042C290_1(object + 816);
    func_L00_0042C290_1(object + 888);
    return object;
}

INCLUDE_ASM("asm/game", func_00339AC8);

INCLUDE_ASM("asm/game", func_00339AD0);

INCLUDE_ASM("asm/game", func_00339F20);

INCLUDE_ASM("asm/game", func_00339F28);

void func_0033A3E0(char *object)
{
    char *s0 = object;

    if (*(int *)(s0 + 1024) == 0)
        return;
    F8ad956d6_FUN_00336320(s0 + 8);
    F8ad956d6_FUN_00336320(s0 + 84);
    F8ad956d6_FUN_002FC560(66, 68);
    F8ad956d6_FUN_002FC560(71, 11);
    F8ad956d6_FUN_00295F60(0, 1);
    F8ad956d6_FUN_002DAF68(0);
    F8ad956d6_FUN_00336320(s0 + 160);
    F8ad956d6_FUN_00336320(s0 + 236);
    F8ad956d6_FUN_00336A78(s0 + 312);
    F8ad956d6_FUN_00336A78(s0 + 384);
    F8ad956d6_FUN_00336A78(s0 + 456);
    F8ad956d6_FUN_00336A78(s0 + 528);
    F8ad956d6_FUN_00336A78(s0 + 600);
    F8ad956d6_FUN_00336A78(s0 + 672);
    F8ad956d6_FUN_00336A78(s0 + 744);
    F8ad956d6_FUN_00336A78(s0 + 816);
    F8ad956d6_FUN_00336A78(s0 + 888);
}

/* walks four 76-byte records calling one function on each, then calls two more functions on later fields and returns its argument */
void *func_0033A4A0(char *p)
{
    int i;
    char *q = p + 4;

    for (i = 3; i != -1; i--) {
        func_L00_0042BAD0_5(q);
        q += 76;
    }
    func_L00_0042BFF8_4(p + 308);
    func_L00_0042C290_5(p + 368);
    return p;
}

INCLUDE_ASM("asm/game", func_0033A518);

INCLUDE_ASM("asm/game", func_0033A520);

/* func_0033A688 is matched in 003399B0-g8.c (-G8 small data). */

INCLUDE_ASM("asm/game", func_0033A6D8);

/* Measured target: byte-identical across all 27 levels. */
void func_0033A9C8(u8 *a0, f32 a1) { *(f32 *)(a0 + 0x204) = a1; }

/* lot6 -- measured bodies with sizes recorded in the catalogue. */
void func_0033A9D0(u8 *a0, s32 a1, s32 a2) { a0 += a1 * 4; *(s32 *)(a0 + 0x1B8) = a2; }

/* writes the value if it is below the maximum; byte-identical across all 27 levels. */
void func_0033A9E0(u8 *a0, s32 a1) { if (a1 < *(s32 *)(a0 + 0x1F8)) *(s32 *)(a0 + 0x208) = a1; }

/* Measured target: byte-identical across all 27 levels. */
void func_0033A9F8(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x20c) = a1; }

INCLUDE_ASM("asm/game", func_0033AA00);

int func_0033AB50(int a0)
{
	char *p = (char *)a0 + 12;
	int i;

	for (i = 4; i != -1; i--) {
		F7d9c8805_FUN_003361A8(p);
		p += 76;
	}
	F7d9c8805_FUN_00336968(a0 + 408);
	F7d9c8805_FUN_00336968(a0 + 480);
	F7d9c8805_FUN_00336968(a0 + 552);
	return a0;
}

INCLUDE_ASM("asm/game", func_0033ABD0);

INCLUDE_ASM("asm/game", func_0033ABD8);

/* Measured target: byte-identical across all 27 levels. */
void func_0033AE18(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x290) = a1; }
