#include "include_asm.h"

extern int func_00282978_5(char *handle);

extern void func_0029F338_2(char *object);

#ifndef RAC2_T_NODE16_F23626AD8
#define RAC2_T_NODE16_F23626AD8
typedef struct Node16_F23626ad8 Node16_F23626ad8;
#endif

#ifndef RAC2_T_NODE104_F23626AD8
#define RAC2_T_NODE104_F23626AD8
typedef struct Node104_F23626ad8 Node104_F23626ad8;
#endif

#ifndef RAC2_T_NODE24_F23626AD8
#define RAC2_T_NODE24_F23626AD8
typedef struct Node24_F23626ad8 Node24_F23626ad8;
#endif

struct Node16_F23626ad8 {
    short h0;
    unsigned char pad2[2];
    unsigned char b4;
    unsigned char b5;
    unsigned char b6;
};

struct Node104_F23626ad8 {
    unsigned char pad0[16];
    Node16_F23626ad8 *p16;
};

struct Node24_F23626ad8 {
    unsigned char pad0[52];
    unsigned short h52;
    unsigned char pad54[50];
    Node104_F23626ad8 *p104;
};

#ifndef RAC2_T_OBJ_F23626AD8
#define RAC2_T_OBJ_F23626AD8
typedef struct Obj_F23626ad8 {
    unsigned char pad0[4];
    union {
        int f4;
        struct {
            unsigned char b4;
            unsigned char b5;
            short h6;
        } b;
    } u;
    unsigned char pad8[16];
    Node24_F23626ad8 *f24;
} Obj_F23626ad8;
#endif

extern void func_002A0488(Node24_F23626ad8 *a, unsigned char b, unsigned char c, unsigned char d);

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

INCLUDE_ASM("asm/levels", func_L00_004096B0);

INCLUDE_ASM("asm/levels", func_L00_00409E48);

INCLUDE_ASM("asm/levels", func_L00_0040A4A8);

INCLUDE_ASM("asm/levels", func_L00_0040AFB8);

INCLUDE_ASM("asm/levels", func_L00_0040B098);

INCLUDE_ASM("asm/levels", func_L00_0040B2C0);

INCLUDE_ASM("asm/levels", func_L00_0040B330);

INCLUDE_ASM("asm/levels", func_L00_0040B398);

INCLUDE_ASM("asm/levels", func_L00_0040B838);

INCLUDE_ASM("asm/levels", func_L00_0040BA30);

INCLUDE_ASM("asm/levels", func_L00_0040C7D8);

INCLUDE_ASM("asm/levels", func_L00_0040CCA0);

INCLUDE_ASM("asm/levels", func_L00_0040D058);

INCLUDE_ASM("asm/levels", func_L00_0040DC08);

/* Tests the handle cached in the object and, while the handle reports itself already released, drops the next sub-handle, otherwise runs the object's destroy helper. */
void func_L00_0040DF00(char *object)
{
    char *handle = *(char **)(object + 104);

    if (func_00282978_5(handle) == 0)
        func_00282978_5(handle + 4);
    else
        func_0029F338_2(object);
}

INCLUDE_ASM("asm/levels", func_L00_0040DF58);

INCLUDE_ASM("asm/levels", func_L00_0040F6A0);

INCLUDE_ASM("asm/levels", func_L00_00410178);

INCLUDE_ASM("asm/levels", func_L00_00410430);

INCLUDE_ASM("asm/levels", func_L00_00410508);

INCLUDE_ASM("asm/levels", func_L00_00410690);

INCLUDE_ASM("asm/levels", func_L00_00410840);

INCLUDE_ASM("asm/levels", func_L00_00410888);

void func_L00_00410D18(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00410D20);

/* Calls a helper with a referenced node and three identity bytes of the object, then copies those bytes into a linked record when a node flag bit is set and the record exists with a non-zero leading short. */
void func_L00_004110D8(Obj_F23626ad8 *p)
{
    Node24_F23626ad8 *s;
    Node16_F23626ad8 *d;
    int v;

    func_002A0488(p->f24, p->u.b.b4, p->u.b.b5, p->u.b.h6);
    s = p->f24;
    if ((s->h52 & 0x20) == 0)
        return;
    d = ((Node104_F23626ad8 *)s->p104)->p16;
    if (d == 0)
        return;
    if (d->h0 == 0)
        return;
    d->b4 = p->u.b.b4;
    d->b5 = p->u.f4 >> 8;
    v = p->u.b.h6;
    d->b6 = v;
}

INCLUDE_ASM("asm/levels", func_L00_004115A8);

INCLUDE_ASM("asm/levels", func_L00_004115E8);

INCLUDE_ASM("asm/levels", func_L00_004117B8);

INCLUDE_ASM("asm/levels", func_L00_004118A8);

INCLUDE_ASM("asm/levels", func_L00_00411B30);

INCLUDE_ASM("asm/levels", func_L00_00411F48);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_00412088(char *p)
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

INCLUDE_ASM("asm/levels", func_L00_00412110);

INCLUDE_ASM("asm/levels", func_L00_00412230);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_00412408(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L00_00412470);
