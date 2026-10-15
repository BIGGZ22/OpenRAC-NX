#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct {
    short a;
    short b;
} Rac2Native_03d37aca6ffcf794_T2;

extern int D_L04_00220D60 __attribute__((nosda));

extern int *D_L04_0021F2E0[];

extern Rac2Native_03d37aca6ffcf794_T2 D_L04_00220860[];

extern int *D_L05_001B2C88 __attribute__((sda));

extern int *D_L05_001B2AA0 __attribute__((sda));

extern int D_001A72D0 __attribute__((sda));

extern int D_001A72D4 __attribute__((sda));

extern int D_L05_001B1FBC __attribute__((sda));

typedef struct { int *saved; } SavedSlot;

extern SavedSlot D_L05_001B2AC0 __attribute__((sda));

typedef struct {
    char gap0[12];
    short count;
    char gap14[2];
    long long *table;
} ResidentTable;

extern void Fa2d20de7_FUN_002A7988(void *object, float first, float second);

extern void Fa2d20de7_FUN_00282CC0(char *first, char *second, void *third);

extern int Fa2d20de7_FUN_002761F8(void *first, char *second, int mode, int value, int extra);

extern void Fa2d20de7_FUN_00282CF0(void *first, void *second, void *third);

extern void Fa2d20de7_FUN_00282D30(void *first, void *second, float value);

extern short Fa2d20de7_D_001B5340[];

extern short Fa2d20de7_D_001BAFA0[];

extern int Fa2d20de7_D_001886CC[];

extern char F391de845_D_001B5340[];

extern float F391de845_FUN_00282E48(char *a, char *b);

extern float F391de845_FUN_002E45C8(void *self, float d, float x, float y);

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

extern u8 D_00188660[];

extern u8 D_00189E20_10[];

extern u8 D_00188660_1[];

typedef struct {
    u8 pad0[4640];
    u32 field4640;
    u8 gap[4204];
    u32 field8848;
} NativeView;

extern int F29eece9b_FUN_002E5AD0(char *p, int a1, char *a2, int a3, int a4);

extern char F29eece9b_D_00188660[];

typedef struct { u8 before[0x1730]; s32 count; u8 *table; } CallbackGlobals;

extern s32 func_00133230(void);

extern s32 func_00132028(void);

extern s32 func_00132AC8(void);

/* Walk a terminated index list and nested records, clearing the low fourteen bits and ORing nonzero replacement values into two selected words. */
void func_002E10B8(void)
{
    int *wp;
    int *ent;
    int i;
    int j;
    int k;
    int *arr;
    int *pe;
    int *base;
    int *e;

    for (wp = &D_L04_00220D60; *wp >= 0; wp++) {
        ent = D_L04_0021F2E0[*wp];
        for (i = 0; i < *(short *)((char *)ent + 40); i++) {
            arr = ent + 16;
            pe = *(int **)((char *)arr + (i << 3));
            base = pe + 4;
            e = (int *)((char *)base + (base[1] << 4)) + 4;
            for (j = 0; j < base[0]; j++) {
                Rac2Native_03d37aca6ffcf794_T2 *s = &D_L04_00220860[((unsigned char *)e)[19]];
                if (s->a != 0)
                    e[12] = (e[12] & 0xFFFFC000) | s->a;
                if (s->b != 0)
                    e[8] = (e[8] & 0xFFFFC000) | s->b;
                e += 16;
            }
        }
    }
}

INCLUDE_ASM("asm/game", func_002E11C0);

INCLUDE_ASM("asm/game", func_002E1228);

ASM_FUNC("asm/game", func_002E2988);

INCLUDE_ASM("asm/game", func_002E2C38);

INCLUDE_ASM("asm/game", func_002E31E8);

INCLUDE_ASM("asm/game", func_002E3360);

INCLUDE_ASM("asm/game", func_002E3468);

INCLUDE_ASM("asm/game", func_002E3478);

extern char *ref_L00_002E3548_00 __attribute__((sda));
extern float ref_L00_002E3548_01[16] __attribute__((sda));
extern void func_00283388(float *);
extern void func_002E3478(int);
extern void func_002E3940(int);
void func_002E3548(void)
{
    int i;
    for (i = 0; i < *(short *)(ref_L00_002E3548_00 + 6); i++) {
        if (i >= 10)
            func_00283388(ref_L00_002E3548_01);
        else
            func_002E3478(i);
        func_002E3940(i);
    }
}

INCLUDE_ASM("asm/game", func_002E35D0);

INCLUDE_ASM("asm/game", func_002E35E0);

/* shared native body of family 060b9f2017649e67 */
void func_002E37B0(void)
{
    ResidentTable *resident = (ResidentTable *)D_L05_001B2AA0;
    int *current = D_L05_001B2C88;
    int index;

    D_L05_001B2AC0.saved = current;
    D_L05_001B2C88 = (int *)((char *)current + 16);
    D_001A72D0 = D_001A72D4;
    D_L05_001B1FBC = 0;
    for (index = 0; index < resident->count; ++index)
        *(long long *)((char *)resident->table + index * 16) = 0;
}

INCLUDE_ASM("asm/game", func_002E3820);

INCLUDE_ASM("asm/game", func_002E3938);

INCLUDE_ASM("asm/game", func_002E3940);

INCLUDE_ASM("asm/game", func_002E3998);

INCLUDE_ASM("asm/game", func_002E3BD8);

INCLUDE_ASM("asm/game", func_002E3E50);

ASM_FUNC("asm/game", func_002E3E54);

ASM_FUNC("asm/game", func_002E3FA0);

ASM_FUNC("asm/game", func_002E40C8);

ASM_FUNC("asm/game", func_002E425C);

INCLUDE_ASM("asm/game", func_002E4388);

void func_002E4478(void *object)
{
    Fa2d20de7_FUN_002A7988(object, 0.5f, 6.0f);
    Fa2d20de7_FUN_00282CC0(object, object, Fa2d20de7_D_001B5340);
    if (Fa2d20de7_FUN_002761F8(Fa2d20de7_D_001B5340, object, 130, Fa2d20de7_D_001886CC[0], 0)) {
        Fa2d20de7_FUN_00282CF0(object, Fa2d20de7_D_001BAFA0, Fa2d20de7_D_001B5340);
        Fa2d20de7_FUN_00282D30(object, object, 0.75f);
        Fa2d20de7_FUN_00282CC0(object, object, Fa2d20de7_D_001B5340);
    }
}

INCLUDE_ASM("asm/game", func_002E4528);

INCLUDE_ASM("asm/game", func_002E45C8);

float func_002E46C0(char *self, char *p)
{
    float v = F391de845_FUN_00282E48(p, F391de845_D_001B5340);
    float *q = *(float **)(self + 8);

    return F391de845_FUN_002E45C8(q, v, q[0], q[1]);
}

INCLUDE_ASM("asm/game", func_002E4708);

void func_002E4860(void)
{
    struct Layout { char reserved[0x48]; int a,b,c,d,e,f; };
    struct Layout *layout = (struct Layout *)D_00188660;
    int width = *(int *)0x1A7BA0;
    layout->a = width * 7 / 10;
    layout->b = *(int *)0x1A7B9C;
    layout->c = width;
    layout->d = width * 6 / 10;
    layout->e = 614;
    layout->f = width * 25 / 32;
}

INCLUDE_ASM("asm/game", func_002E48E8);

extern int D_001507E4;
extern int D_001A79F0 __attribute__((sda));
extern char D_001ABE10[];
extern void func_00133750(int, int);
extern int func_001337F0(void);
extern int func_00133820(void);
extern void func_0026FEB8(char *, int, int, int);

void func_002E4A68(void)
{
    func_00133750(2, D_001507E4);
    func_0026FEB8(D_001ABE10, D_001A79F0, func_001337F0(), func_00133820());
}

INCLUDE_ASM("asm/game", func_002E4AC8);

s32 func_002E59B0(s32 a0,s32 a1) {
    if(a1>=0) {
        u8 *p=D_00188660+a1*0x70;
        if(*(s32 *)(p+0x88)==a0 && (u32)p[0x74]-1<2) return 1;
    }
    return 0;
}

void func_002E59F8(s32 a0) {
    if(a0>=0) {
        u8 *p=D_00188660+a0*0x70;
        s32 state=p[0x74];
        if(state==7) {
            *(s32 *)(p+0x88)=0;
            *(s32 *)(p+0x8c)=0;
            p[0x74]=0;
            return;
        }
        if(state!=0 && state!=6) p[0x74]=4;
    }
}

/* shared native body of family 62e6ff2b3d3a */
s32 func_002E5A50(s32 id)
{
    u8 *record;
    u8 *cursor;
    s32 limit;

    limit = 52;
    if (id == 0 || (((NativeView *)&D_00189E20_10)->field8848 != (u32)id &&
                    ((NativeView *)&D_00189E20_10)->field4640 != (u32)id))
        limit = 42;

    id = 0;
    if (limit != 0) {
        record = (u8 *)&D_00188660_1;
        if (record[116] != 0) {
            cursor = record + 116;
            do {
                id++;
                if (id >= limit)
                    break;
                cursor += 112;
            } while (*cursor != 0);
        }
    }
    return (id != limit) ? id : 52;
}

INCLUDE_ASM("asm/game", func_002E5AD0);

int func_002E5CF8(int index, int a1, char *a2)
{
    int v0;
    int v1;
    int r;
    char *p;

    if (a2 == 0)
        return -1;
    v0 = *(int *)(a2 + 36);
    if (v0 == 0)
        return -1;
    v1 = *(int *)(v0 + 40);
    if (v1 == 0)
        return -1;
    v0 = *(unsigned char *)(v0 + 13);
    if (index >= v0)
        return -1;
    r = F29eece9b_FUN_002E5AD0((char *)(v1 + index * 32), a1, a2, 0, 1024);
    if (r >= 0) {
        p = &F29eece9b_D_00188660[r * 112];
        *(int *)(p + 136) = (int)a2;
        *(short *)(p + 126) = (short)index;
    }
    return r;
}

extern unsigned char D_L00_001DA860[];
extern char *D_L00_001D9F00[];
extern int func_002E5AD0(char *, int, char *, int, int);

int func_002E5DA0(int index, int value, char *object, int kind)
{
    char *header = D_L00_001D9F00[D_L00_001DA860[kind]];
    char *entries;
    int result;
    char *row;
    if (header == 0)
        return -1;
    entries = *(char **)(header + 0x28);
    if (entries == 0)
        return -1;
    if (index >= (unsigned char)header[0xD])
        return -1;
    result = func_002E5AD0(entries + index * 32, value, object, 0, 0x400);
    if (result >= 0) {
        row = D_00188660 + result * 112;
        *(char **)(row + 0x88) = object;
        *(short *)(row + 0x7E) = index;
    }
    return result;
}

INCLUDE_ASM("asm/game", func_002E5E58);

INCLUDE_ASM("asm/game", func_002E5EF0);

INCLUDE_ASM("asm/game", func_002E5F00);

INCLUDE_ASM("asm/game", func_002E5F38);

s32 func_002E5F40(s32 index, s32 callback) {
    u8 *p = D_00188660 + index * 0x70;
    *(s32 *)(p + 0x84) = callback;
    return 1;
}

void func_002E5F60(void) {
    s32 i=0;
    for (i=0;i<((CallbackGlobals *)D_00188660)->count;i++) {
            u8 *entry=(u8 *)(i*0x90+(s32)((CallbackGlobals *)D_00188660)->table);
            void (*callback)(u8 *)=*(void (**)(u8 *))(entry+4);
            if (callback) callback(entry);
    }
}

void func_002E5FE0(void) {
    typedef s32 quad __attribute__((mode(__TI__)));
    s32 count;
    u8 *p;
    func_00133230(); func_00132028(); func_00132AC8();
    while (func_00132028() != 0) {}
    p=D_00188660; count=3;
    do { *(volatile quad *)p = 0; count--; p += 16; } while (count >= 0);


    p=D_00188660;
    *(s32 *)(p+0x40)=0;
    for(count=0;count<52;count++) {
        *(s32 *)(p+count*0x70+0x70)=0;
        *(u8 *)(p+count*0x70+0x74)=0;
    }
}

void func_002E6088(int value, long target) {
    int *destination = (int *)(int)target;
    if (destination != 0)
        *destination = value;
}

void func_002E60A0(int arg0, long arg1) {
    unsigned char *p = (unsigned char *)(int)arg1;

    if (p != 0) {
        *(int *)p = arg0;
        if (arg0 != 0) {
            if (p[4] == 1) {
                p[4] = 2;
            }
        } else {
            *(int *)(p + 0x18) = 0;
            *(int *)(p + 0x1C) = 0;
            p[4] = 0;
        }
    }
}

void func_002E60E8(int arg0, long arg1) {
    int *p = (int *)(int)arg1;

    if (p != 0) {
        *p = arg0;
        if (arg0 == 0) {
            *(int *)((char *)p + 0x18) = 0;
            *(int *)((char *)p + 0x1C) = 0;
            *(unsigned char *)((char *)p + 4) = 0;
        }
    }
}

INCLUDE_ASM("asm/game", func_002E6118);

extern void *D_00214920[];
extern void func_0029F338(void *);

void func_002E6120(void)
{
    void **entry = D_00214920;
    int count = 19;
    do {
        count--;
        if (*entry != 0)
            func_0029F338(*entry);
        *entry++ = 0;
    } while (count >= 0);
}

INCLUDE_ASM("asm/game", func_002E6178);

INCLUDE_ASM("asm/game", func_002E64A0);

INCLUDE_ASM("asm/game", func_002E6A58);

INCLUDE_ASM("asm/game", func_002E6A60);

INCLUDE_ASM("asm/game", func_002E6B78);

INCLUDE_ASM("asm/game", func_002E6D88);
