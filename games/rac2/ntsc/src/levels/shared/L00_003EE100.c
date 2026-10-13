#include "include_asm.h"
#include "common.h"

/* The 0x50-stride record table both walkers below scan; the retail reaches it through $gp in
   ordinary flow, which the assembler only selects from a bare symbol operand. */
extern char *D_L00_001B2BB4_1 __attribute__((sda));

extern void func_002833F0_1(char *a, char *b);

extern float func_00283198_1(float value);

extern void func_00282D30_3(char *a, char *b, float value);

extern float func_00283B98_3(float value, float scale);

extern int func_002ABB78(char *object);

extern float *func_002AB1F0(char *object);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner {
    unsigned char prefix_00[0x68];
    void *context_68;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner;

typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext {
    unsigned char prefix_00[0x10];
    float vector_10[4];
    unsigned int field_20;
    unsigned int active_24;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext;

extern void func_002FDBD8_5(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern void func_00282D30_2(char *dst, char *src, float scale);

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

extern void func_00282F20_1(char *, char *, float);

extern void func_00282DD8(char *, char *, char *);

extern int func_002B4038(int, int);

extern void func_0029F338_1(char *);

extern void func_L00_003F5B10(char *);

extern void func_L00_003F6308(char *);

extern void func_L00_003F6B10(char *);

extern char *func_L00_0032A088(int a0, int a1);

extern char *func_L00_0032A0B8(int a0, int a1);

extern int func_00282978(char *a0);

extern float func_00283B98(float a0, float a1);

#ifndef RAC2_T_NODE16_FE48725C6
#define RAC2_T_NODE16_FE48725C6
typedef struct Node16_Fe48725c6 Node16_Fe48725c6;
#endif

#ifndef RAC2_T_NODE104_FE48725C6
#define RAC2_T_NODE104_FE48725C6
typedef struct Node104_Fe48725c6 Node104_Fe48725c6;
#endif

#ifndef RAC2_T_NODE24_FE48725C6
#define RAC2_T_NODE24_FE48725C6
typedef struct Node24_Fe48725c6 Node24_Fe48725c6;
#endif

struct Node16_Fe48725c6 {
    short h0;
    unsigned char pad2[2];
    unsigned char b4;
    unsigned char b5;
    unsigned char b6;
};

struct Node104_Fe48725c6 {
    unsigned char pad0[16];
    Node16_Fe48725c6 *p16;
};

struct Node24_Fe48725c6 {
    unsigned char pad0[52];
    unsigned short h52;
    unsigned char pad54[50];
    Node104_Fe48725c6 *p104;
};

#ifndef RAC2_T_OBJ_FE48725C6
#define RAC2_T_OBJ_FE48725C6
typedef struct Obj_Fe48725c6 {
    unsigned char pad0[4];
    union {
        int f4;
        struct {
            unsigned char b4;
            unsigned char b5;
            short h6;
        } b;
    } u;
    unsigned char pad8[12];
    Node24_Fe48725c6 *f24;
} Obj_Fe48725c6;
#endif

extern void func_002A0488_1(Node24_Fe48725c6 *a, unsigned char b, unsigned char c, unsigned char d);

/* Family 40487154aa250cc8: fixed-field addend writes into one resource record. */
void func_L00_003EE100(char *object)
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
int func_L00_003EE228(char *object)
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

INCLUDE_ASM("asm/levels", func_L00_003EE4C0);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_003EE8E8(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_5(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003EE920);

INCLUDE_ASM("asm/levels", func_L00_003EED10);

INCLUDE_ASM("asm/levels", func_L00_003EEE20);

INCLUDE_ASM("asm/levels", func_L00_003EEE90);

INCLUDE_ASM("asm/levels", func_L00_003EF3E0);

INCLUDE_ASM("asm/levels", func_L00_003EF548);

extern int D_L00_001B0838 __attribute__((sda));

extern int D_L00_001B2BB4 __attribute__((sda));

void func_L00_003EF550(void)
{
    D_L00_001B2BB4 = 0;
    D_L00_001B0838 = 0;
}

INCLUDE_ASM("asm/levels", func_L00_003EF560);

extern int func_00283DB8(char *local, float value);

void func_L00_003EFA00(char *obj)
{
    char local[16];
    int r;

    r = func_002B4038(*(short *)(obj + 0xAA), *(int *)(*(int *)(obj + 0x68) + 0x28));
    if (r == 0) {
        func_L00_003EF550();
        func_0029F338(obj);
        return;
    }
    qcopy(local, obj + 0x10);
    *(float *)(local + 0xC) = 2.0f;
    r = func_00283DB8(local, 100.0f);
    if (r == -1)
        return;
    func_L00_003EFA98(obj);
    func_L00_003EFD00(obj);
    func_L00_003F0130(obj);
}

INCLUDE_ASM("asm/levels", func_L00_003EFA98);

INCLUDE_ASM("asm/levels", func_L00_003EFD00);

INCLUDE_ASM("asm/levels", func_L00_003EFF58);

extern int func_00282948(char *p);

extern void func_L00_003F0680(char *rec);

void func_L00_003F00A0(char *obj)
{
    char *rec;
    int i;
    int offset;

    offset = 0;
    i = 0;
    do {
        i--;
        rec = (char *)(offset + (int)D_L00_001B2BB4_1);
        if (*(int *)(rec + 0x30) == (int)obj) {
            if (func_00282948(rec + 0x34) != 0 && *(int *)(rec + 0x38) == 0)
                *(int *)(rec + 0x30) = 0;
            func_L00_003F0680(rec);
        }
        offset += 0x50;
    } while (i >= 0);
}

extern void func_L00_003F09A0(char *rec);

void func_L00_003F0130(char *obj)
{
    char *rec;
    int i;
    int offset;

    offset = 0;
    i = 0;
    do {
        i--;
        rec = (char *)(offset + (int)D_L00_001B2BB4_1);
        if (*(int *)(rec + 0x30) == (int)obj)
            func_L00_003F09A0(rec);
        offset += 0x50;
    } while (i >= 0);
}

INCLUDE_ASM("asm/levels", func_L00_003F0198);

INCLUDE_ASM("asm/levels", func_L00_003F0680);

extern char *D_L00_001B2BB8 __attribute__((sda));

extern void ref_L00_003F09A0_00(char *rec, float f1, int a1, int a2, int a3,
                                float f2, int a4, float f3);

void func_L00_003F09A0(char *obj)
{
    char *rec;
    int i;

    for (i = 0; i < 320; i++) {
        rec = (char *)(i * 128 + (int)D_L00_001B2BB8);
        if (*(int *)(rec + 0x7C) == (int)obj)
            ref_L00_003F09A0_00(rec, *(float *)(rec + 0x48), *(int *)(rec + 0x68),
                                *(int *)(rec + 0x60), 46, *(float *)(rec + 0x34),
                                *(int *)(rec + 0x70), 0.25f);
    }
}

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003F0A20(char *p)
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
void func_L00_003F0AA8(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L00_003F0B10);

INCLUDE_ASM("asm/levels", func_L00_003F0CC8);

INCLUDE_ASM("asm/levels", func_L00_003F1300);

INCLUDE_ASM("asm/levels", func_L00_003F1448);

/* resets a state record: zero words, plus/minus one float flags and two 64-bit fields in the retail store order */
void func_L00_003F1DC8(char *p)
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
void func_L00_003F1E50(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L00_003F1EB8);

INCLUDE_ASM("asm/levels", func_L00_003F26C8);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003F28A0(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L00_003F3160);

void func_L00_003F34D8(char *src, int *src4, char *dst16, int *dst4, int *count)
{
    int i = *count;

    if (i < 0x46) {
        qcopy(dst16 + i * 16, src);
        dst4[i] = *src4;
        (*count)++;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003F3518);

extern void ref_L00_003F37A0_00(void *p, char *obj);

extern void func_L00_003F37E8(void);

typedef struct Obj_L00_003F37A0 {
    unsigned char pad0[0x20];
    unsigned char f20;
    unsigned char pad21[0xC0 - 0x21];
    float fC0;
    unsigned char padC4[0xD4 - 0xC4];
    float fD4;
    unsigned char padD8[0xE8 - 0xD8];
    float fE8;
} Obj_L00_003F37A0;

void func_L00_003F37A0(Obj_L00_003F37A0 *obj)
{
    if (obj->f20 == 0) {
        obj->f20 = 1;
        obj->fC0 = 1.0f;
        obj->fD4 = 1.0f;
        obj->fE8 = 1.0f;
    }
    ref_L00_003F37A0_00((void *)func_L00_003F37E8, (char *)obj);
}

INCLUDE_ASM("asm/levels", func_L00_003F37E8);

extern void func_L00_003F3AD0(void);

typedef struct Obj_L00_003F3A88 {
    unsigned char pad0[0x20];
    unsigned char f20;
    unsigned char pad21[0xC0 - 0x21];
    float fC0;
    unsigned char padC4[0xD4 - 0xC4];
    float fD4;
    unsigned char padD8[0xE8 - 0xD8];
    float fE8;
} Obj_L00_003F3A88;

void func_L00_003F3A88(Obj_L00_003F3A88 *obj)
{
    if (obj->f20 == 0) {
        obj->f20 = 1;
        obj->fC0 = 1.0f;
        obj->fD4 = 1.0f;
        obj->fE8 = 1.0f;
    }
    ref_L00_003F3A88_00((void *)func_L00_003F3AD0, (char *)obj);
}

INCLUDE_ASM("asm/levels", func_L00_003F3AD0);

INCLUDE_ASM("asm/levels", func_L00_003F3D70);

INCLUDE_ASM("asm/levels", func_L00_003F4210);

INCLUDE_ASM("asm/levels", func_L00_003F4B68);

INCLUDE_ASM("asm/levels", func_L00_003F4C68);

INCLUDE_ASM("asm/levels", func_L00_003F4EC8);

/* Computes two sub-record addresses of a 192-byte block and hands three of its sub-records to four helpers, threading a second argument through two of them. */
void func_L00_003F51A0(char *obj, char *arg)
{
    char *base = obj + 192;
    char *other = obj + 208;
    float one = 1.0f;

    func_00282F20_1(base, (char *)(*(int *)(obj + 104)), one);
    func_00282DD8(other, base, arg);
    func_00282F20_1(other, other, one);
    func_00282DD8(obj + 224, other, base);
}

INCLUDE_ASM("asm/levels", func_L00_003F5238);

INCLUDE_ASM("asm/levels", func_L00_003F5390);

void func_L00_003F5658(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003F5660);

/* Reads the short at +26 of the object held at +104, calls a callee with (3076, that short), then calls one callee with this when the result is zero or three callees with this when it is non-zero. */
void func_L00_003F56F0(char *p)
{
    if (func_002B4038(3076, *(short *)(*(int *)(p + 104) + 26)) == 0)
        func_0029F338_1(p);
    else {
        func_L00_003F5B10(p);
        func_L00_003F6308(p);
        func_L00_003F6B10(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003F5750);

INCLUDE_ASM("asm/levels", func_L00_003F5870);

INCLUDE_ASM("asm/levels", func_L00_003F59F0);

INCLUDE_ASM("asm/levels", func_L00_003F5B10);

INCLUDE_ASM("asm/levels", func_L00_003F5F90);

INCLUDE_ASM("asm/levels", func_L00_003F6168);

INCLUDE_ASM("asm/levels", func_L00_003F6308);

INCLUDE_ASM("asm/levels", func_L00_003F6900);

/* Walks 120 records of 64 bytes from a base resolved through two descriptor lookups; for each live record it calls an update helper and, when that reports zero, counts it and advances a float pair by one step, clearing the record's live flag otherwise. */
int func_L00_003F6A58(int *a0)
{
    int i;
    int count = 0;
    char *base;
    char *rec;
    char *obj;

    rec = *(char **)((char *)a0 + 104);
    base = func_L00_0032A088(3076, *(short *)(rec + 26));
    base = base + *(int *)(func_L00_0032A0B8(3076, *(short *)(rec + 26)) + 4);
    obj = base + 16;
    rec = base;

    for (i = 119; i >= 0; i--) {
        if (*(short *)(rec + 18) != 0) {
            if (func_00282978(obj) != 0) {
                *(short *)(rec + 18) = 0;
            } else {
                count++;
                *(float *)(rec + 24) = func_00283B98(*(float *)(rec + 24), *(float *)(rec + 28));
            }
        }
        obj += 64;
        rec += 64;
    }
    return count;
}

INCLUDE_ASM("asm/levels", func_L00_003F6B10);

extern void func_002AD0E0(char *a0, char *a1, char *a2, char *a3, char *a4, char *a5);

extern void func_0029F338(char *a0);

extern int func_00282978(char *a0);

extern void ref_L00_003F71E8_01(void *p, char *obj);

extern void ref_L00_003F71E8_00(void);

void func_L00_003F71E8(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);

    if (*(char **)(s1 + 0x3C) != 0) {
        func_002AD0E0(a0, *(char **)(s1 + 0x3C), a0 + 0x10, a0 + 0xF0, a0 + 0x10, a0 + 0xF0);
    }
    if (func_00282978(s1) == 0) {
        func_00282978(s1 + 4);
        ref_L00_003F71E8_01((void *)ref_L00_003F71E8_00, a0);
    } else {
        func_0029F338(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003F7268);

INCLUDE_ASM("asm/levels", func_L00_003F8280);

INCLUDE_ASM("asm/levels", func_L00_003F87B0);

/* Same body as 23626ad829daa908 with the node field 4 bytes earlier: calls a helper with a referenced node and three identity bytes, then copies them into a linked record guarded by a flag bit and a non-zero leading short. */
void func_L00_003F89F8(Obj_Fe48725c6 *p)
{
    Node24_Fe48725c6 *s;
    Node16_Fe48725c6 *d;
    int v;

    func_002A0488_1(p->f24, p->u.b.b4, p->u.b.b5, p->u.b.h6);
    s = p->f24;
    if ((s->h52 & 0x20) == 0)
        return;
    d = ((Node104_Fe48725c6 *)s->p104)->p16;
    if (d == 0)
        return;
    if (d->h0 == 0)
        return;
    d->b4 = p->u.b.b4;
    d->b5 = p->u.f4 >> 8;
    v = p->u.b.h6;
    d->b6 = v;
}

