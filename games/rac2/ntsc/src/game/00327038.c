#include "include_asm.h"

#define RAC2_T_VEC16_FC936841D

typedef struct {
    char pad_000[0x140];
    float x_140;
    float y_144;
    float z_148;
} BootAimVector;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;

typedef unsigned char u8;

typedef int s32;

typedef struct {
    u8 field00[0x14];
    void *field14;
    u8 field18[8];
} NativeObjectSearchRecord32;

extern BootAimVector D_001B5200;

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

extern NativeObjectSearchRecord32 D_L00_002533B0[];

extern unsigned char *D_001BAF98[];

extern char D_001A63A8[];

extern char D_001B01F0[];

extern int func_002B6D28(void);

extern void func_002A8598(int, int);

extern void func_0026FEB8(const char *, ...);

extern float func_002A7878(float, float);

extern void func_00282F20(float *, float *, float);

extern char D_0022B0A0[];

extern void func_00283098(void *, void *, void *);

extern char D_L00_00252C40[][16];

extern char D_L00_00189F00[];

extern float func_002A7878(float, float);

extern void func_002AF000(void *, float, float, float);

extern void func_00282CC0(void *, void *, void *);

#ifndef RAC2_T_VEC16_FC936841D
#endif

/* Finds the first matching object in the thirty resident records. */

INCLUDE_ASM("asm/game", func_00327038);

INCLUDE_ASM("asm/game", func_00327380);

INCLUDE_ASM("asm/game", func_00327848);

/* func_00327A60 uses small data in 00327038-g8.c. */

INCLUDE_ASM("asm/game", func_00327B40);

/* func_00327D08 is matched in 00327038-g8.c (-G8 small data). */

INCLUDE_ASM("asm/game", func_00327D50);

INCLUDE_ASM("asm/game", func_003280C8);

INCLUDE_ASM("asm/game", func_003281F0);

INCLUDE_ASM("asm/game", func_003285C0);

INCLUDE_ASM("asm/game", func_003286D0);

void func_00328758(char *self)
{
    unsigned char *object = D_001BAF98[0];
    char *owner = *(char **)(self + 0x68);
    unsigned char *state;
    char *context;
    if (object == 0)
        return;
    if (*(short *)(*(char **)(object + 0x24) + 0x46) != 18)
        return;
    state = *(unsigned char **)(object + 0x68);
    if (state != 0) {
        if (*(short *)(state + 0x16) == 0) {
            if (state[0x14] != 0) {
                object[0xBC] += 2;
                if (object[0xBC] >= 25) {
                    context = D_001A63A8;
                    if (*(short *)(context + 0x72) == 3 && *(short *)(context + 0x6C) < 6000)
                        func_002B6D28();
                }
                if (object[0x21] != 255)
                    func_002A8598(object[0x21], object[0xBC]);
            }
            *(int *)(owner + 0x54) = 0;
        } else {
            *(int *)(owner + 0x54) = 1;
        }
    } else {
        func_0026FEB8(D_001B01F0, *(short *)(object + 0xAA));
    }
}

void func_00328848(char *a0)
{
    char *rec = D_0022B0A0;
    int *owner = *(int **)(a0 + 0x68);
    int i;

    for (i = 0; i < 100; i++) {
        if (*(int *)(rec + 0x34) != 0) {
            rec += 0x40;
            continue;
        }
        *(int *)(rec + 0x34) = (int)a0;
        *(int *)(rec + 0x30) = 0;
        *(float *)(rec + 0x0) = func_002A7878(-1.0f, 1.0f);
        *(float *)(rec + 0x4) = func_002A7878(-1.0f, 1.0f);
        *(float *)(rec + 0x8) = func_002A7878(-1.0f, 1.0f);
        func_00282F20((float *)rec, (float *)rec, 0.1275f);
        owner[0x48 / 4]++;
        break;
    }
}

void func_00328918(char *owner)
{
    int address = (int)D_0022B0A0;
    char *position = (char *)(address + 0x10);
    char *other = (char *)(address + 0x20);
    int remaining = 99;
    char *base = (char *)address;
    char *data = *(char **)(owner + 0x68);
    do {
        if (*(char **)(position + 0x24) == owner) {
            func_00283098(position, base, owner + 0xC0);
            func_00282F20((float *)other, (float *)position, 1.0f);
            func_00282CC0(position, position, owner + 0x10);
            *(float *)(position + 0x20) += 0.6283186078071594f;
            if (*(float *)(position + 0x20) >= 6.2831854820251465f) {
                *(int *)(position + 0x24) = 0;
                --*(int *)(data + 0x48);
            }
        }
        base += 0x40;
        other += 0x40;
        position += 0x40;
    } while (--remaining >= 0);
}

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

void func_0032A978(void)
{
    int i;
    char *row = (char *)D_L00_00252C40;
    for (i = 49; i >= 0; i--) {
        float first = func_002A7878(0.01f, 7.0f);
        float second = func_002A7940();
        float third = func_002A7878(0.0f, 3.1415927f);
        func_002AF000(row, first, second, third);
        func_00282CC0(row, row, D_L00_00189F00);
        row += 16;
    }
}

INCLUDE_ASM("asm/game", func_0032AA30);

INCLUDE_ASM("asm/game", func_0032ABC0);


INCLUDE_ASM("asm/game", func_0032AE30);



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
