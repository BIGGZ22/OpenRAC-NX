#include "include_asm.h"

extern void func_002833F0_1(char *a, char *b);

extern float func_00283198_1(float value);

extern void func_00282D30_3(char *a, char *b, float value);

extern float func_00283B98_3(float value, float scale);

extern int func_002ABB78(char *object);

extern float *func_002AB1F0(char *object);

extern void func_00282D30_2(char *dst, char *src, float scale);

extern float func_00283B98_2(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

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

extern int func_002A7820_2(int, int);

extern float func_002A7878_5(float, float);

extern float func_002A7940_2(void);

extern void func_00281EB8(char *, int, int, int, float, float, float);

INCLUDE_ASM("asm/levels", func_L00_003E3258);

INCLUDE_ASM("asm/levels", func_L00_003E3668);

/* Family 40487154aa250cc8: fixed-field addend writes into one resource record. */
void func_L00_003E3890(char *object)
{
    char *data;
    float x;
    float y;
    float z;

    data = *(char **)(object + 104);
    func_002833F0_1(object + 192, object + 240);
    x = func_00283198_1(*(float *)(data + 4)) * 0.15f + 0.35000002f;
    y = func_00283198_1(*(float *)(data + 8)) * 0.15f + 0.35000002f;
    z = func_00283198_1(*(float *)(data + 12)) * 0.15f + 0.35000002f;
    func_00282D30_3(object + 192, object + 192, x);
    func_00282D30_3(object + 208, object + 208, y);
    func_00282D30_3(object + 224, object + 224, z);
    *(float *)(data + 4) = func_00283B98_3(*(float *)(data + 4), 0.5f);
    *(float *)(data + 8) = func_00283B98_3(*(float *)(data + 8), 0.5f);
    *(float *)(data + 12) = func_00283B98_3(*(float *)(data + 12), 0.5f);
}

/* calls two guards and returns 1 only when both succeed and the object's float field is not positive */
int func_L00_003E39B8(char *object)
{
    float *p;

    if (func_002ABB78(object) != 0)
        return 0;
    p = func_002AB1F0(object);
    if (p == 0)
        return 0;
    if (0.0f < *p)
        return 0;
    return 1;
}

INCLUDE_ASM("asm/levels", func_L00_003E3A18);

INCLUDE_ASM("asm/levels", func_L00_003E4508);

INCLUDE_ASM("asm/levels", func_L00_003E4948);

INCLUDE_ASM("asm/levels", func_L00_003E4968);

INCLUDE_ASM("asm/levels", func_L00_003E4B80);

INCLUDE_ASM("asm/levels", func_L00_003E4BF0);

INCLUDE_ASM("asm/levels", func_L00_003E51C0);

INCLUDE_ASM("asm/levels", func_L00_003E5328);

INCLUDE_ASM("asm/levels", func_L00_003E5330);

INCLUDE_ASM("asm/levels", func_L00_003E5340);

INCLUDE_ASM("asm/levels", func_L00_003E55B0);

INCLUDE_ASM("asm/levels", func_L00_003E5650);

INCLUDE_ASM("asm/levels", func_L00_003E5710);

INCLUDE_ASM("asm/levels", func_L00_003E5978);

INCLUDE_ASM("asm/levels", func_L00_003E5BD8);

INCLUDE_ASM("asm/levels", func_L00_003E6030);

INCLUDE_ASM("asm/levels", func_L00_003E64F8);

INCLUDE_ASM("asm/levels", func_L00_003E6628);

INCLUDE_ASM("asm/levels", func_L00_003E6710);

INCLUDE_ASM("asm/levels", func_L00_003E6778);

INCLUDE_ASM("asm/levels", func_L00_003E6C10);

INCLUDE_ASM("asm/levels", func_L00_003E6F48);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003E6FC8(char *p)
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

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L00_003E7050(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L00_003E70B8);

INCLUDE_ASM("asm/levels", func_L00_003E7270);

INCLUDE_ASM("asm/levels", func_L00_003E7650);

void func_L00_003E7760(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003E7C88);

INCLUDE_ASM("asm/levels", func_L00_003E7E40);

INCLUDE_ASM("asm/levels", func_L00_003E80A8);

INCLUDE_ASM("asm/levels", func_L00_003E8150);

INCLUDE_ASM("asm/levels", func_L00_003E8180);

INCLUDE_ASM("asm/levels", func_L00_003E8A98);

INCLUDE_ASM("asm/levels", func_L00_003E9490);

INCLUDE_ASM("asm/levels", func_L00_003E9540);

/* Shared native body; original game role is unknown. */
void func_L00_003E9660(int unused, float *p, int n)
{
    float *v;
    int count;
    if (n <= 0)
        return;
    v = p;
    count = n;
    do {
        v[0] = func_00283B98_2(v[0], v[3]);
        v[1] = func_00283B98_2(v[1], v[4]);
        v[2] = func_00283B98_2(v[2], v[5]);
        v[9] = func_00283B98_2(v[9], v[12]);
        v[10] = func_00283B98_2(v[10], v[13]);
        v[11] = func_00283B98_2(v[11], v[14]);
        v[20] = func_00283180(v[0]) * v[6];
        v[21] = func_00283198(v[1]) * v[7];
        v[22] = -func_00283198(v[2]) * v[8];
        v[24] = func_00283180(v[9]) * v[15];
        v[25] = func_00283198(v[10]) * v[16];
        v[26] = -func_00283198(v[11]) * v[17];
        v += 32;
    } while (--count != 0);
}

INCLUDE_ASM("asm/levels", func_L00_003E9780);

INCLUDE_ASM("asm/levels", func_L00_003E9928);

INCLUDE_ASM("asm/levels", func_L00_003E9930);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003E9A98(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

/* Derives a value from half of one argument, merges a helper's low byte into a packed word, then scales an input float, doubles a second helper's result and dispatches a seven-argument spawn. */
void func_L00_003E9B00(int p0, int p1, int p2, int p3, float v)
{
    int r;
    float s;
    float t;

    r = func_002A7820_2(p3 >> 1, p3);
    p2 = p2 | (r << 24);
    s = func_002A7878_5(v * 0.75f, v);
    s = s + s;
    t = func_002A7940_2();
    func_00281EB8((char *)p1, 44, p2, 1, s, 0.08f, t);
}

INCLUDE_ASM("asm/levels", func_L00_003E9B90);

INCLUDE_ASM("asm/levels", func_L00_003EA048);

/* straight-line constant-store body (no calls, no branches, no loads) */
void func_L00_003EA050(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 88) = 0;
    *(float *)(p + 96) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(int *)(p + 84) = 0;
    *(float *)(p + 92) = 1.0f;
    *(int *)(p + 100) = 0;
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
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}
