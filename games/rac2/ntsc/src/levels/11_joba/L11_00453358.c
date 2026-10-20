#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_00282D30_2(char *dst, char *src, float scale);
extern void func_002A7750(void *a0, int a1, int a2);
extern void func_002AB8C0(void *a0, void *a1);
extern void func_002A0488(void *a0, int a1, int a2, int a3);
extern void func_002A8C28(void *a0, void *a1, int a2, float f12, float f13, float f14,
                          int a3, int t0, int t1);
extern void func_L11_00459F30();
extern void func_L11_0045A3C8();
extern void func_L11_0045A578();
extern float func_00283248(float a, float b);
extern float func_002A7878(float a, float b);
extern float func_00283180(float x);
extern float func_00283198(float x);

extern void func_002A7470(void *a0, int a1, int a2, int a3);

extern void func_002A05A8(void *a0);

extern void func_L11_00441978(char *p, int id);

extern int func_L11_00441868(char *p, int id);

extern int func_002B3BA8(void *a0, int a1, int a2, int a3, float f12, float f13);

extern void func_002B60B0(void *a0, float f12, float f13, float f14, float f15);

extern float func_002A7820(int a0, int a1);

extern float func_002A7940(void);

extern void func_00281EB8(void *a0, void *a1, void *a2, float f12, float f13,
                          float f14, int a3);

extern void func_002B3380(void *a0, void *a1, void *a2, int a3);

extern void func_002A0F48(void *a0);

extern void func_L00_002E6CF0(void *a0, void *a1);

extern void func_L11_0045B6C8(char *p);

extern int D_L11_001CAE40;

extern int D_L11_001B46D4;

extern int D_L11_001B30C8;

extern void func_L11_00458DF0(char *a0, void *a1, void *a2, int a3, int t0, float f12,
                              float f13);

/* The vector the aim helpers read, at 0x001BA780 in joba. */
typedef struct {
    char pad_000[0x140];
    float x_140;
    float y_144;
    float z_148;
} L11AimVector;

extern L11AimVector D_L11_001BA780;

extern void func_L11_0045FFB0(char *pkt);

extern long long func_0027C878(char *p);

extern float func_00282E80(char *p, float *v);

extern void func_00283410(float *matrix, float *quat);

extern void func_00282CF0(float *out, float *a, float *b);

extern void func_00282F20(float *dst, float *src, float scale);

extern void func_00280B90(char *pkt, float *matrix, int mode);
extern void func_002E59F8(int index);

extern void func_002E5DA0(int index, int value, char *object, int kind);

extern void func_0032C700(char *, int, int, int, int, int, int, int, float, float, int,
                         int, int, int);

extern void func_00282C88(void *);

extern void func_L11_003973F8(char *, int, float, float);

extern char D_00188660[];

INCLUDE_ASM("asm/levels", func_L11_00453358);

INCLUDE_ASM("asm/levels", func_L11_00454508);

void func_L11_00454AE0(char *p)
{
    char *q = *(char **)(p + 0x68);
    unsigned short c;

    func_002AB8C0(p, q + 0x50);
    if (*(short *)(q + 0x50) == 0) {
        if (*(short *)(q + 0x1FE) == 0) {
            func_002A0488(p, *(int *)(q + 0x1F0), *(int *)(q + 0x1F4), *(int *)(q + 0x1F8));
            *(short *)(q + 0x1FE) = 1;
        }
    } else {
        *(short *)(q + 0x1FE) = 0;
    }
    c = *(unsigned short *)(p + 0x96);
    if (c <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = c + 1;
}

INCLUDE_ASM("asm/levels", func_L11_00454B60);


/* Aims the object's stage, re-arms its effect and stores the new heading. */
void func_L11_00454C78(char *p, int a1, int a2)
{
    char *q = *(char **)(p + 0x68);
    char *definition;
    unsigned short flags;
    int cache;
    float f;

    f = (float)*(short *)(q + 0x24);
    *(short *)(q + 0x1FC) = a2;
    *(short *)(q + 0x1EC) = 0;
    *(unsigned char *)(q + 0x2E) = 1;
    *(float *)(q + 0x20) = f;
    if (*(unsigned char *)(p + 0x43) != 0)
        func_002A7470(p, 0, 0, 10);
    flags = *(unsigned short *)(p + 0x34);
    *(unsigned char *)(p + 0x7F) = 0;
    *(unsigned short *)(p + 0x34) = (flags | 0x1000) & 0xFFB9;
    definition = *(char **)(p + 0x24);
    cache = *(int *)(definition + 0x10);
    *(int *)(p + 0x98) = cache;
    if (*(short *)(q + 0x1FC) != 0) {
        func_002A7750(p, 7, 3);
        *(unsigned short *)(p + 0x34) |= 0x100;
        func_L11_003973F8(p, 0, 2.0f, 2.0f);
    } else {
        func_002A7750(p, 3, 1);
        *(unsigned char *)(p + 0x31) = 1;
        *(unsigned short *)(p + 0x34) &= 0xFEFE;
    }
    func_00282C88(q + 0x1B0);
    f = func_002A7878(20.0f, 30.0f) * 0.017453294f;
    *(short *)(q + 0x1EE) = a1;
    *(float *)(q + 0x1CC) = f;
    if (*(short *)(q + 0x50) != 0) {
        *(short *)(q + 0x50) = 0;
        func_002A0488(p, *(int *)(q + 0x1F0), *(int *)(q + 0x1F4), *(int *)(q + 0x1F8));
        *(short *)(q + 0x52) = 1;
    }
}

/* Releases the object's slot in the shared table and drops it to the idle flag block. */
void func_L11_00454DD8(char *p)
{
    char *q = *(char **)(p + 0x68);
    int i = *(int *)(q + 0x1DC);
    char *entry;

    if (i != -1) {
        entry = D_00188660 + i * 0x70;
        if (*(char **)(entry + 0x88) == p && *(unsigned char *)(entry + 0x74) != 0)
            func_002E59F8(i);
    }
    *(int *)(q + 0x1DC) = -1;
    if (*(short *)(q + 0x1EC) == 1) {
        *(short *)(q + 0x1EC) = 0;
        func_L11_00441978(*(char **)(p + 0xB8), *(short *)(p + 0xAA));
    }
    if (*(unsigned char *)(p + 0x31) != 0) {
        func_0032C700(p + 0x10, 0, 10, 30, 0, 10, 45, 1, 0.3f, 0.0f, 0, 0, 0, 0);
        func_002E5DA0(0, 0, p, 0xDF7);
    }
    func_002A7470(p, 0, 0, 3);
    func_002A05A8(p);
    func_002A7750(p, 1, 0);
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(int *)(p + 0x98) = 0;
    *(unsigned char *)(p + 0x31) = 0;
    *(short *)(q + 0x1EC) = 0;
    *(int *)(q + 0x1C4) = 0;
}

INCLUDE_ASM("asm/levels", func_L11_00454F18);

INCLUDE_ASM("asm/levels", func_L11_00455DF8);

void func_L11_00456010(char *p)
{
    unsigned short c = *(unsigned short *)(p + 0x96);

    if (c < 0xFFFF)
        *(unsigned short *)(p + 0x96) = c + 1;
    func_002A8C28(p, p + 0x10, 0x4000000, 1.6f, 1.0f, 1.0f, 0, 1, 0);
}

INCLUDE_ASM("asm/levels", func_L11_00456070);

INCLUDE_ASM("asm/levels", func_L11_00456378);

INCLUDE_ASM("asm/levels", func_L11_00456438);

/* Retires the state's counter and drops the object to its idle flag block. */
void func_L11_00456540(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(short *)(q + 0x256) == 1) {
        func_L11_00441978(*(char **)(p + 0xB8), 0xDF9);
    }
    *(short *)(q + 0x256) = 0;
    *(int *)(q + 0x240) = 0;
    func_002A7470(p, 0, 0, 3);
    func_002A05A8(p);
    func_002A7750(p, 1, 0);
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(char *)(p + 0x31) = 0;
}

INCLUDE_ASM("asm/levels", func_L11_004565D8);

INCLUDE_ASM("asm/levels", func_L11_004566C0);

INCLUDE_ASM("asm/levels", func_L11_00456938);

INCLUDE_ASM("asm/levels", func_L11_00456A48);

INCLUDE_ASM("asm/levels", func_L11_00456C20);

INCLUDE_ASM("asm/levels", func_L11_00456EA0);

extern void func_L04_0040A090(int, void *, void *, void *, void *, void *);
extern char D_L11_001B1E50[] __attribute__((sda));
extern char D_L11_001B1E70[] __attribute__((sda));
extern char D_L11_001B1E80[] __attribute__((sda));
extern char D_L11_001B1EA0[] __attribute__((sda));
extern char D_L11_001B1EB0[] __attribute__((sda));
extern char D_L11_001B1EE0[] __attribute__((sda));
extern char D_L11_001B1F20[] __attribute__((sda));
extern char D_L11_001B1F30[] __attribute__((sda));
extern char D_L11_001B1F50[] __attribute__((sda));
extern char D_L11_001B1F70[] __attribute__((sda));
extern char D_L11_001B43F0[] __attribute__((sda));
extern char D_L11_001B4430[] __attribute__((sda));
extern char D_L11_001B4450[] __attribute__((sda));
extern char D_L11_001B4490[] __attribute__((sda));
extern char D_L11_001B44B0[] __attribute__((sda));
extern char D_L11_001B44D0[] __attribute__((sda));
extern char D_L11_001B4510[] __attribute__((sda));
extern char D_L11_001B4550[] __attribute__((sda));
extern char D_L11_001B4570[] __attribute__((sda));
extern char D_L11_001B4580[] __attribute__((sda));
extern char D_L11_001B4588[] __attribute__((sda));
extern char D_L11_001B4598[] __attribute__((sda));
extern char D_L11_001B45A0[] __attribute__((sda));
extern char D_L11_001B45B8[] __attribute__((sda));
extern char D_L11_001B45D8[] __attribute__((sda));
extern char D_L11_001B45E0[] __attribute__((sda));
extern char D_L11_001B45F0[] __attribute__((sda));
extern char D_L11_001B4600[] __attribute__((sda));
extern char D_L11_001B4608[] __attribute__((sda));
extern char D_L11_001B4618[] __attribute__((sda));
extern char D_L11_001B4620[] __attribute__((sda));
extern char D_L11_001B4630[] __attribute__((sda));
extern char D_L11_001B4638[] __attribute__((sda));
extern char D_L11_001B4650[] __attribute__((sda));
extern char D_L11_001B4670[] __attribute__((sda));
extern char D_L11_001B4678[] __attribute__((sda));
extern char D_L11_001B4688[] __attribute__((sda));
extern char D_L11_001B4698[] __attribute__((sda));
extern char D_L11_001B46A0[] __attribute__((sda));
extern char D_L11_001B46A4[] __attribute__((sda));
extern char D_L11_001B46A8[] __attribute__((sda));
extern char D_L11_001B46AC[] __attribute__((sda));
extern char D_L11_001B46B0[] __attribute__((sda));
extern char D_L11_001B46B4[] __attribute__((sda));
extern char D_L11_001B46B8[] __attribute__((sda));
extern char D_L11_001B46BC[] __attribute__((sda));
extern char D_L11_001B46C0[] __attribute__((sda));
extern char D_L11_001B46C4[] __attribute__((sda));
extern char D_L11_002626B0[];
extern char D_L11_00262710[];

void func_L11_00457600(void) {
    func_L04_0040A090(4, D_L11_001B1E50, D_L11_001B43F0, D_L11_001B4570, D_L11_001B4608, D_L11_001B46A0);
    func_L04_0040A090(2, D_L11_001B1E70, D_L11_001B4430, D_L11_001B4580, D_L11_001B4618, D_L11_001B46A4);
    func_L04_0040A090(4, D_L11_001B1E80, D_L11_001B4450, D_L11_001B4588, D_L11_001B4620, D_L11_001B46A8);
    {
        char *data = D_L11_001B4490;
        func_L04_0040A090(2, D_L11_001B1EA0, data, D_L11_001B4598, D_L11_001B4630, D_L11_001B46AC);
    }
    func_L04_0040A090(6, D_L11_001B1EB0, D_L11_002626B0, D_L11_001B45A0, D_L11_001B4638, D_L11_001B46B0);
    func_L04_0040A090(8, D_L11_001B1EE0, D_L11_00262710, D_L11_001B45B8, D_L11_001B4650, D_L11_001B46B4);
    func_L04_0040A090(2, D_L11_001B1F20, D_L11_001B44B0, D_L11_001B45D8, D_L11_001B4670, D_L11_001B46B8);
    func_L04_0040A090(4, D_L11_001B1F30, D_L11_001B44D0, D_L11_001B45E0, D_L11_001B4678, D_L11_001B46BC);
    func_L04_0040A090(4, D_L11_001B1F50, D_L11_001B4510, D_L11_001B45F0, D_L11_001B4688, D_L11_001B46C0);
    func_L04_0040A090(2, D_L11_001B1F70, D_L11_001B4550, D_L11_001B4600, D_L11_001B4698, D_L11_001B46C4);
}


INCLUDE_ASM("asm/levels", func_L11_004577F8);

INCLUDE_ASM("asm/levels", func_L11_004580E8);

INCLUDE_ASM("asm/levels", func_L11_00458A98);

INCLUDE_ASM("asm/levels", func_L11_00458C38);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00458D88(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

void func_L11_00458F58(char *p)
{
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(p, 1, -1);
}

INCLUDE_ASM("asm/levels", func_L11_00458F90);

INCLUDE_ASM("asm/levels", func_L11_004590F0);

INCLUDE_ASM("asm/levels", func_L11_00459640);

/* func_L11_00459D48 lives in the sibling small-data unit (L11_00453358-g8.c). */

void func_L11_00459E48(char *p)
{
    int i;

    func_L11_00459F30(p);
    func_L11_0045A3C8(p);
    for (i = 4; i >= 0; i--)
        func_L11_0045A578(p);
}

/* shared native body of family a04928c6 */
void func_L11_00459EA0(char *p)
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

INCLUDE_ASM("asm/levels", func_L11_00459F30);

INCLUDE_ASM("asm/levels", func_L11_0045A3C8);

INCLUDE_ASM("asm/levels", func_L11_0045A578);

INCLUDE_ASM("asm/levels", func_L11_0045A8C0);

INCLUDE_ASM("asm/levels", func_L11_0045B5C8);

INCLUDE_ASM("asm/levels", func_L11_0045B6C8);

INCLUDE_ASM("asm/levels", func_L11_0045FA68);

INCLUDE_ASM("asm/levels", func_L11_0045FDB0);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00460038(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

/* Builds a 144-byte packet, aims a quaternion at the level's vector from pos through three
   float helpers, derives an offset basis by a fourth, writes the last matrix row from pos
   plus that offset, then dispatches the packet. */
void func_L11_004600A0(char *pos, int a1, int a2, char *a3, int t0,
                       float fA, float fB, float fC)
{
    char pkt[144];
    float m[16];
    float quat[4];
    float off[4];
    long long r;

    func_L11_0045FFB0(pkt);
    r = func_0027C878(a3);

    *(long long *)(pkt + 136) = ((long long)t0 << 2) | 0x8000000040LL;
    *(long long *)(pkt + 128) = 280993940374112LL;
    *(int *)(pkt + 76) = a1 + (a2 << 24);
    *(int *)(pkt + 72) = a1 + (a2 << 24);
    *(int *)(pkt + 68) = a1 + (a2 << 24);
    *(int *)(pkt + 64) = a1 + (a2 << 24);
    *(long long *)(pkt + 112) = 0LL;
    *(long long *)(pkt + 120) = r;

    quat[3] = 0.0f;
    quat[2] = func_00283248(D_L11_001BA780.x_140 - *(float *)(pos + 0),
                           D_L11_001BA780.y_144 - *(float *)(pos + 4));
    quat[1] = -func_00283248(func_00282E80(pos, &D_L11_001BA780.x_140),
                             D_L11_001BA780.z_148 - *(float *)(pos + 8));
    quat[0] = fB;

    func_00283410(m, quat);
    func_00282CF0(off, &D_L11_001BA780.x_140, pos);
    func_00282F20(off, off, fC);

    m[12] = *(float *)(pos + 0) + off[0];
    m[13] = *(float *)(pos + 4) + off[1];
    m[14] = *(float *)(pos + 8) + off[2];
    m[15] = 1.0f;

    func_L11_00460038(pkt, fA);
    func_00280B90(pkt, m, 0);
}

INCLUDE_ASM("asm/levels", func_L11_00461308);
