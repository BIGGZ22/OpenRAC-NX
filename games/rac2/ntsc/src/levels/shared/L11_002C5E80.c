#include "include_asm.h"

extern void func_00282D30_2(char *dst, char *src, float scale);

extern void func_L02_00333480(char *a, char *b, char *c);

extern float func_00282C10(float a);

extern void func_00283CF0(float a);

INCLUDE_ASM("asm/levels", func_L11_002C5E80);

INCLUDE_ASM("asm/levels", func_L11_002CDE10);

void func_L11_003404E0(char *a0)
{
    func_L02_00333480(a0, a0 + 0x10, a0 + 0xC0);
}

INCLUDE_ASM("asm/levels", func_L11_00397C08);

INCLUDE_ASM("asm/levels", func_L11_00397E68);

INCLUDE_ASM("asm/levels", func_L11_003CF250);

INCLUDE_ASM("asm/levels", func_L11_003F4198);

INCLUDE_ASM("asm/levels", func_L11_00414660);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_0041E460(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

void func_L11_00439D78(float a, float b, float c, float d)
{
    float v = (d + d) * (a - b);

    func_00283CF0((c + func_00282C10(c * c + v)) / d);
}

INCLUDE_ASM("asm/levels", func_L11_0044FFA0);

INCLUDE_ASM("asm/levels", func_L11_00450508);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_004506C0(char *p)
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

INCLUDE_ASM("asm/levels", func_L11_004507B0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00458D00(char *p)
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

INCLUDE_ASM("asm/levels", func_L11_00458DF0);

INCLUDE_ASM("asm/levels", func_L11_0045FEF8);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_0045FFB0(char *p)
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

INCLUDE_ASM("asm/levels", func_L11_00460258);

INCLUDE_ASM("asm/levels", func_L11_0046B9F0);

INCLUDE_ASM("asm/levels", func_L11_0046BC08);

INCLUDE_ASM("asm/levels", func_L11_0046BD18);



INCLUDE_ASM("asm/levels", func_L11_0046C118);

INCLUDE_ASM("asm/levels", func_L11_0046C1E0);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00472008(char *p)
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

INCLUDE_ASM("asm/levels", func_L11_00472090);

extern int func_002ACBC8(void *a, int b);

extern void func_002829D8(void *a);

extern unsigned char D_L11_00189EA0[];

void func_L11_00474888(char *a0)
{
    if (func_002ACBC8(D_L11_00189EA0, *(int *)(*(char **)(a0 + 0x68))) == 0) return;
    if (*(int *)(D_L11_00189EA0 + 0x2214) != 0x82) return;
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
    func_002829D8(D_L11_00189EA0 + 0x530);
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L11_00478EF0(char *p)
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
