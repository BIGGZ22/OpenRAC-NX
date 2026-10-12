#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

typedef float f32;

/* Relocates serialized header offsets and compacts each row in its original storage. */
typedef unsigned long long NativeCompactU64;

typedef union {
    s32 field00[4];
    struct {
        NativeCompactU64 field00;
        short field08;
        short field0A;
        unsigned short field0C;
        unsigned short field0E;
    } compact;
} NativeCompactRow16;

typedef struct {
    u8 field00[6];
    short field06;
    u8 field08[4];
    f32 field0C;
    u32 field10;
    u32 field14;
    u32 field18;
    u32 field1C;
    u32 field20;
} NativeCompactHeader;

extern s32 func_00282A18(s32 value);

extern void func_00283388 (float *local);

extern void func_00300AA8 (char *object, float a, float b, int flags, float d, float *local);

#ifndef RAC2_T_V4_F213135AA
#define RAC2_T_V4_F213135AA
typedef struct { float x, y, z, w; } V4_F213135aa;
#endif

extern float F213135aa_FUN_002A7878(float a, float b);

extern void F213135aa_FUN_002833F0(char *a, char *b);

extern void F213135aa_FUN_00282F20(char *a, char *b, float f);

extern void F213135aa_FUN_00282CF0(char *a, char *b, char *c);

extern void F213135aa_FUN_00282DD8(char *a, char *b, char *c);

extern void F213135aa_FUN_002AB668(char *a, char *b, float f);

extern int F6eb4f363_FUN_002A77E0(int mode);

extern float F6eb4f363_FUN_002A90F8(char *object, char *local);

extern void F6eb4f363_FUN_00282D30(char *local, int value, float scale);

extern float F6eb4f363_FUN_002A7878(float low, float high);

extern void F6eb4f363_FUN_002BF728(char *object, char *local, float amount, float base);

#ifndef RAC2_T_WORKER_F9E2CD219
#define RAC2_T_WORKER_F9E2CD219
typedef struct {
    unsigned char pad00[76];
    short field4C;
    unsigned char pad4E[2];
    int field50;
    float field54;
    float field58;
} Worker_F9e2cd219;
#endif

#ifndef RAC2_T_OWNER_F9E2CD219
#define RAC2_T_OWNER_F9E2CD219
typedef struct {
    unsigned char pad00[104];
    Worker_F9e2cd219 *child;
} Owner_F9e2cd219;
#endif

extern float F9e2cd219_FUN_002A7798(float value);

extern void F9e2cd219_FUN_002ACF68(void *out, void *in, void *tmp, int mode, float value);

extern void F9e2cd219_FUN_00305310(void *owner, void *out);

extern void F9e2cd219_FUN_00307AD0(void *owner, void *out);

/* relocates serialized offsets and compacts16-byte rows in their original storage using captured sourcewords */
NativeCompactHeader *func_00300618(NativeCompactHeader *header) {
    u32 base = (u32)header;
    NativeCompactRow16 *source;
    s32 index;
    header->field10 += base;
    header->field14 += base;
    header->field18 += base;
    header->field1C += base;
    if (header->field20 != 0) header->field20 += base;
    index = 0;
    source = (NativeCompactRow16 *)header->field18;
    header->field0C *= 0.0032116016f;
    if (header->field06 > 0) {
        do {
            s32 first = source->field00[0];
            s32 second = source->field00[1];
            s32 third = source->field00[2];
            s32 fourth = source->field00[3];
            ((NativeCompactRow16 *)header->field18)[index].compact.field0A = first >> 4;
            source++;
            ((NativeCompactRow16 *)header->field18)[index].compact.field08 = second >> 4;
            ((NativeCompactRow16 *)header->field18)[index].compact.field0C = func_00282A18(third);
            ((NativeCompactRow16 *)header->field18)[index].compact.field0E = func_00282A18(fourth);
            ((NativeCompactRow16 *)header->field18)[index].compact.field00 = 0;
            index++;
        } while (index < header->field06);
    }
    return header;
}

INCLUDE_ASM("asm/game", func_00300748);

INCLUDE_ASM("asm/game", func_00300910);

/* scales a float argument by a field of the object into a stack buffer, then calls a second routine with the object, its flags, the buffer and three float arguments */
void func_00300968 (char *object, int flags, float a, float b, float c, float d)
{
  float local[16];

  if (flags >> 24)
    {
      c = c * *(float *) (object + 12);
      func_00283388 (local);
      local[0] = c;
      local[10] = c;
      func_00300AA8 (object, a, b, flags, d, local);
    }
}

INCLUDE_ASM("asm/game", func_00300A00);

INCLUDE_ASM("asm/game", func_00300AA8);

INCLUDE_ASM("asm/game", func_00300F58);

INCLUDE_ASM("asm/game", func_00301D10);

INCLUDE_ASM("asm/game", func_00301F18);

int func_003022A0(char *obj, char *obj2, float x, float y)
{
    V4_F213135aa m0, m1, m2;
    char *s1;
    char *s3;
    float f22;

    s1 = *(char **)(obj + 104);
    if (*(unsigned char *)(obj + 32) == 4)
        return 0;
    f22 = 0.0f;
    if (x == 0.0f)
        x = F213135aa_FUN_002A7878(-30.0f, 30.0f) * 1.7453294247388839721680e-02f;
    if (y == 0.0f)
        y = F213135aa_FUN_002A7878(f22, 30.0f) * 1.7453294247388839721680e-02f;
    s3 = s1 + 32;
    if (*(unsigned char *)(obj + 32) == 3) {
        F213135aa_FUN_002833F0((char *)&m0, s1 + 16);
        F213135aa_FUN_00282F20(s3, (char *)&m2, 1.3333334028720855712891e-01f);
    }
    *(float *)(s1 + 100) = f22;
    *(float *)(s1 + 76) = x;
    *(float *)(s1 + 72) = y;
    *(char **)(s1 + 96) = obj2;
    F213135aa_FUN_00282CF0((char *)&m1, obj2 + 16, obj + 16);
    s1 = s1 + 48;
    F213135aa_FUN_00282DD8((char *)&m0, s3, (char *)&m1);
    F213135aa_FUN_00282F20(s1, (char *)&m0, 1.0f);
    F213135aa_FUN_002AB668(s1, s1, 2.0943954586982727050781e-01f);
    *(unsigned char *)(obj + 32) = 4;
    return 1;
}

INCLUDE_ASM("asm/game", func_00302440);

INCLUDE_ASM("asm/game", func_00302470);

INCLUDE_ASM("asm/game", func_00302848);

INCLUDE_ASM("asm/game", func_00302A20);

INCLUDE_ASM("asm/game", func_00302A88);

INCLUDE_ASM("asm/game", func_00302DA0);

INCLUDE_ASM("asm/game", func_00303070);

INCLUDE_ASM("asm/game", func_00303210);

INCLUDE_ASM("asm/game", func_00303568);

INCLUDE_ASM("asm/game", func_00303738);

INCLUDE_ASM("asm/game", func_00303AC8);

INCLUDE_ASM("asm/game", func_00303BF8);

INCLUDE_ASM("asm/game", func_00303DA8);

INCLUDE_ASM("asm/game", func_00303E20);

INCLUDE_ASM("asm/game", func_00304128);

INCLUDE_ASM("asm/game", func_00304210);

INCLUDE_ASM("asm/game", func_00304AD0);

void func_00304B68(char *object)
{
    char local[16];
    float base;
    int value = *(int *)(object + 104);

    if (F6eb4f363_FUN_002A77E0(5))
        return;
    base = F6eb4f363_FUN_002A90F8(object + 16, local);
    F6eb4f363_FUN_00282D30(local, value, 0.25f);
    F6eb4f363_FUN_002BF728(object + 16, local, F6eb4f363_FUN_002A7878(0.05f, 0.1f) * 210000.0f, base);
}

INCLUDE_ASM("asm/game", func_00304C18);

INCLUDE_ASM("asm/game", func_00304C78);

INCLUDE_ASM("asm/game", func_00304D38);

INCLUDE_ASM("asm/game", func_00304F28);

INCLUDE_ASM("asm/game", func_00305058);

void func_00305228(Owner_F9e2cd219 *self)
{
    Worker_F9e2cd219 *child = self->child;

    F9e2cd219_FUN_002ACF68((char *)child + 48, (char *)child + 16, (char *)child + 32, 0,
            F9e2cd219_FUN_002A7798(child->field54));

    child->field54 = child->field54 + child->field58;
    F9e2cd219_FUN_00305310(self, (char *)child + 48);

    if (child->field54 > 0.9f)
        child->field50 = 1;
    else
        child->field50 = 0;

    if (child->field54 >= 1.0f) {
        child->field54 = 1.0f;
        child->field4C = 1;
    }
}

INCLUDE_ASM("asm/game", func_00305310);

INCLUDE_ASM("asm/game", func_00305408);

INCLUDE_ASM("asm/game", func_00305418);

INCLUDE_ASM("asm/game", func_00305438);

INCLUDE_ASM("asm/game", func_003054E8);

INCLUDE_ASM("asm/game", func_00305798);

void func_00305838(char *p)
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

INCLUDE_ASM("asm/game", func_003058C8);

INCLUDE_ASM("asm/game", func_00305910);

INCLUDE_ASM("asm/game", func_00305C10);

INCLUDE_ASM("asm/game", func_00306040);

INCLUDE_ASM("asm/game", func_00306318);

INCLUDE_ASM("asm/game", func_00306680);

INCLUDE_ASM("asm/game", func_00306A40);

INCLUDE_ASM("asm/game", func_00307298);

void func_00307330(char *object)
{
    char local[16];
    float base;
    int value = *(int *)(object + 104);

    if (F6eb4f363_FUN_002A77E0(5))
        return;
    base = F6eb4f363_FUN_002A90F8(object + 16, local);
    F6eb4f363_FUN_00282D30(local, value, 0.25f);
    F6eb4f363_FUN_002BF728(object + 16, local, F6eb4f363_FUN_002A7878(0.05f, 0.1f) * 210000.0f, base);
}

INCLUDE_ASM("asm/game", func_003073E0);

INCLUDE_ASM("asm/game", func_00307440);

INCLUDE_ASM("asm/game", func_00307500);

INCLUDE_ASM("asm/game", func_003076F0);

INCLUDE_ASM("asm/game", func_00307808);

void func_003079E8(Owner_F9e2cd219 *self)
{
    Worker_F9e2cd219 *child = self->child;

    F9e2cd219_FUN_002ACF68((char *)child + 48, (char *)child + 16, (char *)child + 32, 0,
            F9e2cd219_FUN_002A7798(child->field54));

    child->field54 = child->field54 + child->field58;
    F9e2cd219_FUN_00307AD0(self, (char *)child + 48);

    if (child->field54 > 0.9f)
        child->field50 = 1;
    else
        child->field50 = 0;

    if (child->field54 >= 1.0f) {
        child->field54 = 1.0f;
        child->field4C = 1;
    }
}

INCLUDE_ASM("asm/game", func_00307AD0);

INCLUDE_ASM("asm/game", func_00307BD0);

INCLUDE_ASM("asm/game", func_00307BF0);

void func_00307C30(char *p)
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

INCLUDE_ASM("asm/game", func_00307CC0);

INCLUDE_ASM("asm/game", func_00307FC0);

INCLUDE_ASM("asm/game", func_003083F8);

INCLUDE_ASM("asm/game", func_00308408);

INCLUDE_ASM("asm/game", func_003085C8);
