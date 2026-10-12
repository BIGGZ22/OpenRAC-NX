#include "include_asm.h"

extern void F2c74c194_FUN_00282D30(char *dst, char *src, float scale);

extern void F8411efa9_FUN_00328CA8(char *pkt);

extern long long F8411efa9_FUN_0027C878(char *p);

extern float F8411efa9_FUN_00283248(float x, float y);

extern float F8411efa9_FUN_00282E20(char *p);

extern void F8411efa9_FUN_00283410(float *matrix, float *quat);

extern void F8411efa9_FUN_00282F20(float *off, char *src, float scale);

extern void F8411efa9_FUN_00328D30(char *pkt, float angle);

extern void F8411efa9_FUN_00280B90(char *pkt, float *matrix, int mode);

extern float func_002A7940(void);

extern float func_002A7878_1(float a, float b);

extern float func_00283180(float a);

extern float func_00283198(float a);

extern int func_002A7820(int a, int b);

extern int func_002A77E0(int a);

extern void func_002BFB38(char *p, float *q, int a, int b);

extern void Fc936841d_FUN_00282CF0(char *local, char *first, char *second);

extern void Fc936841d_FUN_00282CC0(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

typedef unsigned char u8;

typedef int s32;

/* Finds the first matching object in the thirty resident records. */
typedef struct {
    u8 field00[0x14];
    void *field14;
    u8 field18[8];
} NativeObjectSearchRecord32;

extern NativeObjectSearchRecord32 D_L00_002533B0[];

INCLUDE_ASM("asm/game", func_00327038);

INCLUDE_ASM("asm/game", func_00327380);

INCLUDE_ASM("asm/game", func_00327848);

INCLUDE_ASM("asm/game", func_00327A60);

INCLUDE_ASM("asm/game", func_00327B40);

INCLUDE_ASM("asm/game", func_00327D08);

INCLUDE_ASM("asm/game", func_00327D50);

INCLUDE_ASM("asm/game", func_003280C8);

INCLUDE_ASM("asm/game", func_003281F0);

INCLUDE_ASM("asm/game", func_003285C0);

INCLUDE_ASM("asm/game", func_003286D0);

INCLUDE_ASM("asm/game", func_00328758);

INCLUDE_ASM("asm/game", func_00328848);

INCLUDE_ASM("asm/game", func_00328918);

INCLUDE_ASM("asm/game", func_00328A20);

INCLUDE_ASM("asm/game", func_00328B68);

void func_00328CA8(char *p)
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

void func_00328D30(char *p, float scale)
{
    F2c74c194_FUN_00282D30(p, p, scale);
    F2c74c194_FUN_00282D30(p + 16, p + 16, scale);
    F2c74c194_FUN_00282D30(p + 32, p + 32, scale);
    F2c74c194_FUN_00282D30(p + 48, p + 48, scale);
}

void func_00328D98(char *pos, int a1, int a2, char *a3, int t0,
                                        char *src, float f12, float f13)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    F8411efa9_FUN_00328CA8(pkt);
    r = F8411efa9_FUN_0027C878(a3);

    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;

    quat[3] = 0.0f;
    quat[2] = F8411efa9_FUN_00283248(*(float *)(src + 0), *(float *)(src + 4));
    quat[1] = -F8411efa9_FUN_00283248(F8411efa9_FUN_00282E20(src), *(float *)(src + 8));
    quat[0] = f13;

    F8411efa9_FUN_00283410(m, quat);
    F8411efa9_FUN_00282F20(off, src, 0.025f);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    F8411efa9_FUN_00328D30(pkt, f12);
    F8411efa9_FUN_00280B90(pkt, m, 0);
}

INCLUDE_ASM("asm/game", func_00328F18);

INCLUDE_ASM("asm/game", func_003290D0);

INCLUDE_ASM("asm/game", func_00329328);

INCLUDE_ASM("asm/game", func_00329488);

INCLUDE_ASM("asm/game", func_00329630);

INCLUDE_ASM("asm/game", func_00329768);

INCLUDE_ASM("asm/game", func_00329CB8);

INCLUDE_ASM("asm/game", func_00329DC0);

/* Per frame: scales the value one helper returns by the factor another helper returns, stores both scaled samples and a third sample into a three-float record, and hands the record to a setter with two selector results. */
void func_00329E60(char *a0)
{
    float v[3];
    float a;
    float b;
    int i;
    int m;
    int n;

    for (i = 47; i >= 0; i--) {
        a = func_002A7940();
        b = func_002A7878_1(0.0f, 0.050000004f);
        v[0] = func_00283180(a) * b;
        v[1] = func_00283198(a) * b;
        v[2] = func_002A7878_1(0.050000004f, 1.0833334177732467651367e-01f);
        m = func_002A7820(90, 120);
        n = func_002A77E0(2);
        func_002BFB38(a0 + 16, v, n, m);
    }
}

INCLUDE_ASM("asm/game", func_00329F40);

INCLUDE_ASM("asm/game", func_0032A130);

INCLUDE_ASM("asm/game", func_0032A140);

INCLUDE_ASM("asm/game", func_0032A978);

INCLUDE_ASM("asm/game", func_0032AA30);

INCLUDE_ASM("asm/game", func_0032ABC0);

INCLUDE_ASM("asm/game", func_0032ACF0);

INCLUDE_ASM("asm/game", func_0032AE30);

INCLUDE_ASM("asm/game", func_0032AF50);

INCLUDE_ASM("asm/game", func_0032B0A8);

INCLUDE_ASM("asm/game", func_0032B190);

void func_0032B2F0(char *p)
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

void func_0032B378(char *p, float scale)
{
    F2c74c194_FUN_00282D30(p, p, scale);
    F2c74c194_FUN_00282D30(p + 16, p + 16, scale);
    F2c74c194_FUN_00282D30(p + 32, p + 32, scale);
    F2c74c194_FUN_00282D30(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/game", func_0032B3E0);

INCLUDE_ASM("asm/game", func_0032B598);

INCLUDE_ASM("asm/game", func_0032B7D0);

INCLUDE_ASM("asm/game", func_0032BA88);

INCLUDE_ASM("asm/game", func_0032BBF0);

void func_0032BDC8(Vec16_Fc936841d *object, int index)
{
    char local[16];

    Fc936841d_FUN_00282CF0(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    Fc936841d_FUN_00282CC0((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/game", func_0032BE30);

INCLUDE_ASM("asm/game", func_0032C2E8);

INCLUDE_ASM("asm/game", func_0032C688);

INCLUDE_ASM("asm/game", func_0032C700);

INCLUDE_ASM("asm/game", func_0032CA40);

INCLUDE_ASM("asm/game", func_0032D6F0);

/* finds the first matching object pointer in thirty resident32-byte records or returns-1 */
s32 func_0032D700(void *object) {
    s32 result = -1;
    s32 index;
    for (index = 0; index < 30; index++) {
        if (D_L00_002533B0[index].field14 == object) {
            result = index;
            break;
        }
    }
    return result;
}

INCLUDE_ASM("asm/game", func_0032D758);

INCLUDE_ASM("asm/game", func_0032DA50);

INCLUDE_ASM("asm/game", func_0032DA58);

INCLUDE_ASM("asm/game", func_0032DD40);

INCLUDE_ASM("asm/game", func_0032E4E8);

INCLUDE_ASM("asm/game", func_0032E4F0);

INCLUDE_ASM("asm/game", func_0032E568);

INCLUDE_ASM("asm/game", func_0032E588);

INCLUDE_ASM("asm/game", func_0032E700);
