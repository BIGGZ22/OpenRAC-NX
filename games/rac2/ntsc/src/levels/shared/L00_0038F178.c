#include "include_asm.h"

extern int func_002A77E0(int mode);

extern float func_002A90F8(char *object, char *local);

extern void func_00282D30_7(char *local, int value, float scale);

extern float func_002A7878_1(float low, float high);

extern void func_002BF728(char *object, char *local, float amount, float base);

extern char *func_0029EFD0_1(int);

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

extern void func_002FDBD8(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern void func_002FDBD8_1(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

typedef unsigned char u8;

typedef float f32;

/* Prototype775 is a research label. Field names below describe only the
 * independently observed byte23 and binary32 word2C operations. */
typedef struct {
    u8 pad00[0x23];
    u8 field23;
    u8 pad24[8];
    f32 field2C;
} NativeUpdate775View;

extern void func_0029F338(NativeUpdate775View *object);

INCLUDE_ASM("asm/levels", func_L00_0038F178);

/* Family 6eb4f363e5305bed: seed body at func_L00_00387408. */
void func_L00_0038F3D8(char *object)
{
    char local[16];
    float base;
    int value = *(int *)(object + 104);

    if (func_002A77E0(5))
        return;
    base = func_002A90F8(object + 16, local);
    func_00282D30_7(local, value, 0.25f);
    func_002BF728(object + 16, local, func_002A7878_1(0.05f, 0.1f) * 210000.0f, base);
}

INCLUDE_ASM("asm/levels", func_L00_0038F5F0);

INCLUDE_ASM("asm/levels", func_L00_0038F8B8);

INCLUDE_ASM("asm/levels", func_L00_0038FC48);

INCLUDE_ASM("asm/levels", func_L00_0038FE10);

/* Calls a callee with the constant 167 and, when it returns a non-zero object, stores the argument pointer at +4 of the object held at +104, clears the words at +0, +8 and +12 of that object, sets byte +48 to 255, the halfword at +50 to 126 and bytes +49 and +32 to 0, then returns the object. */
char *func_L00_00390938(char *p)
{
    char *o = func_0029EFD0_1(167);
    int *v;

    if (o != 0) {
        v = *(int **)(o + 104);
        v[1] = (int)p;
        v[0] = 0;
        v[2] = 0;
        v[3] = 0;
        *(unsigned char *)(o + 48) = 255;
        *(short *)(o + 50) = 126;
        o[49] = 0;
        o[32] = 0;
    }
    return o;
}

INCLUDE_ASM("asm/levels", func_L00_00390E10);

INCLUDE_ASM("asm/levels", func_L00_00390F28);

INCLUDE_ASM("asm/levels", func_L00_00390FF8);

INCLUDE_ASM("asm/levels", func_L00_00391310);

INCLUDE_ASM("asm/levels", func_L00_00391398);

INCLUDE_ASM("asm/levels", func_L00_003914E0);

INCLUDE_ASM("asm/levels", func_L00_003918C8);

INCLUDE_ASM("asm/levels", func_L00_00392530);

INCLUDE_ASM("asm/levels", func_L00_00395240);

INCLUDE_ASM("asm/levels", func_L00_003952E0);

INCLUDE_ASM("asm/levels", func_L00_00395428);

INCLUDE_ASM("asm/levels", func_L00_003959C0);

INCLUDE_ASM("asm/levels", func_L00_00395A88);

INCLUDE_ASM("asm/levels", func_L00_00396078);

INCLUDE_ASM("asm/levels", func_L00_00396288);

INCLUDE_ASM("asm/levels", func_L00_003973E8);

INCLUDE_ASM("asm/levels", func_L00_00397E60);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_00398118(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00398150);

INCLUDE_ASM("asm/levels", func_L00_00398810);

INCLUDE_ASM("asm/levels", func_L00_0039B160);

INCLUDE_ASM("asm/levels", func_L00_0039B288);

INCLUDE_ASM("asm/levels", func_L00_0039CBA0);

INCLUDE_ASM("asm/levels", func_L00_0039D110);

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_0039D340(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_1(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_0039D378);

INCLUDE_ASM("asm/levels", func_L00_0039D598);

INCLUDE_ASM("asm/levels", func_L00_0039E158);

/* multiply binary32 field2C by exact1.025f then subtract3 modulo256 from byte23; call original pointer helper if updated byte below4; no original semantic field names recovered */
void func_L00_0039E5B0(NativeUpdate775View *object)
{
    object->field2C *= 1.025f;
    object->field23 -= 3;
    if (object->field23 < 4)
        func_0029F338(object);
}

INCLUDE_ASM("asm/levels", func_L00_0039E600);

INCLUDE_ASM("asm/levels", func_L00_0039E7A8);

INCLUDE_ASM("asm/levels", func_L00_0039EBA8);

INCLUDE_ASM("asm/levels", func_L00_0039F300);

INCLUDE_ASM("asm/levels", func_L00_003A09F8);

INCLUDE_ASM("asm/levels", func_L00_003A0C00);

INCLUDE_ASM("asm/levels", func_L00_003A0DA0);
