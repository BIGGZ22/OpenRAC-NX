#include "include_asm.h"

#ifndef RAC2_T_V4_F0FBA48D3
#define RAC2_T_V4_F0FBA48D3
typedef int V4_F0fba48d3 __attribute__((mode(TI)));
#endif

extern void func_00282CF0_2(void *b, void *c, void *d);

extern void func_00282F20(void *dst, void *src, float x);

extern void func_00282CC0(void *c, void *c2, void *s);

extern void func_00282CF0_2(void *b, void *c, void *p);

extern void func_00282F20(void *dst, void *src, float y);

extern void func_00282CC0(void *b, void *p, void *s);

extern void func_00282CF0_2(void *b, void *s, void *e);

extern char D_L00_001BF220_1[];

extern void func_00282CF0_7(char *local, char *data);

extern float func_00282DB0_1(char *local, char *p);

extern float D_L00_001A8900 __attribute__((sda));

extern void func_00282CF0_4(float *out, char *a, char *b);

extern void func_00282D08(float *out, char *a, char *b, float k);

extern float func_00282DF0(float *v);

extern void func_00282F20_3(float *out, float *in, float k);

INCLUDE_ASM("asm/levels", func_L00_004224D0);

INCLUDE_ASM("asm/levels", func_L00_00423038);

INCLUDE_ASM("asm/levels", func_L00_00423290);

INCLUDE_ASM("asm/levels", func_L00_004233F8);

INCLUDE_ASM("asm/levels", func_L00_004237C8);

INCLUDE_ASM("asm/levels", func_L00_004239A8);

INCLUDE_ASM("asm/levels", func_L00_00423BB0);

/* reads a pointer from the field at +112 of the first argument, advances it by 176, then runs three 16-byte records and two float parameters through a fixed sequence of seven calls that alternate between three helpers */
void func_L00_00423BC0(char *a0, void *a1, void *a2, void *a3,
                                      void *a4, float t0, float t1)
{
    V4_F0fba48d3 b0, b1, b2, b3;
    void *p = (void *)(*(int *)(a0 + 112));

    p = (void *)((int)p + 176);

    func_00282CF0_2(&b0, a2, a3);
    func_00282F20(&b1, &b0, t0);
    func_00282CC0(a2, a2, &b1);
    func_00282CF0_2(&b2, a2, p);
    func_00282F20(&b2, &b2, t1);
    func_00282CC0(&b3, p, &b2);
    func_00282CF0_2(a1, &b3, a4);
}

INCLUDE_ASM("asm/levels", func_L00_00423CA8);

/* Returns 0 unless the second argument equals 1; then it takes a pointer out of the first argument's field at +112, adds 352 to it, passes a 16-byte stack buffer and a data symbol to one routine, passes the same buffer and the shifted pointer to a second routine that returns a float, and returns 1 only when that float is below zero. */
int func_L00_00424070(int a0, int a1)
{
    char local[16];
    int s0;
    int r;

    if (a1 != 1) {
        r = 0;
    } else {
        s0 = *(int *)(a0 + 112) + 352;
        func_00282CF0_7(local, D_L00_001BF220_1);
        r = func_00282DB0_1(local, (char *)s0) < 0.0f ? 1 : 0;
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L00_004240D8);

INCLUDE_ASM("asm/levels", func_L00_004243C0);

INCLUDE_ASM("asm/levels", func_L00_00424940);

INCLUDE_ASM("asm/levels", func_L00_00424978);

INCLUDE_ASM("asm/levels", func_L00_00424A08);

INCLUDE_ASM("asm/levels", func_L00_00424A50);

INCLUDE_ASM("asm/levels", func_L00_00424A88);

INCLUDE_ASM("asm/levels", func_L00_00424AD8);

INCLUDE_ASM("asm/levels", func_L00_00424B20);

INCLUDE_ASM("asm/levels", func_L00_00424B48);

INCLUDE_ASM("asm/levels", func_L00_00424B70);

INCLUDE_ASM("asm/levels", func_L00_00424BB8);

INCLUDE_ASM("asm/levels", func_L00_00424C00);

INCLUDE_ASM("asm/levels", func_L00_00424C28);

INCLUDE_ASM("asm/levels", func_L00_00424C60);

INCLUDE_ASM("asm/levels", func_L00_00424C90);

INCLUDE_ASM("asm/levels", func_L00_00424EB0);

INCLUDE_ASM("asm/levels", func_L00_00424F00);

INCLUDE_ASM("asm/levels", func_L00_00424F58);

INCLUDE_ASM("asm/levels", func_L00_00424F98);

INCLUDE_ASM("asm/levels", func_L00_00425118);

/* Builds three vectors from an owner block, blends the stored value toward the closest of the sampled vector length and the stored reference gap, and writes the remaining gap back. */
void func_L00_00425490(char *obj)
{
    char *p = *(char **)(obj + 112);
    char *r = p + 496;
    char *q = p + 352;
    float buf[4];
    float a, b, c;

    if (*(short *)(p + 16) <= 0) return;
    func_00282CF0_4(buf, p, p + 176);
    func_00282D08((float *)q, q, (char *)buf, D_L00_001A8900);
    a = func_00282DF0((float *)q);
    b = *(float *)(q + 44);
    if (b < a) {
        a = b;
        func_00282F20_3((float *)q, (float *)q, a);
        c = *(float *)(q + 44);
    } else {
        float d = b - *(float *)(r + 48);
        if (a < d) {
            a = d;
            func_00282F20_3((float *)q, (float *)q, a);
        }
        c = *(float *)(q + 44);
    }
    *(float *)(r + 48) = c - a;
}

INCLUDE_ASM("asm/levels", func_L00_00425560);

INCLUDE_ASM("asm/levels", func_L00_00425A98);

INCLUDE_ASM("asm/levels", func_L00_00425CD0);

INCLUDE_ASM("asm/levels", func_L00_00426248);
