#include "include_asm.h"

extern char F669d6320_D_001BAFC0[];

extern void F669d6320_FUN_00282F20(void *out, void *in, float scale);

extern void F669d6320_FUN_00282CC0(void *out, void *in1, void *in2);

extern int F669d6320_FUN_002761F8(void *a, void *b, int mode, void *self, int flag);

extern void F669d6320_FUN_00282D30(void *out, void *in, float scale);

extern void F2c74c194_FUN_00282D30(char *dst, char *src, float scale);

extern void F5a19df7a_FUN_002FC560(unsigned int a, unsigned long b);

extern void F5a19df7a_FUN_0031E2A0(char *p);

extern void F5a19df7a_FUN_0031E560(char *p);

extern char *F5a19df7a_FUN_002A7820(int a, int b);

extern float F5a19df7a_FUN_002A7940(void);

extern void F5a19df7a_FUN_0031EB00(char *a, int b, float e, char *c, float f, int d, int g);

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

/* The four 16-byte blocks the record carries and scales. */
#ifndef RAC2_T_BLOCK_0031A340
#define RAC2_T_BLOCK_0031A340
typedef struct {
    float q[4];
} Block_0031a340;
#endif

/* The 240-byte record func_0031A340 builds: a scaled matrix, a colour, the
   looked-up handle, the fixed fields and the two computed vectors. */
#ifndef RAC2_T_LOC_0031A340
#define RAC2_T_LOC_0031A340
typedef struct {
    Block_0031a340 b0;
    Block_0031a340 b1;
    Block_0031a340 b2;
    Block_0031a340 b3;
    int k0;
    int k1;
    int k2;
    int k3;
    int z80;
    int z84;
    int z88;
    float f92;
    float f96;
    int z100;
    float f104;
    float f108;
    long long v112;
    long long v120;
    long long v128;
    long long v136;
    float m00;
    float m01;
    float m02;
    float m03;
    float m10;
    float m11;
    float m12;
    float m13;
    float m20;
    float m21;
    float m22;
    float m23;
    float v192;
    float v196;
    float v200;
    float v204;
    float v208;
    float f212;
    float f216;
    float v220;
    float e0;
    float e1;
    float e2;
    float e3;
} Loc_0031a340;
#endif

/* The state block whose +0x140 holds the position the record is built around. */
#ifndef RAC2_T_STATE_001B5200
#define RAC2_T_STATE_001B5200
typedef struct {
    char pad_000[0x140];
    float pos[3];
} State_001b5200;
#endif

extern State_001b5200 D_001B5200;

INCLUDE_ASM("asm/game", func_003173B0);

INCLUDE_ASM("asm/game", func_00317D78);

extern void func_003182C0(void *);

void func_003181E0(void *object) {
    func_003182C0(object);
}

extern void func_002AB8C0(char *, char *);
extern int func_00282978(void *);
extern void func_002AEE70(char *, float, float, char *, float, char *);
extern void func_002AEF28(float, float, char *, char *, float, char *);

void func_00318200(char *object)
{
    char *data = *(char **)(object + 0x68);
    func_002AB8C0(object, data + 0x50);
    func_00282978(data + 0x270);
    if (func_00282978(data + 0x272)) {
        func_002AEE70(object, 0.1f, 0.08726647496223449707f, data + 0x27C, -1.0f, data + 0x280);
        func_002AEF28(0.13962635397911071777f, 0.026179943233728408813f, object, data + 0x284, 0.026179943233728408813f, data + 0x288);
    }
    if (*(unsigned short *)(object + 0x96) <= 65534)
        (*(unsigned short *)(object + 0x96))++;
}

INCLUDE_ASM("asm/game", func_00318340);

INCLUDE_ASM("asm/game", func_00319780);

INCLUDE_ASM("asm/game", func_00319B38);

extern void func_002AB8C0(char *, char *);
extern int func_00282978(void *);
extern void func_002B0990(char *, char *, int, int, float);

void func_00319EC0(char *object)
{
    char *data = *(char **)(object + 0x68);
    func_002AB8C0(object, data + 0x70);
    func_00282978(data + 0x280);
    func_00282978(data + 0x282);
    if (*(unsigned short *)(object + 0x96) < 65535)
        (*(unsigned short *)(object + 0x96))++;
    func_002B0990(object, data + 0x380, 5, 1, 2.0f);
}

extern void func_0031A158(char *);
extern void func_0031A340(char *, char *, float, int, int, int);

void func_00319FB8(char *object)
{
    char *data = *(char **)(object + 0x68);
    int mode = *(unsigned char *)(object + 0x20);
    if (mode == 12 || mode == 14 || mode == 16 || mode == 13 || *(unsigned char *)(object + 0x95) == 1)
        func_0031A158(object);
    func_0031A340(object, data + 0x310, 0.125f, 0x36F6, 0x20000000, 0x20000000);
    func_0031A340(object, data + 0x320, 0.15f, 0x1050B0, 0x30000000, 0x20000000);
    func_0031A340(object, data + 0x330, 0.15f, 0x1050B0, 0x10000000, 0x30000000);
}

INCLUDE_ASM("asm/game", func_0031A098);

INCLUDE_ASM("asm/game", func_0031A158);

extern void func_0031A098(void *record);
extern long long func_0027C878_1(int id);
extern float func_00283248(float x, float y);
extern float func_00282E80(void *a, void *b);
extern float func_002A7940(void);
extern void func_00283410(void *dst, void *src);
extern void func_00282CF0(void *dst, void *a, void *b);
extern void func_00282F20(void *dst, void *src, float scale);
extern void func_00282D30(void *dst, void *src, float scale);
extern void func_00280B90(void *dst, void *src, int flag);

/* Builds the 240-byte record around a position: the looked-up handle, the colour
   of the object, the position the object sits at and the scaled matrix it keeps.
   The first argument is not read. */
void func_0031A340(char *unused, char *p1, float scale, int col0, int col1, int col2)
{
    Loc_0031a340 local;
    float x;

    func_0031A098(&local);
    local.v120 = func_0027C878_1(11);
    local.k3 = col0 + col1;
    local.k2 = col0 + col1;
    local.k1 = col0 + col1;
    local.k0 = col0 + col1;
    local.v136 = 0x0000008000000048LL;
    local.v220 = 0.0f;
    local.f216 = func_00283248(D_001B5200.pos[0] - *(float *)p1,
                                D_001B5200.pos[1] - *(float *)(p1 + 4));
    x = func_00283248(func_00282E80(p1, &D_001B5200.pos),
                      D_001B5200.pos[2] - *(float *)(p1 + 8));
    local.f212 = -x;
    local.v208 = func_002A7940();
    func_00283410(&local.m00, &local.v208);
    func_00282CF0(&local.e0, &D_001B5200.pos, p1);
    func_00282F20(&local.e0, &local.e0, 0.5f);
    local.v192 = *(float *)p1 + local.e0;
    local.v196 = *(float *)(p1 + 4) + local.e1;
    local.v200 = *(float *)(p1 + 8) + local.e2;
    local.v204 = 1.0f;
    func_00282D30(&local.b0, &local.b0, scale);
    func_00282D30(&local.b1, &local.b1, scale);
    func_00282D30(&local.b2, &local.b2, scale);
    func_00282D30(&local.b3, &local.b3, scale);
    func_00280B90(&local, &local.m00, 0);
    local.k3 = col0 + col2;
    local.k2 = col0 + col2;
    local.k1 = col0 + col2;
    local.k0 = col0 + col2;
    func_00282D30(&local.b0, &local.b0, 0.5f);
    func_00282D30(&local.b1, &local.b1, 0.5f);
    func_00282D30(&local.b2, &local.b2, 0.5f);
    func_00282D30(&local.b3, &local.b3, 0.5f);
    func_00280B90(&local, &local.m00, 0);
}

int func_0031A708(unsigned char *object) {
    return object[0x21] != 255;
}

INCLUDE_ASM("asm/game", func_0031A718);

INCLUDE_ASM("asm/game", func_0031A848);

void func_0031A998(char *self, char *p1, int p2)
{
    float a[4];
    float b[4];

    F669d6320_FUN_00282F20(a, self + 224, 0.1f);
    F669d6320_FUN_00282F20(b, self + 224, -3.0f);
    F669d6320_FUN_00282CC0(a, a, self + 16);
    F669d6320_FUN_00282CC0(b, b, self + 16);

    if (F669d6320_FUN_002761F8(a, b, 2, self, 0))
        F669d6320_FUN_00282F20(p1, F669d6320_D_001BAFC0, 1.0f);
    else
        F669d6320_FUN_00282F20(p1, self + 224, -1.0f);

    if (p2 == 0)
        F669d6320_FUN_00282D30(p1, p1, -1.0f);
}

extern void func_00282CF0(void *, void *, void *);
extern unsigned int func_0031AB08(unsigned int);
extern void func_0031AB18(void *, unsigned int, void *, int);

/* func_0031AAB0 is matched in 0031AAB0.c. */


unsigned int func_0031AB08(unsigned int param_1)
{
    unsigned int inner_ptr;
    unsigned int result;

    inner_ptr = *(unsigned int *)(param_1 + 0x68);
    result = *(unsigned int *)(inner_ptr + 0x18);
    return result;
}

INCLUDE_ASM("asm/game", func_0031AB18);

INCLUDE_ASM("asm/game", func_0031ABE0);

INCLUDE_ASM("asm/game", func_0031BCC8);

INCLUDE_ASM("asm/game", func_0031C498);

extern void func_0031C970(char *);
extern void func_0031CB78(char *, char *, int, int, int, float);
extern void func_0031CDF8(char *, char *, float, int, int);

/* Refresh the auxiliary state for modes 4, 5, 10 or flag 2, then issue
   three calls with the embedded vectors and three with the referenced vectors. */
void func_0031C7A0(char *self)
{
    char *data = *(char **)(self + 0x68);
    unsigned char mode = self[0x20];

    if (mode == 4 || mode == 5 || (unsigned char)self[0x95] == 2 || mode == 10)
        func_0031C970(self);

    func_0031CB78(self, data + 0x370, 0x41DE, 0x10000000, 0x20000000, 0.125f);
    func_0031CB78(self, data + 0x380, 0x00800000, 0x10000000, 0x30000000, 0.125f);
    func_0031CB78(self, data + 0x390, 0x00800000, 0x10000000, 0x30000000, 0.125f);

    func_0031CDF8(self, *(char **)(data + 0x270) + 0x10, 0.075f, 0x204080, 0x204080);
    func_0031CDF8(self, *(char **)(data + 0x274) + 0x10, 0.075f, 0x204080, 0x204080);
    func_0031CDF8(self, *(char **)(data + 0x278) + 0x10, 0.075f, 0x204080, 0x204080);
}

void func_0031C8E0(char *p)
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

INCLUDE_ASM("asm/game", func_0031C970);

INCLUDE_ASM("asm/game", func_0031CB78);

INCLUDE_ASM("asm/game", func_0031CDF8);

INCLUDE_ASM("asm/game", func_0031D070);

INCLUDE_ASM("asm/game", func_0031D268);

/* func_0031D308 is in the -G8 sibling. */

INCLUDE_ASM("asm/game", func_0031D350);

INCLUDE_ASM("asm/game", func_0031D3E0);

INCLUDE_ASM("asm/game", func_0031D588);

INCLUDE_ASM("asm/game", func_0031DC10);

INCLUDE_ASM("asm/game", func_0031DC18);

INCLUDE_ASM("asm/game", func_0031DEE8);

INCLUDE_ASM("asm/game", func_0031E3A8);

INCLUDE_ASM("asm/game", func_0031E528);

INCLUDE_ASM("asm/game", func_0031E560);

INCLUDE_ASM("asm/game", func_0031E6D8);



void func_0031E750(char *p)
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

INCLUDE_ASM("asm/game", func_0031E7D8);

void func_0031E8F8(char *p, float scale)
{
    F2c74c194_FUN_00282D30(p, p, scale);
    F2c74c194_FUN_00282D30(p + 16, p + 16, scale);
    F2c74c194_FUN_00282D30(p + 32, p + 32, scale);
    F2c74c194_FUN_00282D30(p + 48, p + 48, scale);
}

void func_0031E960(char *p)
{
    char *q;
    char *o;
    char *r;
    float v;
    float w;

    q = p + 16;
    o = *(char **)(p + 104);
    F5a19df7a_FUN_002FC560(0x47, 0x53001UL);
    F5a19df7a_FUN_0031E2A0(p);
    F5a19df7a_FUN_0031E560(p);

    v = *(float *)(o + 24);
    v = v * 0.5f;
    r = F5a19df7a_FUN_002A7820(80, 96);
    w = F5a19df7a_FUN_002A7940();
    F5a19df7a_FUN_0031EB00(q, 0x103080, v, r, w, 49, 2);

    v = *(float *)(o + 24);
    v = v * 3.00000011920928955078125e-01f;
    r = F5a19df7a_FUN_002A7820(80, 96);
    w = F5a19df7a_FUN_002A7940();
    F5a19df7a_FUN_0031EB00(q, 0xFFFFFF, v, r, w, 49, 2);

    v = *(float *)(o + 24);
    v = v * 6.99999988079071044921875e-01f;
    r = F5a19df7a_FUN_002A7820(8, 22);
    w = F5a19df7a_FUN_002A7940();
    F5a19df7a_FUN_0031EB00(q, 0x803010, v, r, w, 11, 2);

    v = *(float *)(o + 24);
    v = v * 4.49999988079071044921875e-01f;
    r = F5a19df7a_FUN_002A7820(32, 64);
    w = F5a19df7a_FUN_002A7940();
    F5a19df7a_FUN_0031EB00(q, 0x803010, v, r, w, 11, 2);

    F5a19df7a_FUN_002FC560(0x47, 0x5360bUL);
}

extern float D_FLT_001B5200[];
extern int func_0027C878(int);
extern float func_00283248(float, float);
extern float func_00282E80(void *, void *);
extern void func_00283410(void *, void *);
extern void func_00280B90(void *, void *, int);

void func_0031EB00(char *position, int color, float scale, char *resource, float alpha, int mode, int flag)
{
    unsigned char work[0xE0];
    unsigned int rgba;
    unsigned long long mode_bits;
    float first_angle;
    float second_angle;
    float distance;

    func_0031E750((char *)work);
    *(long long *)(work + 0x78) = func_0027C878(mode);

    mode_bits = ((unsigned long long)flag << 2) | 0x0000008000000040ULL;
    *(long long *)(work + 0x88) = mode_bits;
    *(long long *)(work + 0x80) = 0x0000FF9000000260ULL;

    rgba = (unsigned int)color + ((unsigned int)resource << 24);
    *(int *)(work + 0x4C) = rgba;
    *(int *)(work + 0x48) = rgba;
    *(int *)(work + 0x44) = rgba;
    *(int *)(work + 0x40) = rgba;
    *(long long *)(work + 0x70) = 0;

    *(int *)(work + 0xDC) = 0;
    first_angle = func_00283248(D_FLT_001B5200[0x140 / 4] - *(float *)(position + 0),
                                D_FLT_001B5200[0x144 / 4] - *(float *)(position + 4));
    *(float *)(work + 0xD8) = first_angle;

    distance = func_00282E80(position, &D_FLT_001B5200[0x140 / 4]);
    second_angle = func_00283248(distance, D_FLT_001B5200[0x148 / 4] - *(float *)(position + 8));
    *(float *)(work + 0xD0) = alpha;
    *(float *)(work + 0xD4) = -second_angle;

    func_00283410(work + 0x90, work + 0xD0);
    *(float *)(work + 0xC0) = *(float *)(position + 0);
    *(float *)(work + 0xC4) = *(float *)(position + 4);
    *(float *)(work + 0xC8) = *(float *)(position + 8);
    *(float *)(work + 0xCC) = 1.0f;
    func_0031E8F8((char *)work, scale);
    func_00280B90(work, work + 0x90, 0);
}

INCLUDE_ASM("asm/game", func_0031EC68);

INCLUDE_ASM("asm/game", func_0031EFC8);

void func_0031F1A8(Vec16_Fc936841d *object, int index)
{
    char local[16];

    Fc936841d_FUN_00282CF0(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    Fc936841d_FUN_00282CC0((char *)&object[index - 1], (char *)&object[index - 2], local);
}
