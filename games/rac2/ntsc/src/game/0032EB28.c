#include "include_asm.h"

extern void F7242f0a4_FUN_00282D30(char *out, void *source, float value);

extern void F7242f0a4_FUN_00334730(char *buffer, int mode);

extern void F7242f0a4_FUN_00334710(int value, char *buffer);

extern void F7242f0a4_FUN_00282CC0(char *first, char *second, char *third);

extern float F7242f0a4_FUN_00282DB0(void *owner, char *buffer);

extern int F7242f0a4_D_001B53A0[];

typedef float f32;

typedef int s32;

extern s32 func_00283D38(s32, s32, f32);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_81be3b52930eb806_FallbackInterleavedFloat4Layout {
    unsigned char unknown_00[8];
    float first_0;
    float second_0;
    float first_1;
    float second_1;
    float first_2;
    float second_2;
    float first_3;
    float second_3;
} Rac2Native_81be3b52930eb806_FallbackInterleavedFloat4Layout;

typedef unsigned char u8;

/* func_0034CA68: owner ABI/declaration review retained. */
extern int func_00335E20(unsigned char *field);

extern void func_00336ED0(int a, int b);

#ifndef RAC2_T_OBJ_F7B2F1854
#define RAC2_T_OBJ_F7B2F1854
typedef struct Obj_F7b2f1854 {
    int *f0;
    int *f4;
    int *f8;
    int *f12;
    int *f16;
    int f20;
    int f24;
    int f28;
    int f32;
    int f36;
    int f40;
    int f44;
} Obj_F7b2f1854;
#endif

extern int *F7b2f1854_FUN_00336E40(int n);

extern int *F7b2f1854_FUN_00336DA8(int size, int *p);

extern void F7b2f1854_FUN_00335E68(Obj_F7b2f1854 *o, int v);

extern void F36efd143_FUN_00335FC8(char *a);

extern void *F36efd143_FUN_00336E40(char *a);

extern void *F36efd143_FUN_00336DA8(int size, void *b);

typedef unsigned int u32;

extern void F081de6c9_FUN_00335FC8(char *s, int a1, char *a2);

extern int F081de6c9_FUN_00336E40(int x);

extern int *F081de6c9_FUN_00336DA8(int n, int x);

INCLUDE_ASM("asm/game", func_0032EB28);

INCLUDE_ASM("asm/game", func_0032F3D0);

INCLUDE_ASM("asm/game", func_0032F590);

INCLUDE_ASM("asm/game", func_0032F598);

INCLUDE_ASM("asm/game", func_0032FDC0);

INCLUDE_ASM("asm/game", func_0032FDD0);

INCLUDE_ASM("asm/game", func_0032FEA8);

INCLUDE_ASM("asm/game", func_0032FFD8);

INCLUDE_ASM("asm/game", func_00330150);

INCLUDE_ASM("asm/game", func_003304D8);

INCLUDE_ASM("asm/game", func_003309A0);

INCLUDE_ASM("asm/game", func_00330BC0);

INCLUDE_ASM("asm/game", func_00330CA0);

INCLUDE_ASM("asm/game", func_00330E48);

INCLUDE_ASM("asm/game", func_00330EC0);

INCLUDE_ASM("asm/game", func_00331238);

INCLUDE_ASM("asm/game", func_003313E0);

INCLUDE_ASM("asm/game", func_00331580);

INCLUDE_ASM("asm/game", func_00331718);

INCLUDE_ASM("asm/game", func_003317E0);

INCLUDE_ASM("asm/game", func_00331800);

INCLUDE_ASM("asm/game", func_003320E8);

INCLUDE_ASM("asm/game", func_003320F0);

INCLUDE_ASM("asm/game", func_003323F0);

INCLUDE_ASM("asm/game", func_00332440);

INCLUDE_ASM("asm/game", func_00332588);

INCLUDE_ASM("asm/game", func_00332590);

INCLUDE_ASM("asm/game", func_00332658);

INCLUDE_ASM("asm/game", func_00333048);

INCLUDE_ASM("asm/game", func_003334B0);

INCLUDE_ASM("asm/game", func_00333AF8);

INCLUDE_ASM("asm/game", func_00334050);

INCLUDE_ASM("asm/game", func_003340A8);

INCLUDE_ASM("asm/game", func_003340F0);

INCLUDE_ASM("asm/game", func_00334100);

INCLUDE_ASM("asm/game", func_00334160);

INCLUDE_ASM("asm/game", func_00334550);

INCLUDE_ASM("asm/game", func_00334710);

INCLUDE_ASM("asm/game", func_00334728);

INCLUDE_ASM("asm/game", func_00334730);

INCLUDE_ASM("asm/game", func_00334898);

INCLUDE_ASM("asm/game", func_003348A8);

void func_003349E8(void *owner, int flag, float value)
{
    char buffer[32];
    int *root = F7242f0a4_D_001B53A0;

    F7242f0a4_FUN_00282D30(buffer, root + 8, value);
    if (flag)
        F7242f0a4_FUN_00334730(buffer + 16, 1);
    else
        F7242f0a4_FUN_00334710(root[-4], buffer + 16);
    F7242f0a4_FUN_00282CC0(buffer, buffer, buffer + 16);
    F7242f0a4_FUN_00282D30(owner, root + 12, F7242f0a4_FUN_00282DB0(root + 12, buffer));
}

INCLUDE_ASM("asm/game", func_00334A88);

INCLUDE_ASM("asm/game", func_003351B0);

INCLUDE_ASM("asm/game", func_00335270);

INCLUDE_ASM("asm/game", func_00335370);

INCLUDE_ASM("asm/game", func_003353C8);

INCLUDE_ASM("asm/game", func_00335420);

INCLUDE_ASM("asm/game", func_00335438);

INCLUDE_ASM("asm/game", func_003355A8);

INCLUDE_ASM("asm/game", func_003356E8);

INCLUDE_ASM("asm/game", func_00335798);

INCLUDE_ASM("asm/game", func_00335840);

INCLUDE_ASM("asm/game", func_00335860);

INCLUDE_ASM("asm/game", func_00335870);

INCLUDE_ASM("asm/game", func_003358A0);

INCLUDE_ASM("asm/game", func_00335918);

INCLUDE_ASM("asm/game", func_00335960);

INCLUDE_ASM("asm/game", func_00335AC8);

INCLUDE_ASM("asm/game", func_00335AE0);

/* Interpolate four scalar float components from separate input arrays using (1.0f - factor) * first + factor * second, writing each component to the destination. */
void func_00335B10(float factor, void *context,
                                   float *destination,
                                   const float *first, const float *second)
{
    float complement = 1.0f - factor;
    destination[0] = complement * first[0] + factor * second[0];
    destination[1] = complement * first[1] + factor * second[1];
    destination[2] = complement * first[2] + factor * second[2];
    destination[3] = complement * first[3] + factor * second[3];
}

/* calls one four-argument helper four times, left[i] and right[i] with the float scale, storing each result into dst[i]; the first pointer parameter is unused. */
void func_00335B80(s32 *unused, s32 *dst, s32 *left, s32 *right, f32 scale) {
    dst[0] = func_00283D38(left[0], right[0], scale);
    dst[1] = func_00283D38(left[1], right[1], scale);
    dst[2] = func_00283D38(left[2], right[2], scale);
    dst[3] = func_00283D38(left[3], right[3], scale);
}

INCLUDE_ASM("asm/game", func_00335C10);

INCLUDE_ASM("asm/game", func_00335C60);

/* Interpolate four scalar float components from interleaved endpoint pairs at source offsets 0x08 through 0x24 using (1.0f - factor) * first + factor * second, writing each component to the destination. */
void func_00335CC0(float factor,
                                     const Rac2Native_81be3b52930eb806_FallbackInterleavedFloat4Layout *source,
                                     float *destination)
{
    float complement = 1.0f - factor;
    destination[0] = complement * source->first_0 + factor * source->second_0;
    destination[1] = complement * source->first_1 + factor * source->second_1;
    destination[2] = complement * source->first_2 + factor * source->second_2;
    destination[3] = complement * source->first_3 + factor * source->second_3;
}

INCLUDE_ASM("asm/game", func_00335D30);

INCLUDE_ASM("asm/game", func_00335D60);

INCLUDE_ASM("asm/game", func_00335D80);

INCLUDE_ASM("asm/game", func_00335DA0);

INCLUDE_ASM("asm/game", func_00335DC0);

INCLUDE_ASM("asm/game", func_00335E08);

/* Measured target: byte-identical across all 27 levels. */
s32 func_00335E10(u8 *a0) { return *(s32 *)(a0 + 0x0); }

/* Measured target: byte-identical across all 27 levels. */
s32 func_00335E18(u8 *a0) { return *(s32 *)(a0 + 0x4); }

/* Measured target: byte-identical across all 27 levels. */
s32 func_00335E20(u8 *a0) { return *(s32 *)(a0 + 0xc); }

INCLUDE_ASM("asm/game", func_00335E28);

void func_00335E38(u8 *a0,f32 f0,f32 f1,f32 f2,f32 f3) {
 (*(f32 **)a0)[0]=f0;(*(f32 **)a0)[1]=f1;
 (*(f32 **)a0)[2]=f2;(*(f32 **)a0)[3]=f3;
}

INCLUDE_ASM("asm/game", func_00335E60);

void func_00335E68(u8 *a0, s32 a1) {
 f32 *p=*(f32 **)(a0+0x10);
 *p=(a1!=0)?1.0f:0.0f;
}

s32 func_00335E88(u8 *a0) {
 return 0.0f<**(f32 **)(a0+0x10);
}

/* Compares the second argument with the word at offset 4 of the object; when they differ, and when the word at offset 44 is non-zero while the word at offset 24 is zero, it calls a two-argument helper with those two words, marks offset 24 with 1, and finally writes the second argument into offset 4. */
void func_00335EB0(char *a0, int a1)
{
    if (a1 != *(int *)(a0 + 4)) {
        if (*(int *)(a0 + 44) != 0 && *(int *)(a0 + 24) == 0) {
            func_00336ED0(*(int *)(a0 + 44), *(int *)(a0 + 4));
            *(int *)(a0 + 24) = 1;
        }
        *(int *)(a0 + 4) = a1;
    }
}

INCLUDE_ASM("asm/game", func_00335F18);

INCLUDE_ASM("asm/game", func_00335F20);

void func_00335F88(u8 *a0,f32 f0,f32 f1,f32 f2,f32 f3) {
 (*(f32 **)(a0+4))[0]=f0;(*(f32 **)(a0+4))[1]=f1;
 (*(f32 **)(a0+4))[2]=f2;(*(f32 **)(a0+4))[3]=f3;
}

INCLUDE_ASM("asm/game", func_00335FB0);

void func_00335FC8(Obj_F7b2f1854 *o, int a1, int count)
{
    int *p;
    int n;

    o->f44 = count;
    if (count != 0) {
        p = F7b2f1854_FUN_00336DA8(16, F7b2f1854_FUN_00336E40(count));
        n = o->f44;
        o->f0 = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;

        p = F7b2f1854_FUN_00336DA8(16, F7b2f1854_FUN_00336E40(n));
        n = o->f44;
        o->f8 = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;

        p = F7b2f1854_FUN_00336DA8(16, F7b2f1854_FUN_00336E40(n));
        n = o->f44;
        o->f4 = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;

        p = F7b2f1854_FUN_00336DA8(16, F7b2f1854_FUN_00336E40(n));
        n = o->f44;
        o->f12 = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;

        p = F7b2f1854_FUN_00336DA8(16, F7b2f1854_FUN_00336E40(n));
        o->f16 = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;
    }
    o->f40 = a1;
    o->f32 = 0;
    o->f20 = 0;
    o->f28 = 0;
    o->f24 = 0;
    o->f36 = 0;
    F7b2f1854_FUN_00335E68(o, 1);
}

INCLUDE_ASM("asm/game", func_003360F0);

INCLUDE_ASM("asm/game", func_003360F8);

INCLUDE_ASM("asm/game", func_003361A8);

void func_003361E0(char *obj)
{
    int *p;
    int x;
    F36efd143_FUN_00335FC8(obj);
    if (*(int *)(obj + 44) != 0) {
        p = (int *)F36efd143_FUN_00336DA8(16, F36efd143_FUN_00336E40(*(int *)(obj + 44)));
        *(int *)(obj + 52) = (int)p;
        x = *(int *)(obj + 44);
        p[1] = 0; p[2] = 0; p[3] = 0; p[0] = 0;
        p = (int *)F36efd143_FUN_00336DA8(16, F36efd143_FUN_00336E40(x));
        *(int *)(obj + 56) = (int)p;
        {
            float *f = (float *)*(int *)(obj + 4);
            p[1] = 0;
            p[2] = 0;
            p[3] = 0;
            p[0] = 0;
            f[0] = 1.0f;
            *(float *)(*(int *)(obj + 4) + 4) = 1.0f;
            *(float *)(*(int *)(obj + 4) + 8) = 1.0f;
        }
    }
    *(int *)(obj + 68) = 0;
    *(int *)(obj + 72) = 0;
}

INCLUDE_ASM("asm/game", func_00336288);

INCLUDE_ASM("asm/game", func_00336308);

/* Measured target: byte-identical across all 27 levels. */
s32 func_00336318(u8 *a0) { return *(s32 *)(a0 + 0x38); }

INCLUDE_ASM("asm/game", func_00336320);

INCLUDE_ASM("asm/game", func_00336390);

void func_003363C0(u8 *a0, f32 a1) { f32 *p = *(f32 **)(a0 + 0x38); *p = a1; }

INCLUDE_ASM("asm/game", func_003363D0);

INCLUDE_ASM("asm/game", func_00336408);

INCLUDE_ASM("asm/game", func_00336468);

/* lot12 -- measured bodies with sizes recorded in the catalogue. */
void func_00336498(u8 *a0, s32 a1, s32 a2) {
    *(s32 *)*(u8 **)(a0 + 0xc) = a1;
    *(s32 *)(*(u8 **)(a0 + 0xc) + 0x4) = a2;
}

void func_003364B0(u8 *a0, s32 a1, s32 a2) {
    *(s32 *)(*(u8 **)(a0 + 0xc) + 0x8) = a1;
    *(s32 *)(*(u8 **)(a0 + 0xc) + 0xc) = a2;
}

void func_003364C8(u8 *a0, s32 a1, s32 a2) {
    u8 *o = a0;
    *(u32 *)*(u8 **)(o + 0xC) = (*(u32 *)*(u8 **)(o + 0xC) & 0x00FFFFFF) | (u32)(a1 << 24);
    *((u32 *)*(u8 **)(o + 0xC) + 1) = (*((u32 *)*(u8 **)(o + 0xC) + 1) & 0x00FFFFFF) | (u32)(a2 << 24);
}

void func_00336508(u8 *a0, s32 a1, s32 a2) {
    u8 *o = a0;
    *((u32 *)*(u8 **)(o + 0xC) + 2) = (*((u32 *)*(u8 **)(o + 0xC) + 2) & 0x00FFFFFF) | (u32)(a1 << 24);
    *((u32 *)*(u8 **)(o + 0xC) + 3) = (*((u32 *)*(u8 **)(o + 0xC) + 3) & 0x00FFFFFF) | (u32)(a2 << 24);
}

INCLUDE_ASM("asm/game", func_00336548);

INCLUDE_ASM("asm/game", func_003366D0);

INCLUDE_ASM("asm/game", func_00336708);

void func_00336710(char *s1, int a1, char *a2)
{
    F081de6c9_FUN_00335FC8(s1, a1, a2);
    if (a2 != 0) {
        int *v = F081de6c9_FUN_00336DA8(16, F081de6c9_FUN_00336E40(*(int *)(s1 + 44)));

        *(int **)(s1 + 52) = v;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
        v[0] = 0;
    }
    ((int *)*(int **)(s1 + 52))[0] = 0;
    ((int *)*(int **)(s1 + 52))[1] = 0;
    *(float *)(*(char **)s1 + 0) = 100.0f;
    *(float *)(*(char **)s1 + 4) = 100.0f;
    *(float *)(*((char **)s1 + 1) + 0) = 64.0f;
    *(float *)(*((char **)s1 + 1) + 4) = 64.0f;
    *(int *)(s1 + 56) = 0;
}

/* Measured target: byte-identical across all 27 levels. */
s32 func_003367B8(u8 *a0) { return *(s32 *)(a0 + 0x34); }

INCLUDE_ASM("asm/game", func_003367C0);
