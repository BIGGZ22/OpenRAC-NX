#include "include_asm.h"

typedef int s32;

typedef float f32;

extern float func_00283BE0_4(float x, float y);

extern float func_00283180_7(float x);

extern float func_00283B98_7(float x, float y);

extern float func_00283180_5(float x);

extern float func_00283BE0_2(float a, float b);

extern float func_00283B98_5(float a, float b);

typedef unsigned char u8;

#ifndef RAC2_T_V4_F36D8A248
#define RAC2_T_V4_F36D8A248
typedef struct {
    f32 x, y, z, w;
} V4_F36d8a248;
#endif

#ifndef RAC2_T_M4_F36D8A248
#define RAC2_T_M4_F36D8A248
typedef struct {
    V4_F36d8a248 r[4];
} M4_F36d8a248;
#endif

extern void func_00283410_3(M4_F36d8a248 *, V4_F36d8a248 *);

extern void func_00283788_2(M4_F36d8a248 *, M4_F36d8a248 *, V4_F36d8a248 *);

extern void func_002AB6D8_2(M4_F36d8a248 *, V4_F36d8a248 *);

typedef int BatchA_A06_s32;

typedef short BatchA_A06_BatchAs16;

#ifndef RAC2_T_SRC_F66766F2B
#define RAC2_T_SRC_F66766F2B
typedef struct {
    char pad0[35];        /* 0   */
    unsigned char f35;    /* 35  */
    char pad36[8];        /* 36  */
    float f44;            /* 44  */
    unsigned char f48;    /* 48  */
    unsigned char f49;    /* 49  */
    unsigned short f50;   /* 50  */
    unsigned short f52;   /* 52  */
    char pad54[2];        /* 54  */
    long long f56;        /* 56  */
    char pad64[34];       /* 64  */
    unsigned char f98;    /* 98  */
    char pad99[21];       /* 99  */
    int f120;             /* 120 */
    unsigned short f124;  /* 124 */
    char pad126[26];      /* 126 */
    int f152;             /* 152 */
    char pad156[14];      /* 156 */
    short f170;           /* 170 */
} Src_F66766f2b;
#endif

#ifndef RAC2_T_LIST_F66766F2B
#define RAC2_T_LIST_F66766F2B
typedef struct {
    char pad0[256];       /* 0   */
    int a[4];             /* 256 */
    int b[4];             /* 272 */
    Src_F66766f2b *c[4];            /* 288 */
    float d[4];           /* 304 */
    int pad320;           /* 320 */
    Src_F66766f2b *src;             /* 324 */
    int count;            /* 328 */
} List_F66766f2b;
#endif

extern Src_F66766f2b *func_0029EFD0(short);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_5b4b17178a13f443_SmallSlotReset128Owner {
    unsigned char prefix_00[0x120];
    void *slots_120[10];
    int count_148;
    int active_14c;
} Rac2Native_5b4b17178a13f443_SmallSlotReset128Owner;

extern void func_0029F338_3(void *object);

typedef unsigned int u32;

typedef struct {
    u8 prefix[0x40];
    void *payload;
    u32 unused44;
    short key;
    u8 marker;
    u8 unused4b;
    u8 selector;
    u8 tail[3];
} GornRecordWrite64;

extern f32 func_00283198_4(f32);

extern f32 func_00283B98_4(f32, f32);

extern void func_00283D38_2(s32, s32, f32);

INCLUDE_ASM("asm/levels", func_L00_00316F60);

/* computes a scalar complement-power curve by repeated multiplication */
f32 func_L00_0031A6C8(f32 value, s32 count) {
    f32 factor = 1.0f - value;
    f32 product = factor;
    for (; count > 1; --count)
        product = product * factor;
    return 1.0f - product;
}

INCLUDE_ASM("asm/levels", func_L00_0031A710);

INCLUDE_ASM("asm/levels", func_L00_0031AA28);

/* Blends a value toward a target: helper(second,first) halved, scaled by (1 - helper(third*pi)); the first argument and that product go to a two-float helper whose float result is returned. */
float func_L00_0031ABE0(float a, float b, float c)
{
    float t;
    float u;

    t = func_00283BE0_4(b, a);
    u = func_00283180_7(c * 3.1415927f);
    t = t * 0.5f;
    return func_00283B98_7(a, t * (1.0f - u));
}

INCLUDE_ASM("asm/levels", func_L00_0031AC60);

/* converts a normalised phase to radians, derives a blend factor of half the complement of a phase helper's result, and applies it to a two-argument helper over three consecutive float components written back over the first vector. */
void func_L00_0031AD40(float *a0, float *a1, float *a2, float t)
{
    float k;

    k = (1.0f - func_00283180_5(t * 3.1415927f)) * 0.5f;
    a0[0] = func_00283B98_5(a1[0], func_00283BE0_2(a2[0], a1[0]) * k);
    a0[1] = func_00283B98_5(a1[1], func_00283BE0_2(a2[1], a1[1]) * k);
    a0[2] = func_00283B98_5(a1[2], func_00283BE0_2(a2[2], a1[2]) * k);
}

INCLUDE_ASM("asm/levels", func_L00_0031AFF0);

INCLUDE_ASM("asm/levels", func_L00_0031B398);

INCLUDE_ASM("asm/levels", func_L00_0031B750);

INCLUDE_ASM("asm/levels", func_L00_0031B9F8);

INCLUDE_ASM("asm/levels", func_L00_0031BC48);

INCLUDE_ASM("asm/levels", func_L00_0031BDC0);

INCLUDE_ASM("asm/levels", func_L00_0031BE88);

INCLUDE_ASM("asm/levels", func_L00_0031C0F8);

INCLUDE_ASM("asm/levels", func_L00_0031C3D0);

INCLUDE_ASM("asm/levels", func_L00_0031C978);

INCLUDE_ASM("asm/levels", func_L00_0031C998);

INCLUDE_ASM("asm/levels", func_L00_0031CAC8);

INCLUDE_ASM("asm/levels", func_L00_0031CDD0);

INCLUDE_ASM("asm/levels", func_L00_0031D618);

INCLUDE_ASM("asm/levels", func_L00_0031D620);

INCLUDE_ASM("asm/levels", func_L00_0031D670);

INCLUDE_ASM("asm/levels", func_L00_0031D958);

INCLUDE_ASM("asm/levels", func_L00_0031EC88);

INCLUDE_ASM("asm/levels", func_L00_0031F1C0);

INCLUDE_ASM("asm/levels", func_L00_0031F220);

/* One fixed ordinary scalar query. Signed halfword width, offset and
   equality constant are measured; no original class or field-name claim. */
int func_L00_0031F2B0(const unsigned char *entity) {
    return *(const signed short *)(entity + 0xAA) == 0x0CDB;
}

INCLUDE_ASM("asm/levels", func_L00_003200F8);

INCLUDE_ASM("asm/levels", func_L00_00320460);

INCLUDE_ASM("asm/levels", func_L00_003206B8);

INCLUDE_ASM("asm/levels", func_L00_003207F8);

INCLUDE_ASM("asm/levels", func_L00_00320B70);

INCLUDE_ASM("asm/levels", func_L00_00321878);

INCLUDE_ASM("asm/levels", func_L00_00322428);

INCLUDE_ASM("asm/levels", func_L00_00322A10);

/* builds two 64-byte blocks from a 3-float vector and from a vector inside the caller object, combines them, then scales one block again from the same source vector */
void func_L00_00322A18(u8 *a0, f32 a1, f32 a2, f32 a3) {
    V4_F36d8a248 in;
    M4_F36d8a248 m1;
    M4_F36d8a248 m2;
    in.x = a1;
    in.y = a2;
    in.z = a3;
    func_00283410_3(&m1, &in);
    func_00283410_3(&m2, (V4_F36d8a248 *)(a0 + 240));
    func_00283788_2(&m2, &m2, &m1);
    func_002AB6D8_2(&m2, (V4_F36d8a248 *)(a0 + 240));
}

/* A06-inline-init-40 freshly exact-verified under family reservation */
void func_L00_00322A98(void *a0, char *a1){
    if (*(BatchA_A06_s32 *)(a1 + 0x14C) == 0) {
        *(void **)(a1 + 0x144) = a0;
        *(BatchA_A06_s32 *)(a1 + 0x14C) = 1;
        *(BatchA_A06_BatchAs16 *)(a1 + 0x142) = 0;
        *(BatchA_A06_s32 *)(a1 + 0x148) = 0;
        *(BatchA_A06_BatchAs16 *)(a1 + 0x140) = 0;
    }
}

/* Appends a record to a fixed four-entry table: aborts when the table is full or an index argument is out of range, copies selected fields from one linked record into a helper-allocated record, then stores the three inputs and that record into four parallel tables and bumps the count. */
void func_L00_00322AC0(List_F66766f2b *obj, int a1, int a2, float v)
{
    Src_F66766f2b *q;
    short n;

    if (obj->count == 4)
        return;
    if (a2 >= 9)
        return;

    q = func_0029EFD0(obj->src->f170);
    if (q == 0)
        return;

    n = q->f170;
    q->f124 = obj->src->f124;
    q->f50 = obj->src->f50;
    q->f56 = obj->src->f56;
    q->f44 = obj->src->f44;
    q->f120 = obj->src->f120;
    q->f49 = 1;
    q->f52 = 2058;
    q->f48 = 0;
    q->f152 = 0;
    if (n == 0)
        q->f98 = 0;
    q->f35 = a1;

    obj->b[obj->count] = a2;
    obj->a[obj->count] = a1;
    obj->d[obj->count] = v;
    obj->c[obj->count] = q;
    obj->count = obj->count + 1;
}

INCLUDE_ASM("asm/levels", func_L00_00322BE0);

/* When active field 0x14C is nonzero, iterate the slots beginning at 0x120 using count 0x148, call the bound helper for each non-null slot, clear each processed slot, and reset the active field. Original game role is unknown. */
void func_L00_00322EC8(Rac2Native_5b4b17178a13f443_SmallSlotReset128Owner *owner)
{
    int index;
    if (owner->active_14c != 0) {
        for (index = 0; index < owner->count_148; ++index) {
            if (owner->slots_120[index] != 0) {
                func_0029F338_3(owner->slots_120[index]);
                owner->slots_120[index] = 0;
            }
        }
        owner->active_14c = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L00_00322F48);

INCLUDE_ASM("asm/levels", func_L00_00323338);

INCLUDE_ASM("asm/levels", func_L00_00323448);

INCLUDE_ASM("asm/levels", func_L00_00323958);

/* Write a payload pointer in the first selector/key record before the list terminator */
void func_L00_00323E48(GornRecordWrite64 *record, u32 selector,
                            int key, void *payload)
{
    if (record->marker != 0) {
        u8 marker;
        do {
            if (record->selector == selector && record->key == key) {
                record->payload = payload;
                return;
            }
            marker = record->marker;
            ++record;
        } while (marker != 1);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00323F28);

INCLUDE_ASM("asm/levels", func_L00_00323F70);

INCLUDE_ASM("asm/levels", func_L00_00324120);

INCLUDE_ASM("asm/levels", func_L00_00324D18);

INCLUDE_ASM("asm/levels", func_L00_00324E48);

INCLUDE_ASM("asm/levels", func_L00_00324F10);

INCLUDE_ASM("asm/levels", func_L00_00324F18);

INCLUDE_ASM("asm/levels", func_L00_003252A8);

INCLUDE_ASM("asm/levels", func_L00_003252C8);

INCLUDE_ASM("asm/levels", func_L00_00325300);

INCLUDE_ASM("asm/levels", func_L00_003253A0);

INCLUDE_ASM("asm/levels", func_L00_003255A0);

INCLUDE_ASM("asm/levels", func_L00_00325600);

INCLUDE_ASM("asm/levels", func_L00_00325760);

INCLUDE_ASM("asm/levels", func_L00_00325AE8);

INCLUDE_ASM("asm/levels", func_L00_00325CC0);

INCLUDE_ASM("asm/levels", func_L00_00325FC0);

/* stores a wrapped angle back into the caller field and forwards its [0,1] rescaled value (x*0.5+0.5) with two pointer arguments to a second callee */
void func_L00_00325FC8(f32 *a0, s32 a1, s32 a2, f32 a3) {
    f32 v = func_00283198_4(a0[0]);
    f32 s = v * 0.5f + 0.5f;
    a0[0] = func_00283B98_4(a0[0], a3);
    func_00283D38_2(a1, a2, s);
}

INCLUDE_ASM("asm/levels", func_L00_00326048);

INCLUDE_ASM("asm/levels", func_L00_00326080);

INCLUDE_ASM("asm/levels", func_L00_00326108);

INCLUDE_ASM("asm/levels", func_L00_00326110);

INCLUDE_ASM("asm/levels", func_L00_00326200);

INCLUDE_ASM("asm/levels", func_L00_003263D0);

INCLUDE_ASM("asm/levels", func_L00_00326620);

INCLUDE_ASM("asm/levels", func_L00_003266D8);

INCLUDE_ASM("asm/levels", func_L00_003268E8);

INCLUDE_ASM("asm/levels", func_L00_00326950);

/* One fixed ordinary scalar source hypothesis. Byte offset and mask are
   measured; no original object/class/field name or allocation claim. The
   observed callers pass scalar one and ignore the result. */
void func_L00_003294F0(unsigned char *entity, int enabled) {
    if (enabled)
        entity[0xBE] |= 4;
    else
        entity[0xBE] &= 0xFB;
}

INCLUDE_ASM("asm/levels", func_L00_00329520);

INCLUDE_ASM("asm/levels", func_L00_003296C0);

INCLUDE_ASM("asm/levels", func_L00_00329E90);

INCLUDE_ASM("asm/levels", func_L00_00329FF0);

INCLUDE_ASM("asm/levels", func_L00_0032A088);

INCLUDE_ASM("asm/levels", func_L00_0032A0B8);

INCLUDE_ASM("asm/levels", func_L00_0032A0E8);

INCLUDE_ASM("asm/levels", func_L00_0032A138);

INCLUDE_ASM("asm/levels", func_L00_0032A170);

INCLUDE_ASM("asm/levels", func_L00_0032A1A8);

INCLUDE_ASM("asm/levels", func_L00_0032A258);

INCLUDE_ASM("asm/levels", func_L00_0032A530);

INCLUDE_ASM("asm/levels", func_L00_0032AA68);

INCLUDE_ASM("asm/levels", func_L00_0032B268);
