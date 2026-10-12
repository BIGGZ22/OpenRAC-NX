#include "include_asm.h"

typedef float f32;

typedef unsigned char u8;

extern void F4778f810_FUN_002B00D8(char *a, char *b, char *c, f32 d);

extern void F4778f810_FUN_00282CC0(char *a, char *b, char *c);

extern void F4778f810_FUN_00283098(char *a, char *b, char *c);

extern void F4778f810_FUN_00283958(char *a, char *b);

extern void F4778f810_FUN_00283738(char *a, char *b, char *c);

extern void F4778f810_FUN_00282CF0(char *a, char *b, char *c);

#ifndef RAC2_T_V4_F4778F810
#define RAC2_T_V4_F4778F810
typedef struct {
    f32 x, y, z, w;
} V4_F4778f810;
#endif

#ifndef RAC2_T_SUB_F4778F810
#define RAC2_T_SUB_F4778F810
typedef struct {
    u8 pad000[0x10];
    u8 at010[0x10];
    u8 at020[0x28];
    f32 f048;
} Sub_F4778f810;
#endif

#ifndef RAC2_T_OBJ_F4778F810
#define RAC2_T_OBJ_F4778F810
typedef struct {
    u8 pad000[0x10];
    u8 at010[0x58];
    char *p068;
    u8 pad06C[0xC0 - 0x6C];
    u8 at0C0[0x10];
} Obj_F4778f810;
#endif

/* Append an observed point index and update the original geometric descriptor. */
typedef struct {
    unsigned char gap0[0x10];
    float plane[4];
    unsigned char gap20[0x10];
    float (*points)[4];
    unsigned char gap34[0x25];
    unsigned char indices[3];
    unsigned char count;
} OozlaAppendDescriptor164;

typedef struct {
    unsigned char gap0[0x68];
    OozlaAppendDescriptor164 *descriptor;
    unsigned char gap6c[0x54];
    float transform[3][4];
} OozlaAppendObject164;

extern void func_00274810(OozlaAppendObject164 *, int, const float *);

extern void func_00282CF0(float *, const float *, const float *);

extern void func_00283098(float *, const float *, const float *);

extern void func_00274B58(OozlaAppendObject164 *, float *, const float *);

extern void func_00283028(void *a, void *b, void *c);

extern float func_00282DF0_5(void *a);

extern float func_00282DB0_3(void *a, void *b);

extern void func_002AA2E8_2(float *p, float a, float b);

extern void func_00282F20_16(void *a, void *b, float f);

/* Measured partial layouts; original type and game roles are unknown. */
struct Rac2Native_00169805055547ed_Rec31cd0302 {
    int v[10];
};

extern struct Rac2Native_00169805055547ed_Rec31cd0302 D_L04_001BCE48[];

extern char Fd8e166aa_D_001B7F80[];

extern int Fcdb4d905_FUN_0027A0F8(int a, int b, int c);

void func_00274128(Obj_F4778f810 *obj)
{
    V4_F4778f810 tmp[6];
    Sub_F4778f810 *p = (Sub_F4778f810 *)obj->p068;

    F4778f810_FUN_002B00D8((char *)obj + 0x10, (char *)p, (char *)p, p->f048);
    F4778f810_FUN_00282CC0((char *)obj + 0x10, (char *)obj + 0x10, (char *)p);
    F4778f810_FUN_00283098((char *)&tmp[0], (char *)p + 0x20, (char *)obj + 0xC0);
    F4778f810_FUN_00283958((char *)p + 0x10, (char *)&tmp[3]);
    F4778f810_FUN_00283738((char *)obj + 0xC0, (char *)&tmp[3], (char *)obj + 0xC0);
    F4778f810_FUN_00283098((char *)&tmp[1], (char *)p + 0x20, (char *)obj + 0xC0);
    F4778f810_FUN_00282CF0((char *)&tmp[2], (char *)&tmp[1], (char *)&tmp[0]);
    F4778f810_FUN_00282CF0((char *)obj + 0x10, (char *)obj + 0x10, (char *)&tmp[2]);
}

void func_00274208(Obj_F4778f810 *obj)
{
    V4_F4778f810 tmp[6];
    Sub_F4778f810 *p = (Sub_F4778f810 *)obj->p068;

    F4778f810_FUN_002B00D8((char *)obj + 0x10, (char *)p, (char *)p, p->f048);
    F4778f810_FUN_00282CC0((char *)obj + 0x10, (char *)obj + 0x10, (char *)p);
    F4778f810_FUN_00283098((char *)&tmp[0], (char *)p + 0x20, (char *)obj + 0xC0);
    F4778f810_FUN_00283958((char *)p + 0x10, (char *)&tmp[3]);
    F4778f810_FUN_00283738((char *)obj + 0xC0, (char *)&tmp[3], (char *)obj + 0xC0);
    F4778f810_FUN_00283098((char *)&tmp[1], (char *)p + 0x20, (char *)obj + 0xC0);
    F4778f810_FUN_00282CF0((char *)&tmp[2], (char *)&tmp[1], (char *)&tmp[0]);
    F4778f810_FUN_00282CF0((char *)obj + 0x10, (char *)obj + 0x10, (char *)&tmp[2]);
}

INCLUDE_ASM("asm/game", func_002742E8);

INCLUDE_ASM("asm/game", func_00274500);

INCLUDE_ASM("asm/game", func_002746C8);

/* appendindexbyte59/count5C descriptor68, originalthreeGPR geometricupdate inclincomingdirectionA2, ifcount!=1 originalpointdifference24/transform40/bound80 andterminalcountbyte return */
unsigned int func_00274768(OozlaAppendObject164 *object, unsigned int index, const float *direction)
{
    float difference[4];
    OozlaAppendDescriptor164 *descriptor = object->descriptor;
    descriptor->indices[descriptor->count] = index;
    ++descriptor->count;
    func_00274810(object, (int)descriptor->count - 1, direction);
    if (descriptor->count != 1) {
        func_00282CF0(difference, descriptor->points[descriptor->indices[0]], descriptor->points[descriptor->indices[1]]);
        func_00283098(difference, difference, &object->transform[0][0]);
        func_00274B58(object, descriptor->plane, difference);
    }
    return descriptor->count;
}

INCLUDE_ASM("asm/game", func_00274810);

INCLUDE_ASM("asm/game", func_00274A88);

INCLUDE_ASM("asm/game", func_00274B58);

/* Keeps the float one helper returns, and when a later helper reports a ratio above 0.7 scales it by a factor with a floor, hands it to a setter through a local, then reports whether the kept value is non-zero. */
int func_00274BA8(void *p, void *q, void *r, float f12, float f13)
{
    float saved;
    float t;
    float v;

    func_00283028(p, p, q);
    saved = func_00282DF0_5(p);
    v = func_00282DB0_3(r, q);
    if (v > 0.7f) {
        t = saved * f12;
        if (t < f13) t = f13;
        func_002AA2E8_2(&saved, 0.0f, t);
        func_00282F20_16(p, p, saved);
    }
    if (saved != 0.0f) return 1;
    return 0;
}

INCLUDE_ASM("asm/game", func_00274C90);

INCLUDE_ASM("asm/game", func_00274D90);

INCLUDE_ASM("asm/game", func_00276158);

INCLUDE_ASM("asm/game", func_002761F8);

INCLUDE_ASM("asm/game", func_00277258);

INCLUDE_ASM("asm/game", func_00277F48);

INCLUDE_ASM("asm/game", func_00278870);

INCLUDE_ASM("asm/game", func_00278CE0);

INCLUDE_ASM("asm/game", func_00278E78);

INCLUDE_ASM("asm/game", func_00278EA8);

INCLUDE_ASM("asm/game", func_00278EB0);

INCLUDE_ASM("asm/game", func_00278F80);

INCLUDE_ASM("asm/game", func_00278FC0);

/* Store ten integer arguments in the indexed record when the unsigned index is below 32; otherwise leave the table unchanged. */
void func_00279790(int p0, int p1, int p2, int p3, int p4, int p5,
                                      int p6, int p7, int p8, int p9, unsigned idx)
{
    if (idx >= 32u)
        return;
    D_L04_001BCE48[idx].v[0] = p0;
    D_L04_001BCE48[idx].v[1] = p1;
    D_L04_001BCE48[idx].v[2] = p2;
    D_L04_001BCE48[idx].v[3] = p3;
    D_L04_001BCE48[idx].v[4] = p4;
    D_L04_001BCE48[idx].v[5] = p5;
    D_L04_001BCE48[idx].v[6] = p6;
    D_L04_001BCE48[idx].v[7] = p7;
    D_L04_001BCE48[idx].v[8] = p8;
    D_L04_001BCE48[idx].v[9] = p9;
}

INCLUDE_ASM("asm/game", func_00279808);

void func_00279828(void)
{
    char *g = Fd8e166aa_D_001B7F80;
    int i = 0;
    int *table = *(int **)(g + 8);
    char *p = (char *)table + table[*(unsigned char *)0x1A7BB4];
    int n = *(int *)p;

    *(char **)(g + 12) = p + 8;
    *(int *)(g + 16) = n;
    for (i = 0; i < *(int *)(g + 16); i++) {
        char *q = *(char **)(g + 12);
        char *r = q - 8;

        *(int *)(q + i * 16) = *(int *)(q + i * 16) + (int)r;
    }
}

INCLUDE_ASM("asm/game", func_002798A0);

INCLUDE_ASM("asm/game", func_00279988);

INCLUDE_ASM("asm/game", func_002799A8);

INCLUDE_ASM("asm/game", func_00279B68);

INCLUDE_ASM("asm/game", func_00279B70);

INCLUDE_ASM("asm/game", func_00279BD0);

INCLUDE_ASM("asm/game", func_00279BD8);

INCLUDE_ASM("asm/game", func_00279D18);

INCLUDE_ASM("asm/game", func_00279FF0);

/* shared native body of family 74360f8d53836176 */
int func_0027A0F8(int a0, int a1, int a2)
{
    unsigned char *base = *(unsigned char **)0x001B23C0;
    unsigned char *end = base + *(int *)base;
    unsigned short *node = (unsigned short *)(base + 4);
    int i2;
    int i1;
    int i0;

    i2 = a2 - node[0];
    if (i2 < 0) return 0;
    if (!(i2 < node[1])) return 0;
    if (!node[i2 + 2]) return 0;
    node = (unsigned short *)(base + node[i2 + 2] * 4);
    i1 = a1 - node[0];
    if (i1 < 0) return 0;
    if (!(i1 < node[1])) return 0;
    if (!node[i1 + 2]) return 0;
    node = (unsigned short *)(base + node[i1 + 2] * 4);
    i0 = a0 - node[0];
    if (i0 < 0 || !(i0 < node[1])) return 0;
    if (node[i0 + 2] == 0xFFFF) return 0;
    return (int)(end + node[i0 + 2] * 128);
}

int func_0027A1D0(float x, int a, int b, int c, int d, int e, int g)
{
    int r, p, q, s;
    if (x < 0.5f) {
        r = Fcdb4d905_FUN_0027A0F8(a, b, c);
        if (r) return r;
        p = d;
        q = e;
        s = g;
    } else {
        r = Fcdb4d905_FUN_0027A0F8(d, e, g);
        if (r) return r;
        p = a;
        q = b;
        s = c;
    }
    return Fcdb4d905_FUN_0027A0F8(p, q, s);
}

INCLUDE_ASM("asm/game", func_0027A288);

INCLUDE_ASM("asm/game", func_0027A9A0);

INCLUDE_ASM("asm/game", func_0027A9F0);

INCLUDE_ASM("asm/game", func_0027AB40);

INCLUDE_ASM("asm/game", func_0027B2F8);

INCLUDE_ASM("asm/game", func_0027B440);
