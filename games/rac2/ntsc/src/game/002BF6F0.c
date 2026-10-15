#include "include_asm.h"

/* Qwen draft (branch qwen/autonomy, c8b09022), checked by tools/check.py */
void func_002B9180(void *);

int func_00282978(short *);

extern int func_00282978_9(char *p);

extern void func_002B9180_13(char *self);

extern float func_00283CE0_2(int v);

extern int func_00283CF0_6(float f);

extern void func_00282CC0_17(char *dst, char *src, char *vec);

extern void func_00300190_4(char *dst, char *src);

extern void func_00282D30_12(char *dst, char *table, float scale);

extern char D_0018A0E0_1[];

extern void Fa2a84657_FUN_00282D48(float *tmp, char *v, float k);

extern void Fa2a84657_FUN_00282CD8(float *tmp, char *a, char *v);

extern int Fa2a84657_FUN_00282978(char *field);

extern void Fa2a84657_FUN_002B9180(unsigned char *p);

typedef struct { unsigned int f0, f4, f8, f12, f16, f20; } T;

extern int func_00282978_12(short *);

extern void func_002B9180_19(char *);

extern float func_00283CE0_5(int);

extern int func_00283D38_6(int, int, float);

struct T {
    float a;
    float b;
    int c;
    int d;
    float e;
};

extern void F15a8b7f0_FUN_00282D30(char *first, char *second, float blend);

extern void F15a8b7f0_FUN_00282CC0(char *first, char *second, char *third);

extern void F15a8b7f0_FUN_002B9180(char *object);

typedef unsigned char u8;

extern void func_00282CC0_19(void *out, void *in, void *src);

extern int func_00282978_11(void *p);

extern void func_002B9180_15(void *self);

extern float func_00283CE0_4(int n);

extern int func_00283D38_3(int handle, int previous, float ratio);

extern char D_00189E20_18[];

extern void Fccb147a4_FUN_002B9180(char *a, char *b);

extern void Fccb147a4_FUN_00300190(char *a, char *b);

extern void Fa03721e9_FUN_00282CC0(char *a, char *b, char *c);

extern void Fa03721e9_FUN_002B9180(char *p);

typedef int s32;

#ifndef RAC2_T_S16_F570CFB4B
#define RAC2_T_S16_F570CFB4B
typedef short s16_F570cfb4b;
#endif

extern void func_002B9180_7(u8 *);

typedef float f32;

extern int F7cacd225_FUN_00282978(short *p);

extern void F7cacd225_FUN_002B9180(char *o);

extern float F7cacd225_FUN_00283CE0(int n);

extern int F7cacd225_FUN_00283D38(int a, int b, float q);

#ifndef RAC2_T_OBJ_F7CACD225
#define RAC2_T_OBJ_F7CACD225
typedef struct {
    int pad0;
    int f4;
    short pad8;
    short s10;
    float f12;
    float f16;
    float f20;
    float f24;
    int pad28;
    int f32;
} Obj_F7cacd225;
#endif

extern int func_00282978_6(char *);

extern void func_002B9180_10(char *);

extern void func_00282CC0_21(char *, char *, char *);

extern void func_00300190_2(char *, char *);

#ifndef RAC2_T_VEC16_F9A4B4D8B
#define RAC2_T_VEC16_F9A4B4D8B
typedef struct {
    float f[4];
} Vec16_F9a4b4d8b;
#endif

#ifndef RAC2_T_V4_FC3CB9262
#define RAC2_T_V4_FC3CB9262
typedef int V4_Fc3cb9262 __attribute__((mode(TI)));
#endif

extern void Fc3cb9262_FUN_00282CC0(char *a, char *b, char *c);

extern int Fc3cb9262_FUN_00283CF0(float f);

extern int Fc3cb9262_FUN_00282978(char *a);

extern void Fc3cb9262_FUN_002B9180(char *a);

void func_002BF6F0(void *param_1)
{
  int lVar1;

  lVar1 = func_00282978((short *)((char *)param_1 + 10));
  if (lVar1 != 0) {
    func_002B9180(param_1);
  }
  return;
}

INCLUDE_ASM("asm/game", func_002BF728);

INCLUDE_ASM("asm/game", func_002BF8F0);

INCLUDE_ASM("asm/game", func_002BFB38);

INCLUDE_ASM("asm/game", func_002BFC50);

INCLUDE_ASM("asm/game", func_002BFE80);

INCLUDE_ASM("asm/game", func_002BFFE8);

INCLUDE_ASM("asm/game", func_002C00F0);

INCLUDE_ASM("asm/game", func_002C01F8);

INCLUDE_ASM("asm/game", func_002C02F0);

INCLUDE_ASM("asm/game", func_002C04C0);

INCLUDE_ASM("asm/game", func_002C0C70);

INCLUDE_ASM("asm/game", func_002C0CC0);

INCLUDE_ASM("asm/game", func_002C0E50);

INCLUDE_ASM("asm/game", func_002C14D8);

INCLUDE_ASM("asm/game", func_002C16C0);

/* Updates one byte-addressed object by packing a converted short-ratio value with three bytes, applying the bound helper sequence to three stored floats, and accumulating one byte and one float. */
void func_002C1840(char *self)
{
    float vec[4];
    float tmp[4];
    char *s1;
    char *p;
    float f20;
    float t;
    unsigned int v;

    if (func_00282978_9(self + 10)) {
        func_002B9180_13(self);
        return;
    }
    s1 = self + 32;
    f20 = func_00283CE0_2(*(short *)(s1 + 30));
    t = func_00283CE0_2(*(short *)(s1 + 28));
    v = func_00283CF0_6(f20 * ((float)*(short *)(self + 10) / t));
    v = v << 24;
    v = v | (*(unsigned char *)(s1 + 19) << 16);
    v = v | (*(unsigned char *)(s1 + 18) << 8);
    v = v | *(unsigned char *)(s1 + 17);
    *(unsigned int *)(self + 4) = v;
    vec[3] = 0.0f;
    vec[0] = *(float *)(self + 32);
    vec[1] = *(float *)(s1 + 4);
    vec[2] = *(float *)(s1 + 8);
    func_00282CC0_17((char *)self + 16, (char *)self + 16, (char *)vec);
    if (*(float *)(self + 28) != 0.0f) {
        if (*(float *)(self + 28) == 2.0f)
            func_00300190_4((char *)self + 16, (char *)self + 16);
        *(float *)(self + 28) = 2.0f;
    }
    p = (char *)tmp;
    func_00282D30_12(p, D_0018A0E0_1, *(float *)(s1 + 12));
    func_00282CC0_17((char *)vec, (char *)vec, p);
    func_00282D30_12((char *)vec, (char *)vec, *(float *)(s1 + 24));
    *(float *)s1 = vec[0];
    *(float *)(s1 + 4) = vec[1];
    *(float *)(s1 + 8) = vec[2];
    *(unsigned char *)(self + 8) = *(unsigned char *)(self + 8) + *(unsigned char *)(s1 + 16);
    *(float *)(self + 12) = *(float *)(self + 12) + *(float *)(s1 + 20);
}

INCLUDE_ASM("asm/game", func_002C19D8);

INCLUDE_ASM("asm/game", func_002C1B38);

INCLUDE_ASM("asm/game", func_002C1C88);

INCLUDE_ASM("asm/game", func_002C1DB0);

INCLUDE_ASM("asm/game", func_002C1E50);

INCLUDE_ASM("asm/game", func_002C1EF0);

INCLUDE_ASM("asm/game", func_002C1FC0);

void func_002C2080(unsigned char *p)
{
    float tmp[4];
    char *v;
    int n;

    v = (char *)p + 32;
    Fa2a84657_FUN_00282D48(tmp, v, 9.4999998807907104492188e-01f);
    *(float *)(v + 8) = *(float *)(v + 8) + 1.3888889225199818611145e-03f;
    Fa2a84657_FUN_00282CD8(tmp, (char *)p + 16, v);

    n = *(int *)(p + 4) + (int)0xFE000000;
    *(int *)(p + 4) = n;
    if (((unsigned)(n & 0xFF000000) - 1) > 0x5EFFFFFFu || Fa2a84657_FUN_00282978((char *)p + 10) != 0) {
        Fa2a84657_FUN_002B9180(p);
    } else {
        float f0;
        float f1;

        f1 = *(float *)(p + 12);
        f0 = 2.1000000000000000000000e+05f - f1;
        f0 = f0 * 7.0000000298023223876953e-02f;
        f1 = f1 + f0;
        *(float *)(p + 12) = f1;
        p[8] = p[8] + 1;
    }
}

/* Tests the halfword at offset 10 through a helper; on zero it stores a scaled int at offset 4 and a 210000.0f-scaled interpolation of two floats at offset 12, otherwise calls a second helper. */
void func_002C2170(char *p)
{
    if (func_00282978_12((short *)(p + 10)) != 0) {
        func_002B9180_19(p);
    } else {
        struct T *t = (struct T *)(p + 32);
        float k = func_00283CE0_5(*(short *)(p + 10)) * t->e;

        *(int *)(p + 4) = func_00283D38_6(t->d, t->c, k);
        *(float *)(p + 12) = ((t->a - t->b) * k + t->b) * 210000.0f;
    }
}

INCLUDE_ASM("asm/game", func_002C2210);

INCLUDE_ASM("asm/game", func_002C23B8);

INCLUDE_ASM("asm/game", func_002C2658);

INCLUDE_ASM("asm/game", func_002C27B0);

void func_002C28F8(char *object)
{
    int count;
    int low;
    int high;

    F15a8b7f0_FUN_00282D30(object + 32, object + 32, 0.975f);
    F15a8b7f0_FUN_00282CC0(object + 16, object + 16, object + 32);
    *(float *)(object + 12) += 6300.0f;
    count = *(unsigned char *)(object + 4) - 4;
    if (count <= 0)
        F15a8b7f0_FUN_002B9180(object);
    else {
        low = (count << 8) | 0x60000000;
        high = (count << 16) | low;
        *(int *)(object + 4) = high | count;
    }
}

INCLUDE_ASM("asm/game", func_002C29A0);

INCLUDE_ASM("asm/game", func_002C2A88);

INCLUDE_ASM("asm/game", func_002C2C80);

INCLUDE_ASM("asm/game", func_002C2FD8);

extern char D_00189EA0[];
extern void func_00282CC0(char *, char *, char *);

void func_002C30F8(char *self)
{
    char *part = self + 0x20;
    if (*(int *)(part + 0x10) == 1)
        func_00282CC0(self + 0x10, D_00189EA0, part);
    if (func_00282978((short *)(self + 0xA)) != 0)
        func_002B9180(self);
}

INCLUDE_ASM("asm/game", func_002C3150);

/* advances three floats of a sub-object by a resident offset triple, calls a three-pointer update, subtracts the offsets back out, rejects the result when any coordinate leaves the 2.0..1021.0 band, and otherwise rescales a 24-bit handle by the ratio of two helper lookups over a signed halfword index */
void func_002C3280(u8 *self)
{
    char *s2 = (char *)self + 32;
    int n;
    float a;
    float b;

    if (*(int *)(s2 + 28) == 1) {
        char *base = D_00189E20_18;
        *(float *)(self + 16) = *(float *)(base + 128) + *(float *)(s2 + 16);
        *(float *)(self + 20) = *(float *)(base + 132) + *(float *)(s2 + 20);
        *(float *)(self + 24) = *(float *)(base + 136) + *(float *)(s2 + 24);
        func_00282CC0_19(self + 16, self + 16, s2);
        *(float *)(s2 + 16) = *(float *)(self + 16) - *(float *)(base + 128);
        *(float *)(s2 + 20) = *(float *)(self + 20) - *(float *)(base + 132);
        *(float *)(s2 + 24) = *(float *)(self + 24) - *(float *)(base + 136);
    } else {
        func_00282CC0_19(self + 16, self + 16, s2);
    }

    if (*(float *)(self + 16) < 2.0f || *(float *)(self + 16) > 1021.0f
        || *(float *)(self + 20) < 2.0f || *(float *)(self + 20) > 1021.0f
        || *(float *)(self + 24) < 2.0f || *(float *)(self + 24) > 1021.0f) {
        func_002B9180_15(self);
        return;
    }

    if (func_00282978_11((char *)self + 10) != 0) {
        func_002B9180_15(self);
        return;
    }

    n = *(int *)(self + 4) & 0xFFFFFF;
    a = func_00283CE0_4(*(short *)(self + 10) - 1);
    b = func_00283CE0_4(*(short *)(self + 10));
    *(int *)(self + 4) = func_00283D38_3(n, *(int *)(self + 4), a / b);
}

INCLUDE_ASM("asm/game", func_002C3430);

INCLUDE_ASM("asm/game", func_002C36D0);

void func_002C37E0(char *obj)
{
    char *sub = obj + 32;
    int w;
    int n;

    switch (*(unsigned char *)(sub + 2)) {
    case 2:
        *(float *)(obj + 16) = *(float *)(*(char **)(sub + 4) + 16) + *(float *)(sub + 16);
        *(float *)(obj + 20) = *(float *)(*(char **)(sub + 4) + 20) + *(float *)(sub + 20);
        *(float *)(obj + 24) = *(float *)(*(char **)(sub + 4) + 24) + *(float *)(sub + 24);
        break;
    case 1:
        goto tail;
    }
    w = *(int *)(obj + 4);
    n = (w >> 24);
    n = n - *(short *)(obj + 32);
    if (n <= 0) {
        Fccb147a4_FUN_002B9180(obj, sub);
        return;
    }
    *(int *)(obj + 4) = (w & 0xffffff) | (n << 24);
tail:
    if (*(unsigned char *)(sub + 3) != 0) {
        obj += 16;
        Fccb147a4_FUN_00300190(obj, obj);
    }
}

INCLUDE_ASM("asm/game", func_002C38A0);

INCLUDE_ASM("asm/game", func_002C38E0);

INCLUDE_ASM("asm/game", func_002C3B08);

INCLUDE_ASM("asm/game", func_002C3D80);

void func_002C3EE8(char *p)
{
    char *q = p + 32;

    Fa03721e9_FUN_00282CC0(p + 16, p + 16, q);
    *(float *)(p + 12) = *(float *)(p + 12) + *(float *)(q + 16);
    *(unsigned char *)(p + 8) = *(unsigned char *)(p + 8) + 1;
    if (--*(short *)(p + 10) == 0) {
        *(unsigned int *)(p + 4) = *(unsigned int *)(p + 4) + 0xff000000;
        *(short *)(p + 10) = 2;
    }
    if ((*(unsigned int *)(p + 4) & 0xff000000) == 0)
        Fa03721e9_FUN_002B9180(p);
}

INCLUDE_ASM("asm/game", func_002C3F88);

INCLUDE_ASM("asm/game", func_002C4100);

INCLUDE_ASM("asm/game", func_002C43C8);

INCLUDE_ASM("asm/game", func_002C4500);

/* decrements a signed short counter and, at zero, rotates a 24-bit identifier beside it and reloads the counter; calls a helper when the identifier's top byte is clear */
void func_002C4788(u8 *a0) {
    s16_F570cfb4b *p = (s16_F570cfb4b *)(a0 + 10);
    if (--*p == 0) {
        *(s32 *)(a0 + 4) += 0xff000000;
        *p = 2;
    }
    if ((*(s32 *)(a0 + 4) & 0xff000000) == 0) {
        func_002B9180_7(a0);
    }
}

INCLUDE_ASM("asm/game", func_002C47E8);

void func_002C4B10(u8 *a0)
{
    f32 value = *(f32 *)(a0 + 0x0C);
    *(u8 *)(a0 + 0x08) = *(u8 *)(a0 + 0x08) + *(u8 *)(a0 + 0x38);
    if (value <= 0.0f) {
        *(f32 *)(a0 + 0x0C) = 0.0f;
        func_002B9180(a0);
    }
}

INCLUDE_ASM("asm/game", func_002C4B50);

INCLUDE_ASM("asm/game", func_002C4D18);

INCLUDE_ASM("asm/game", func_002C4F08);



INCLUDE_ASM("asm/game", func_002C5068);

INCLUDE_ASM("asm/game", func_002C5120);

INCLUDE_ASM("asm/game", func_002C5400);

void func_002C5550(Obj_F7cacd225 *o)
{
    int v;

    if (o->f16 < 2.0f || o->f16 > 1021.0f ||
        o->f20 < 2.0f || o->f20 > 1021.0f ||
        o->f24 < 2.0f || o->f24 > 1021.0f ||
        F7cacd225_FUN_00282978(&o->s10) != 0) {
        F7cacd225_FUN_002B9180((char *)o);
        return;
    }
    v = o->f32;
    if (v != 0) {
        o->f12 = o->f12 * 0.99f;
        o->f4 = F7cacd225_FUN_00283D38(0x30FFFFFF, o->f4, F7cacd225_FUN_00283CE0(o->s10 - 1) / F7cacd225_FUN_00283CE0(o->s10));
    } else {
        o->f12 = o->f12 * 0.95f;
        o->f4 = F7cacd225_FUN_00283D38(0x603F1008, o->f4, F7cacd225_FUN_00283CE0(o->s10 - 1) / F7cacd225_FUN_00283CE0(o->s10));
    }
}

INCLUDE_ASM("asm/game", func_002C56B0);

INCLUDE_ASM("asm/game", func_002C58A8);

INCLUDE_ASM("asm/game", func_002C5B70);

INCLUDE_ASM("asm/game", func_002C5D00);

INCLUDE_ASM("asm/game", func_002C6360);

INCLUDE_ASM("asm/game", func_002C64D8);

INCLUDE_ASM("asm/game", func_002C65A0);

/* If a probe call on a sub-object returns zero, folds a byte counter into the top byte of a packed word, rebuilds a 16-byte vector from two sub-records for a helper, then clamps a running float against a limit and accumulates two sub-record fields into the object. */
void func_002C6748(char *obj)
{
    Vec16_F9a4b4d8b local;
    char *s1;
    char *s2;
    int a;
    int v1;

    if (func_00282978_6(obj + 10) != 0) {
        func_002B9180_10(obj);
        return;
    }

    a = *(int *)(obj + 4);
    s1 = obj + 32;
    *(int *)(obj + 4) = (a & 0x00ffffff) | (((a >> 24) + *(unsigned char *)(s1 + 20)) << 24);

    s2 = obj + 16;
    local.f[0] = *(float *)(obj + 32);
    local.f[1] = *(float *)(s1 + 4);
    local.f[2] = *(float *)(s1 + 8);
    local.f[3] = 0.0f;
    func_00282CC0_21(s2, s2, (char *)&local);

    *(float *)(s1 + 8) = *(float *)(s1 + 8) - *(float *)(s1 + 12);

    if (*(float *)(obj + 28) != 0.0f) {
        if (*(float *)(obj + 28) == 2.0f)
            func_00300190_2(s2, s2);
        *(float *)(obj + 28) = 2.0f;
    }
    v1 = *(unsigned char *)(s1 + 21);

    *(unsigned char *)(obj + 8) = *(unsigned char *)(obj + 8) + v1;
    *(float *)(obj + 12) += *(float *)(s1 + 16);
}

INCLUDE_ASM("asm/game", func_002C6868);

INCLUDE_ASM("asm/game", func_002C6A08);

INCLUDE_ASM("asm/game", func_002C6B68);

INCLUDE_ASM("asm/game", func_002C6DD8);

INCLUDE_ASM("asm/game", func_002C6EF0);

INCLUDE_ASM("asm/game", func_002C7050);

INCLUDE_ASM("asm/game", func_002C72A0);

void func_002C7538(char *a0)
{
    V4_Fc3cb9262 buf;
    char *s0 = a0 + 32;
    float f0;
    float f1;
    float f2;

    f1 = *(float *)(s0 + 16) + 1.0000000474974513053894e-03f;
    f2 = *(float *)(s0 + 8) - 3.0000000260770320892334e-03f;
    *(float *)(s0 + 16) = f1;
    *(float *)(s0 + 8) = f2;
    f0 = *(float *)(s0 + 20) * f1 * 2.1000000000000000000000e+05f;
    buf = *(V4_Fc3cb9262 *)s0;
    *(float *)(a0 + 12) = f0;
    Fc3cb9262_FUN_00282CC0(a0 + 16, a0 + 16, (char *)&buf);
    *(int *)(a0 + 4) = (Fc3cb9262_FUN_00283CF0((float)*(short *)(a0 + 10)) << 24) | 0x00FFD2D2;
    *(unsigned char *)(a0 + 8) = *(unsigned char *)(a0 + 8) + *(unsigned char *)(s0 + 24);
    if (Fc3cb9262_FUN_00282978(a0 + 10) != 0)
        Fc3cb9262_FUN_002B9180(a0);
}

INCLUDE_ASM("asm/game", func_002C7620);
