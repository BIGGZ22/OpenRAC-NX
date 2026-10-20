#include "include_asm.h"

extern char D_00189E20_8[];

extern int func_L00_002D1DA0(int, int *, int *);

/* Frozen Batch A A14-struct-2field-20; acknowledged claim 3e0e0bb01b3da304ddd50c7212de48bebcbf96d3b2d117369779044d96f69b21. */
typedef unsigned int BatchA_A14_u32;

typedef struct { unsigned char pad[0x2494]; BatchA_A14_u32 a; BatchA_A14_u32 b; } BatchA_A14_S;

extern BatchA_A14_S D_00189E20;

typedef struct { short key; short val; } Entry;

extern Entry *D_0018C2B8;

/* Measured partial layouts; original type and game roles are unknown. */
typedef int Rac2Native_28c929cadd7aca24_s32;

typedef unsigned char Rac2Native_28c929cadd7aca24_u8;

typedef struct {
    Rac2Native_28c929cadd7aca24_u8 prefix[0xc40];
    Rac2Native_28c929cadd7aca24_s32 selected;
    Rac2Native_28c929cadd7aca24_s32 index;
    Rac2Native_28c929cadd7aca24_u8 gap[0x1648];
    Rac2Native_28c929cadd7aca24_u8 *object;
} Rac2Native_28c929cadd7aca24_NativeResidentView;

extern Rac2Native_28c929cadd7aca24_NativeResidentView D_00189E20_19;

extern char D_00189E20_17[];

typedef unsigned char u8;

extern u8 D_00189E20_1[];

typedef struct { u8 prefix[0x34]; unsigned short flags; } FlagPairObjectView;

typedef struct {
    u8 prefix[0x1860];
    FlagPairObjectView *secondary;
    u8 between[0xa2c];
    FlagPairObjectView *primary;
} FlagPairResidentView;

/* Guards three fields of a level state block, calls one helper with two stack outputs, then copies a shared float into offset 72 of two optional objects. */
void func_L00_002D1ED8(void)
{
    char *d = D_00189E20_8;
    int a;
    int b;
    int *o;

    if (*(unsigned char *)(d + 8884) != 0)
        return;
    if (*(int *)(d + 4916) != 2)
        return;
    if (func_L00_002D1DA0(*(unsigned char *)(*(int *)(d + 8848) + 67), &a, &b) == 0)
        return;
    o = (int *)*(int *)(d + 4880);
    if (o != 0)
        *(float *)((char *)o + 72) = *(float *)(d + 3104);
    o = (int *)*(int *)(d + 4884);
    if (o != 0)
        *(float *)((char *)o + 72) = *(float *)(d + 3104);
}

/* A14-struct-2field-20 freshly exact-verified under family reservation */
void func_L00_002D1F60(void) {
    D_00189E20.a = 0;
    D_00189E20.b = 0;
}

/* shared native body of family c68ad20a */
int func_L00_002D1F78(int key, int *out)
{
    Entry *e = D_0018C2B8;
    Entry *p;

    if (e == 0)
        return 0;
    if (e->key == -1)
        goto notfound;
    p = e;
    for (;;) {
        if (key == p->key) {
            *out = p->val;
            return 1;
        }
        p++;
        if (p->key == -1)
            goto notfound;
    }
notfound:
    *out = 0;
    return 0;
}

/* The resident block this reads its state from. */
extern char D_00189E20_2[];

extern void func_002A7470(void *, int, int, int);

/* Routes the object the state word names to the mode the helper reports: the helper's value
   and the caller's index are handed over with the caller's float as an int, otherwise the
   object's own flag picks the fixed mode 7. Same two gates as its sibling below. */
void func_L00_002D1FD8(int a0, int a1, float f12)
{
    char *base = D_00189E20_2;
    int out;

    if (*(int *)(base + 0x2494) == 0)
        return;
    if (*(int *)(base + 0x2498) == 0)
        return;
    if (func_L00_002D1F78(a0, &out) != 0) {
        func_002A7470(*(void **)(base + 0x2494), out, a1, (int)f12);
    } else if (*(unsigned char *)(*(char **)(base + 0x2494) + 0x43) != 0) {
        func_002A7470(*(void **)(base + 0x2494), 0, 0, 7);
    }
}

/* Publishes the resident clock into the object the state word names, once the state word and
   the second gate are both set and the helper accepts the object's key. */
void func_L00_002D2070(void)
{
    char *base = D_00189E20_2;
    int out;
    int object;

    if (*(int *)(base + 0x2494) == 0)
        return;
    if (*(int *)(base + 0x2498) == 0)
        return;
    if (func_L00_002D1F78(*(unsigned char *)(*(char **)(base + 0x2290) + 0x43), &out) == 0)
        return;
    object = *(int *)(base + 0x2494);
    *(float *)(object + 0x48) = *(float *)(base + 0xC20);
}

INCLUDE_ASM("asm/levels", func_L00_002D20D0);

INCLUDE_ASM("asm/levels", func_L00_002D2168);

INCLUDE_ASM("asm/levels", func_L00_002D23C0);

/* updates the resident selection and index after validating the selected table entry */
void func_L00_002D2470(Rac2Native_28c929cadd7aca24_s32 selected, Rac2Native_28c929cadd7aca24_s32 index) {
    if (selected >= 0) {
        Rac2Native_28c929cadd7aca24_u8 *object = D_00189E20_19.object;
        Rac2Native_28c929cadd7aca24_s32 offset = object[0x43] * 4;
        Rac2Native_28c929cadd7aca24_u8 *table = *(Rac2Native_28c929cadd7aca24_u8 **)(object + 0x24);
        Rac2Native_28c929cadd7aca24_u8 *entry = *(Rac2Native_28c929cadd7aca24_u8 **)(table + offset + 0x48);
        if (index < entry[0x10]) {
            D_00189E20_19.selected = selected;
            D_00189E20_19.index = index;
        }
    }
}

/* Marks no slot as running and parks the cursor past the end of the list. */
void func_L00_002D24B8(void) {
    char *b = D_00189E20_8;
    *(int *)(b + 0xC40) = -1;
    *(int *)(b + 0xC48) = 0;
}

/* The resident word published below. */
extern int D_0018AA68;

/* Publishes the argument into the resident word, with 0xFFFFFFFF becoming 5. */
void func_L00_002D24D0(int value)
{
    if (value == -1)
        value = 5;
    D_0018AA68 = value;
}

INCLUDE_ASM("asm/levels", func_L00_002D24E8);

INCLUDE_ASM("asm/levels", func_L00_002D2A10);

INCLUDE_ASM("asm/levels", func_L00_002D2E68);

/* shared native body of family ea34650e369e */
void func_L00_002D3068(void)
{
    char *b = D_00189E20_17;
    char *p;
    int i;

    p = *(char **)(b + 3096);
    *(unsigned short *)(p + 52) &= 0xFFFE;

    for (i = 0; i < 7; i++) {
        p = *(char **)(b + 4640 + i * 80);
        if (p != 0)
            *(unsigned short *)(p + 52) &= 0xFFFE;
        p = *(char **)(b + 4644 + i * 80);
        if (p != 0)
            *(unsigned short *)(p + 52) &= 0xFFFE;
        p = *(char **)(b + 4648 + i * 80);
        if (p != 0)
            *(unsigned short *)(p + 52) &= 0xFFFE;
    }

    if (1) {
        char *c = D_00189E20_17;
        p = *(char **)(c + 4892);
        if (p != 0)
            *(unsigned short *)(p + 52) &= 0xFFFE;
    }

    if (1) {
        char *c = D_00189E20_17;
        if (*(unsigned char *)(c + 8884) == 1) {
            c = D_00189E20_17;
            p = *(char **)(c + 6244);
            if (p != 0)
                *(unsigned short *)(p + 52) &= 0xFFFE;
        }
    }

    if (1) {
        char *c = D_00189E20_17;
        if (*(short *)(c + 9468) != 0 || *(int *)(c + 8860) == 31) {
            c = D_00189E20_17;
            p = *(char **)(c + 4880);
            if (p != 0)
                *(unsigned short *)(p + 52) |= 0x41;
            c = D_00189E20_17;
            p = *(char **)(c + 4884);
            if (p != 0)
                *(unsigned short *)(p + 52) |= 0x41;
            c = D_00189E20_17;
            p = *(char **)(c + 4892);
            if (p != 0)
                *(unsigned short *)(p + 52) |= 0x41;
        }
    }

    if (1) {
        char *c = D_00189E20_17;
        if ((*(unsigned char *)(c + 8894) != 0 && *(int *)(c + 4680) == 10) ||
            *(unsigned char *)(c + 8895) != 0) {
            char *d = D_00189E20_17;
            p = *(char **)(d + 4640);
            if (p != 0)
                *(unsigned short *)(p + 52) |= 0x41;
        }
    }
}

/* Set bit0 of the primary object's halfword flags and of the optional secondary object's flags */
void func_L00_002D31D0(void) {
    FlagPairResidentView *root = (FlagPairResidentView *)D_00189E20_1;
    FlagPairObjectView *primary = root->primary;
    FlagPairObjectView *secondary;
    primary->flags |= 1;
    secondary = root->secondary;
    if (secondary) secondary->flags |= 1;
}

/* Clear bit0 of the primary object's halfword flags and of the optional secondary object's flags */
void func_L00_002D3208(void) {
    FlagPairResidentView *root = (FlagPairResidentView *)D_00189E20_1;
    FlagPairObjectView *primary = root->primary;
    FlagPairObjectView *secondary;
    primary->flags &= ~1;
    secondary = root->secondary;
    if (secondary) secondary->flags &= ~1;
}

INCLUDE_ASM("asm/levels", func_L00_002D3240);

INCLUDE_ASM("asm/levels", func_L00_002D3248);

INCLUDE_ASM("asm/levels", func_L00_002D3290);
