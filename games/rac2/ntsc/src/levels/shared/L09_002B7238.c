#include "include_asm.h"

typedef signed char s8;

typedef unsigned char u8;

typedef unsigned short u16;

typedef signed short s16;

typedef signed int s32;

typedef unsigned int u32;

typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

typedef struct {
    char b0[16];
    char b1[16];
    char b2[16];
    char b3[16];
    int k0, k1, k2, k3;
    int z80, z84, z88;
    float f92, f96;
    int z100;
    float f104, f108;
    long long v112, v120, v128, v136;
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float v192, v196, v200, v204;
    float v208, f212, f216, v220;
} LOC_L09_003AF508;

typedef unsigned long long u64;


extern unsigned int D_L09_001FE6C0[];

extern unsigned int D_L09_001FB560[];

extern char *D_L09_001B285C __attribute__((sda));

extern int func_002AD980(void);

extern int D_0018C0B0;

extern int func_002B6120(char *object, unsigned int entry);

extern int func_002B6228(char *object, unsigned int entry);

extern int func_L06_00382850(char *object, unsigned int entry);

extern float func_L01_003377C8(void *entry);

extern int func_L01_00337888(char *object, unsigned int entry);

extern void func_00282CC0(char *first, char *second, char *third);

extern void func_00282DD8(char *first, char *second, char *third);

extern void func_00282F20(char *dst, char *src, float x);

extern float func_002A7940(void);

extern void func_002AB668(void *a, void *b, float f);

extern void func_00282D30(char *a, char *b, float f);

extern void func_002ACDE8(char *a, char *b, char *c);

extern float func_00283BE0(float a, float b);

extern void func_L00_002E6CF0(char *table, char *object);

extern char D_L09_003AEF78[];

extern void func_L00_003C1D30(void *p);

extern long long func_0027C878(int id);

extern int func_002A7820(int lo, int hi);

extern float func_00283248(float x, float y);

extern float func_00282E80_1(char *a, char *b);

extern float func_002A7940(void);

extern void func_00283410(void *dst, void *src);

extern float func_002A7878(float lo, float hi);

extern void func_00280B90(void *dst, void *src, int flag);

extern char D_L09_001BF98C0[];

extern void func_003AEFD0(void *out);

extern float func_00316AC0(float lo, float hi);

extern void func_002EA4D0(char *dst, char *src, char *extra, float f);

extern int func_002E2828(int id);

extern int func_001163B0(void);

extern int func_00316A68(int lo, int hi);

extern float func_002EAB28(float a, float b);

extern float func_00316B88(float f);

extern void func_002EACF0(char *dst, char *src);

extern void func_002E6C68(char *dst, char *src, int flag);

extern void func_003AF9E8(char *object, char *v);

extern void func_0031F728(char *object, char *a1, char *a2, float f1, float f2, float f3);

extern void func_0031F7E0(char *object, char *a1, char *a2, float f1, float f2, float f3);

extern float func_002EB478(float a, float b);

extern int func_L09_0033E8B0_1(char *object, int index, void *out, int a, int b, float f);

extern void func_L09_003E9E60(char *object, void *buf, float f);

extern float func_00282E48_2(char *a, int b);

extern int func_L06_00382850_1(char *object, unsigned int entry, float value);

extern int func_002B6228_1(char *object, unsigned int entry, int value, int flag);

extern void func_00282978(void *p);

extern void func_002829A8(void *p);

extern void func_002E5CF8(int a, int b, char *c);

extern void func_002A6C58(char *a, int b, int c);

extern void func_0029F338(void *entry);

extern void func_0032C700(void *p, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                          float f1, float f2, int s1, int s2, int s3, int s4);

extern float func_00282E48(char *a, char *b);

extern void func_002A93B0(void *object);

extern char D_L09_001B9A00[];

extern void func_00282948(void *a0);

extern void func_002A04B8(char *object, int *out0, int *out1, int *out2);

extern void func_002A0488(char *object, int a, int b, int c);

extern void func_0029F338(void *entry);

extern float func_00282E80(char *a, int b);

extern int func_L09_0033E690_1(char *object, int index, float value);

extern int func_L09_00329178_1(char *object, int index, int value, int flag);

extern void ref_L09_003F8858_00(char *object, int value);

/* index -> entry of a per-level table; a live entry's first word is a one-based id. */

INCLUDE_ASM("asm/levels", func_L09_002B7238);

INCLUDE_ASM("asm/levels", func_L09_00325C58);

INCLUDE_ASM("asm/levels", func_L09_00326E90);

int func_L09_00329028(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return 0;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return 0;
    return func_002B6120(object, entry);
}

int func_L09_00329178(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return 0;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return 0;
    return func_002B6228(object, entry);
}

INCLUDE_ASM("asm/levels", func_L09_003364E0);

/* func_L09_0033E658 is matched in L09_0033E658-rc2.c. */


int func_L09_0033E690(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return -1;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1;
    return func_L06_00382850(object, entry);
}

float func_L09_0033E7A0(int index)
{
    unsigned int entry;

    if (index == -1)
        return -1.0f;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1.0f;
    return func_L01_003377C8((void *)entry);
}

int func_L09_0033E8B0(char *object, int index)
{
    unsigned int entry;

    if (index == -1)
        return -1;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1;
    return func_L01_00337888(object, entry);
}

INCLUDE_ASM("asm/levels", func_L09_0037FCE8);

INCLUDE_ASM("asm/levels", func_L09_003806A0);

INCLUDE_ASM("asm/levels", func_L09_003A01D8);

INCLUDE_ASM("asm/levels", func_L09_003A06B0);

INCLUDE_ASM("asm/levels", func_L09_003AE8C8);

INCLUDE_ASM("asm/levels", func_L09_003AEA88);

void func_L09_003AEE10(char *object)
{
    char *p = *(char **)(object + 104);
    char *q = object + 0x10;

    func_00282CC0(p, q, p + 0x30);
    func_00282DD8(p, p, q);
    func_00282F20(p, p, 0.25f);
    *(float *)(p + 0x5C) = func_002A7940();
}

/* rebuilds the object's orientation matrix from its stored angles and hands it to the stage */
void func_L09_003AEE80(char *object)
{
    char *p = *(char **)(object + 0x68);
    float m[4];
    char *q = object + 0x10;
    char *r = p + 0x10;

    func_00282F20((char *)m, p + 0x30, 1.0f);
    func_002AB668(m, m, *(float *)(p + 0x5C));
    func_00282D30(r, p, *(float *)(p + 0x50));
    func_002ACDE8(r, r, (char *)m);
    *(float *)(p + 0x20) = -*(float *)(p + 0x10);
    *(float *)(p + 0x24) = -*(float *)(p + 0x14);
    *(float *)(p + 0x28) = -*(float *)(p + 0x18);
    func_00282CC0(r, q, r);
    func_00282CC0(p + 0x20, q, p + 0x20);
    *(float *)(p + 0x5C) = func_00283BE0(*(float *)(p + 0x5C), 0.3141593039035797f);
    func_L00_002E6CF0(D_L09_003AEF78, object);
}

INCLUDE_ASM("asm/levels", func_L09_003AF060);

/* the effect's record: four blocks, four colour words, a matrix and a vector */

/* measures the object against the level's target point and hands the effect to the stage */
void func_L09_003AF508(char *object)
{
    LOC_L09_003AF508 local;
    char *frame;
    char *data;
    float len;
    float scale;
    int color;

    frame = *(char **)(object + 0x68);
    func_L00_003C1D30(&local);
    local.v120 = func_0027C878(11);
    local.v128 = 0xFF9000000260LL;
    local.v136 = 0x0000008000000048LL;
    local.v112 = 0;
    color = (func_002A7820(0x20, 0x30) << 24) + 0x103080;
    local.k0 = local.k1 = local.k2 = local.k3 = color;
    data = D_L09_001BF98C0;
    local.v220 = 0.0f;
    local.f216 = func_00283248(*(float *)(data + 0x140) - *(float *)(object + 0x10),
                               *(float *)(data + 0x144) - *(float *)(object + 0x14));
    len = func_00282E80_1(object + 0x10, data + 0x140);
    local.f212 = -func_00283248(len, *(float *)(data + 0x148) - *(float *)(object + 0x18));
    local.v208 = func_002A7940();
    func_00283410(&local.m00, &local.v208);
    local.v192 = *(float *)(object + 0x10);
    local.v196 = *(float *)(object + 0x14);
    local.v200 = *(float *)(object + 0x18);
    local.v204 = 1.0f;
    scale = func_002A7878(0.35f, 0.45f) * *(float *)(frame + 0x50);
    func_00282D30((char *)&local.b0, (char *)&local.b0, scale);
    func_00282D30((char *)&local.b1, (char *)&local.b1, scale);
    func_00282D30((char *)&local.b2, (char *)&local.b2, scale);
    func_00282D30((char *)&local.b3, (char *)&local.b3, scale);
    func_00280B90(&local, &local.m00, 0);
}

/* builds the effect's frame, colour word and query box, then hands the result to the stage */
INCLUDE_ASM("asm/levels", func_L09_003AF6B0);

INCLUDE_ASM("asm/levels", func_L09_003AF9E8);

INCLUDE_ASM("asm/levels", func_L09_003B6550);

/* snapshots the object's frame and re-seeds its four steering angles from the new one */
INCLUDE_ASM("asm/levels", func_L09_003E99E0);

INCLUDE_ASM("asm/levels", func_L09_003E9AA0);

INCLUDE_ASM("asm/levels", func_L09_003E9E60);

/* steps the run's frame index and hands the sample it lands on to the linked stage */
void func_L09_003EA058(char *object, int index) {
    char *frame = *(char **)(object + 0x68);
    char buf[16];
    short value = *(short *)(frame + 0x118);
    int dir = value < *(short *)(frame + 0x11A) ? 1 : -1;
    float f;
    int off;
    unsigned int base;

    *(short *)(frame + 0x118) =
        func_L09_0033E8B0_1(object, index, buf, value, dir, *(float *)(frame + 0x78) * 10.0f);
    base = D_L09_001FE6C0[index];
    off = (*(short *)(frame + 0x11A) << 4) + 0x10;
    f = func_00282E48_2(object + 0x10, base + off);
    func_L09_003E9E60(object, buf, f);
}

INCLUDE_ASM("asm/levels", func_L09_003EA108);

int func_L09_003EA2D0(char *object, int index, int value)
{
    unsigned int entry = D_L09_001FE6C0[index];
    char *p = *(char **)(object + 0x68);
    int result;

    if (entry != 0) {
        result = func_L06_00382850_1(object + 0x10, entry, 0.0f);
        result = func_002B6228_1(object, entry, value, result);
        return *(short *)(p + 0x118);
    }
    *(short *)(p + 0x118) = 0;
    return -1;
}

INCLUDE_ASM("asm/levels", func_L09_003EA360);

INCLUDE_ASM("asm/levels", func_L09_003EA798);

void func_L09_003EA9D0(char *object)
{
    unsigned short count = *(unsigned short *)(object + 0x96);
    char *p = *(char **)(object + 104);

    if (count <= 0xFFFE)
        *(unsigned short *)(object + 0x96) = count + 1;
    func_00282978(p + 0x36A);
    func_002829A8(p + 0x368);
}

INCLUDE_ASM("asm/levels", func_L09_003EAB20);

void func_L09_003EAF60(char *object)
{
    func_002E5CF8(2, 0, object);
    func_002A6C58(object, 0, 3);
    func_0032C700(object + 0x10, 1, 0x19, 0x14, 0x14, 0x14, 0x2D, 1,
                  1.0f, 20.0f, 1, 1, 0, 0);
    func_0029F338(object);
}

INCLUDE_ASM("asm/levels", func_L09_003F65E0);

void func_L09_003F7158(char *object)
{
    if (*(unsigned char *)(object + 0x31) != 0) {
        if (func_00282E48(object + 0x10, D_L09_001B9A00) < 24.0f) {
            func_002A93B0(object);
            *(unsigned char *)(object + 0x6F) = 0x16;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L09_003F71C8);

INCLUDE_ASM("asm/levels", func_L09_003F72A8);

/* spawns one measured stage with the object's placement lifted by DY */
INCLUDE_ASM("asm/levels", func_L09_003F7370);

void func_L09_003F73E8(char *object)
{
    char *p = *(char **)(object + 104);

    if (*(unsigned char *)(object + 33) != 0xFF && *(unsigned char *)(p + 0x2EB) != 0) {
        func_00282948(*(void **)(p + 0x304));
    }
    *(unsigned char *)(p + 0x2EB) = 0;
}

void func_L09_003F7430(char *object)
{
    char *p = *(char **)(object + 104);
    char *q;

    if (*(unsigned char *)(object + 33) == 0xFF)
        return;
    if (*(unsigned char *)(p + 0x2EB) != 0)
        return;
    q = *(char **)(p + 0x304);
    (*(int *)q)++;
    *(unsigned char *)(p + 0x2EB) = 1;
}

void func_L09_003F7468(char *object)
{
    char *parent;

    char *p;

    p = *(char **)(object + 104);
    *(int *)(object + 152) = 0;
    parent = *(char **)(p + 0x2F8);
    if (parent == 0)
        return;
    if (*(short *)(parent + 170) != 3342)
        return;
    if (*(unsigned char *)(parent + 32) == 254)
        return;
    if (*(unsigned char *)(parent + 32) != 253)
        *(int *)(parent + 152) = 0;
}

/* shared native body of family cd6f3d9eca5f8edd */
/* attempt 1 - shape read straight off the disassembly:
   save object's word 152 from its owner's field 16, then repeat for a second
   object found through two pointer hops, skipping the sentinel byte values. */

void func_L09_003F74A8(char *p)
{
    char *q;
    char *r;
    char *s;
    int v;

    if (p == 0)
        return;
    if (*(unsigned char *)(p + 32) == 254)
        return;
    if (*(unsigned char *)(p + 32) == 253)
        return;

    q = *(char **)(p + 36);
    r = *(char **)(p + 104);
    *(int *)(p + 152) = *(int *)(q + 16);

    s = *(char **)(r + 760);
    if (s == 0)
        return;
    if (*(short *)(s + 170) != 3342)
        return;
    if (*(unsigned char *)(s + 32) == 254)
        return;
    if (*(unsigned char *)(s + 32) == 253)
        return;

    v = *(int *)(s + 36);
    *(int *)(s + 152) = *(int *)(v + 16);
}

INCLUDE_ASM("asm/levels", func_L09_003F7518);

void func_L09_003F7788(char *object, char *target)
{
    int out0, out1, out2;

    func_002A04B8(object, &out0, &out1, &out2);
    func_002A0488(target, out0, out1, out2);
}

void func_L09_003F77D0(char *object)
{
    char *descriptor = *(char **)(object + 0x68);
    char *slots;
    int i;

    if (*(int *)(descriptor + 0x2F8) != 0) {
        func_0029F338(*(void **)(descriptor + 0x2F8));
        *(int *)(descriptor + 0x2F8) = 0;
    }
    slots = descriptor + 0x2F0;
    for (i = 0; i < 2; ++i) {
        if (*(int *)(slots + i * 4) != 0) {
            func_0029F338(*(void **)(slots + i * 4));
            *(int *)(slots + i * 4) = 0;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L09_003F7840);

/* reports whether the index still names a reachable entry, or whether the frame moved close enough */
int func_L09_003F7A88(char *object, int index) {
    char *frame = *(char **)(object + 0x68);
    int entry = func_L09_00329028(object, index);

    if (entry == 0 && index != -1) {
        int off = (*(short *)(frame + 0x1CA) << 4) + 0x10;

        if (func_00282E80(object + 0x10, D_L09_001FE6C0[index] + off) <
            *(float *)(frame + 0x134))
            entry = 1;
    }
    return entry;
}

INCLUDE_ASM("asm/levels", func_L09_003F7B28);

INCLUDE_ASM("asm/levels", func_L09_003F7C50);

INCLUDE_ASM("asm/levels", func_L09_003F7CE0);

INCLUDE_ASM("asm/levels", func_L09_003F8598);

void func_L09_003F87D8(char *unused, int index)
{
    unsigned int entry;
    unsigned short *q;

    if (index == 0xFF)
        return;
    entry = D_L09_001FB560[index];
    if (entry == 0)
        return;
    q = (unsigned short *)entry;
    do {
        char *r = D_L09_001B285C + ((q[0] & 0x7FFF) << 8);

        if (*(signed char *)(r + 0x20) >= 0 && *(short *)(r + 0xAA) == 0xD0F)
            *(unsigned char *)(*(int *)(r + 0x68) + 0x2EA) = 1;
    } while ((short)*q++ >= 0);
}

void func_L09_003F8858(char *object)
{
    char *p = *(char **)(object + 104);

    ref_L09_003F8858_00(object, *(unsigned char *)(object + 33));
    ref_L09_003F8858_00(object, *(int *)(p + 0x308));
}

/* func_L09_003F8898 lives in the L09_002B7238-g8.c sibling: its four counters are small data. */

INCLUDE_ASM("asm/levels", func_L09_003F8960);

INCLUDE_ASM("asm/levels", func_L09_003F8AE0);

INCLUDE_ASM("asm/levels", func_L09_003F9040);

INCLUDE_ASM("asm/levels", func_L09_003F9108);
