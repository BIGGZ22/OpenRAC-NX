#include "include_asm.h"

extern int func_002A77E0(int mode);

extern float func_002A90F8(char *object, char *local);

extern void func_00282D30_7(char *local, int value, float scale);

extern float func_002A7878_1(float low, float high);

extern void func_002BF728(char *object, char *local, float amount, float base);

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_b1b523716b470b36_FamilyNative192Vector {
    float x;
    float y;
    float z;
    float w;
} Rac2Native_b1b523716b470b36_FamilyNative192Vector;

extern void func_00282CF0_11(Rac2Native_b1b523716b470b36_FamilyNative192Vector *out,
    const Rac2Native_b1b523716b470b36_FamilyNative192Vector *first, const Rac2Native_b1b523716b470b36_FamilyNative192Vector *second);

extern void func_00282CC0_23(Rac2Native_b1b523716b470b36_FamilyNative192Vector *out,
    const Rac2Native_b1b523716b470b36_FamilyNative192Vector *first, const Rac2Native_b1b523716b470b36_FamilyNative192Vector *second);

#ifndef RAC2_T_WORKER_F9E2CD219
#define RAC2_T_WORKER_F9E2CD219
typedef struct {
    unsigned char pad00[76];
    short field4C;
    unsigned char pad4E[2];
    int field50;
    float field54;
    float field58;
} Worker_F9e2cd219;
#endif

#ifndef RAC2_T_OWNER_F9E2CD219
#define RAC2_T_OWNER_F9E2CD219
typedef struct {
    unsigned char pad00[104];
    Worker_F9e2cd219 *child;
} Owner_F9e2cd219;
#endif

extern float func_002A7798(float value);

extern void func_002ACF68(void *out, void *in, void *tmp, int mode, float value);

extern void func_L00_00387BB0(void *owner, void *out);

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned char QwenRecovery_4ecc6b5a4034_u8;

typedef unsigned int QwenRecovery_4ecc6b5a4034_u32;

/* Minimum external views; neither declaration allocates original game storage. */
typedef struct {
    QwenRecovery_4ecc6b5a4034_u8 prefix[0x24];
    QwenRecovery_4ecc6b5a4034_u32 word;
    QwenRecovery_4ecc6b5a4034_u8 tail[8];
} QwenRecovery_4ecc6b5a4034_ExtraResetRecord164;

extern QwenRecovery_4ecc6b5a4034_ExtraResetRecord164 D_L00_002372C0[100];

extern QwenRecovery_4ecc6b5a4034_u32 D_L00_00238580[139];

extern void func_00282CF0_12(Rac2Native_b1b523716b470b36_FamilyNative192Vector *out,
    const Rac2Native_b1b523716b470b36_FamilyNative192Vector *first, const Rac2Native_b1b523716b470b36_FamilyNative192Vector *second);

extern void func_00282CC0_24(Rac2Native_b1b523716b470b36_FamilyNative192Vector *out,
    const Rac2Native_b1b523716b470b36_FamilyNative192Vector *first, const Rac2Native_b1b523716b470b36_FamilyNative192Vector *second);

extern void func_L00_0038A650(void *owner, void *out);

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_03c444112283bc5f_u32;

extern Rac2Native_03c444112283bc5f_u32 D_L00_00238DF0[139];

extern void func_L00_0038DB68(char *object);

extern void func_L00_0038D750(char *object);

extern float func_00283B98_5(float value, float step);

extern void func_L00_0038D650(char *object);

extern void func_00300190_3(char *target, char *source);

extern void func_00300248(char *target, char *source);

extern void func_L00_0038F020(char *object);

void func_L00_0037E898(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_0037F790);

INCLUDE_ASM("asm/levels", func_L00_0037F7E8);

INCLUDE_ASM("asm/levels", func_L00_0037FA90);

INCLUDE_ASM("asm/levels", func_L00_0037FE08);

INCLUDE_ASM("asm/levels", func_L00_0037FF98);

INCLUDE_ASM("asm/levels", func_L00_00380058);

void func_L00_00380A88(void)
{
}

void func_L00_00381F78(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003825C8);

INCLUDE_ASM("asm/levels", func_L00_003827C0);

INCLUDE_ASM("asm/levels", func_L00_00382910);

INCLUDE_ASM("asm/levels", func_L00_00382AE8);

INCLUDE_ASM("asm/levels", func_L00_00382BF0);

INCLUDE_ASM("asm/levels", func_L00_00382CE8);

INCLUDE_ASM("asm/levels", func_L00_00382F28);

INCLUDE_ASM("asm/levels", func_L00_00383B90);

INCLUDE_ASM("asm/levels", func_L00_00384070);

INCLUDE_ASM("asm/levels", func_L00_00384248);

INCLUDE_ASM("asm/levels", func_L00_00384678);

INCLUDE_ASM("asm/levels", func_L00_00384C20);

INCLUDE_ASM("asm/levels", func_L00_00384CA8);

INCLUDE_ASM("asm/levels", func_L00_00384D40);

INCLUDE_ASM("asm/levels", func_L00_00384E08);

INCLUDE_ASM("asm/levels", func_L00_00384F28);

INCLUDE_ASM("asm/levels", func_L00_00385040);

INCLUDE_ASM("asm/levels", func_L00_003851A0);

INCLUDE_ASM("asm/levels", func_L00_003852A0);

INCLUDE_ASM("asm/levels", func_L00_00385370);

INCLUDE_ASM("asm/levels", func_L00_00387370);

/* Family 6eb4f363e5305bed: seed body at func_L00_00387408. */
void func_L00_00387408(char *object)
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

/* Update the owner vector using the two bound vector helpers, refresh the three saved source coordinates, and conditionally set the measured owner byte to 7; original game role is unknown. */
void func_L00_00387518(void *object)
{
    Rac2Native_b1b523716b470b36_FamilyNative192Vector input;
    Rac2Native_b1b523716b470b36_FamilyNative192Vector output;
    char *base = (char *)object;
    char *state;
    char *source;
    char *kind;
    char *extra;

    input.w = 0.0f;
    state = *(char **)(base + 0x68);
    input.x = *(float *)(state + 0x74);
    input.y = *(float *)(state + 0x78);
    input.z = *(float *)(state + 0x7c);
    source = *(char **)(state + 0x70);
    func_00282CF0_11(&output,
        (const Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(source + 0x10), &input);
    func_00282CC0_23((Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(base + 0x10), &output,
        (const Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(base + 0x10));
    source = *(char **)(state + 0x70);
    *(float *)(state + 0x74) = *(float *)(source + 0x10);
    *(float *)(state + 0x78) = *(float *)(source + 0x14);
    *(float *)(state + 0x7c) = *(float *)(source + 0x18);
    kind = *(char **)(source + 0x24);
    if (*(short *)(kind + 0x46) == 0x12) {
        extra = *(char **)(source + 0x68);
        if (*(unsigned char *)(extra + 0x14) == 0) {
            *(unsigned char *)(base + 0x20) = 7;
        }
    }
}

/* Shared native body; original game role is unknown. */
void func_L00_00387AC8(Owner_F9e2cd219 *self)
{
    Worker_F9e2cd219 *child = self->child;

    func_002ACF68((char *)child + 48, (char *)child + 16, (char *)child + 32, 0,
            func_002A7798(child->field54));

    child->field54 = child->field54 + child->field58;
    func_L00_00387BB0(self, (char *)child + 48);

    if (child->field54 > 0.9f)
        child->field50 = 1;
    else
        child->field50 = 0;

    if (child->field54 >= 1.0f) {
        child->field54 = 1.0f;
        child->field4C = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_00387BB0);

INCLUDE_ASM("asm/levels", func_L00_00387CA8);

/* Clear the selected word in 100 records, reset 136 state words, and set the three trailing state words. */
void func_L00_00387EF0(void)
{
    int i;
    for (i = 99; i >= 0; --i)
        D_L00_002372C0[i].word = 0;
    D_L00_00238580[138] = 5;
    D_L00_00238580[137] = 0;
    D_L00_00238580[136] = 0;
    for (i = 0; i < 64; ++i) {
        D_L00_00238580[i] = 0;
        D_L00_00238580[i + 64] = 0;
    }
    for (i = 7; i >= 0; --i)
        D_L00_00238580[i + 128] = 0;
}

INCLUDE_ASM("asm/levels", func_L00_00387F98);

/* shared native body of family a04928c6 */
void func_L00_003883B8(char *p)
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

INCLUDE_ASM("asm/levels", func_L00_00388490);

INCLUDE_ASM("asm/levels", func_L00_00388790);

INCLUDE_ASM("asm/levels", func_L00_00389E18);

/* Family 6eb4f363e5305bed: seed body at func_L00_00387408. */
void func_L00_00389EB0(char *object)
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

/* Update the owner vector using the two bound vector helpers, refresh the three saved source coordinates, and conditionally set the measured owner byte to 7; original game role is unknown. */
void func_L00_00389FC0(void *object)
{
    Rac2Native_b1b523716b470b36_FamilyNative192Vector input;
    Rac2Native_b1b523716b470b36_FamilyNative192Vector output;
    char *base = (char *)object;
    char *state;
    char *source;
    char *kind;
    char *extra;

    input.w = 0.0f;
    state = *(char **)(base + 0x68);
    input.x = *(float *)(state + 0x74);
    input.y = *(float *)(state + 0x78);
    input.z = *(float *)(state + 0x7c);
    source = *(char **)(state + 0x70);
    func_00282CF0_12(&output,
        (const Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(source + 0x10), &input);
    func_00282CC0_24((Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(base + 0x10), &output,
        (const Rac2Native_b1b523716b470b36_FamilyNative192Vector *)(base + 0x10));
    source = *(char **)(state + 0x70);
    *(float *)(state + 0x74) = *(float *)(source + 0x10);
    *(float *)(state + 0x78) = *(float *)(source + 0x14);
    *(float *)(state + 0x7c) = *(float *)(source + 0x18);
    kind = *(char **)(source + 0x24);
    if (*(short *)(kind + 0x46) == 0x12) {
        extra = *(char **)(source + 0x68);
        if (*(unsigned char *)(extra + 0x14) == 0) {
            *(unsigned char *)(base + 0x20) = 7;
        }
    }
}

/* Shared native body; original game role is unknown. */
void func_L00_0038A568(Owner_F9e2cd219 *self)
{
    Worker_F9e2cd219 *child = self->child;

    func_002ACF68((char *)child + 48, (char *)child + 16, (char *)child + 32, 0,
            func_002A7798(child->field54));

    child->field54 = child->field54 + child->field58;
    func_L00_0038A650(self, (char *)child + 48);

    if (child->field54 > 0.9f)
        child->field50 = 1;
    else
        child->field50 = 0;

    if (child->field54 >= 1.0f) {
        child->field54 = 1.0f;
        child->field4C = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L00_0038A650);

INCLUDE_ASM("asm/levels", func_L00_0038A748);

/* Reset two 64-word banks, eight trailing words, and three state words. The original game role is unknown. */
void func_L00_0038A968(void)
{
    int i;
    D_L00_00238DF0[138] = 5;
    D_L00_00238DF0[137] = 0;
    D_L00_00238DF0[136] = 0;
    for (i = 0; i < 64; ++i) {
        D_L00_00238DF0[i] = 0;
        D_L00_00238DF0[i + 64] = 0;
    }
    for (i = 7; i >= 0; --i)
        D_L00_00238DF0[i + 128] = 0;
}

INCLUDE_ASM("asm/levels", func_L00_0038A9E0);

/* shared native body of family a04928c6 */
void func_L00_0038AA40(char *p)
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

INCLUDE_ASM("asm/levels", func_L00_0038AAD0);

INCLUDE_ASM("asm/levels", func_L00_0038ADD0);

INCLUDE_ASM("asm/levels", func_L00_0038B208);

INCLUDE_ASM("asm/levels", func_L00_0038B6F8);

INCLUDE_ASM("asm/levels", func_L00_0038B7F8);

INCLUDE_ASM("asm/levels", func_L00_0038B848);

INCLUDE_ASM("asm/levels", func_L00_0038B888);

INCLUDE_ASM("asm/levels", func_L00_0038BA80);

INCLUDE_ASM("asm/levels", func_L00_0038BD00);

INCLUDE_ASM("asm/levels", func_L00_0038BE70);

INCLUDE_ASM("asm/levels", func_L00_0038BF18);

INCLUDE_ASM("asm/levels", func_L00_0038D368);

/* Dispatches on a byte mode at object+32: mode 0 updates two float angles from a linked node and runs a follow-up call; mode 1 refreshes a 16-byte block and a node copy then calls a final update. */
void func_L00_0038D468(char *object)
{
    char *s0 = object;
    unsigned char mode = *(unsigned char *)(s0 + 32);
    char *s1 = *(char **)(s0 + 104);
    float result;

    switch (mode) {
    case 0:
        if (*(int *)(s1 + 20) != 0) {
            func_L00_0038DB68(s0);
        } else {
            func_L00_0038D750(s0);
        }
        result = func_00283B98_5(*(float *)(s0 + 240), 3.4906592220067977905273e-02f);
        *(float *)(s0 + 240) = result;
        result = func_00283B98_5(*(float *)(s0 + 244), 1.3089971244335174560547e-01f);
        *(float *)(s0 + 244) = result;
        func_L00_0038D650(s0);
        break;
    case 1:
        func_00300190_3((char *)(s0 + 16), (char *)(s0 + 16));
        func_00300248(s1, s1);
        func_L00_0038F020(s0);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L00_0038D530);

INCLUDE_ASM("asm/levels", func_L00_0038D650);

INCLUDE_ASM("asm/levels", func_L00_0038DE70);

INCLUDE_ASM("asm/levels", func_L00_0038E880);

INCLUDE_ASM("asm/levels", func_L00_0038F020);
