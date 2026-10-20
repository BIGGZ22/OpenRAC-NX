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

/* Level object-update walk: one pass over the object list the game exposes at
 * D_L00_001B1EA8, with a per-class dispatch. */
typedef unsigned char L00_0037F7E8_u8;
typedef unsigned short L00_0037F7E8_u16;
typedef unsigned int L00_0037F7E8_u32;

extern void func_00284E18(int, int, int);

extern int func_002B49E0(int, int, int, int, int);

/* The empty method this call reaches sits in a different copy in each level
 * (the catalogue splits the identical bodies), so the callee takes a ref_ name:
 * its value is read from the retail bytes at every place. */
extern unsigned int ref_L00_0036F480_00(void);

extern void func_002F5220(void);

extern void func_002E4AC8();

extern L00_0037F7E8_u8 *func_002A08A0(void);

extern L00_0037F7E8_u8 *D_L00_001B1EA8 __attribute__((sda));

extern void func_002A05A8(char *);

extern void func_002A0F48(void *);

extern void func_L00_002A6DD0(void);

extern void func_00272368(void);

extern void func_002FF450(void);

extern void func_002B9390(void);

extern void func_002F52B8(void);

extern L00_0037F7E8_u32 D_001A7424 __attribute__((sda));

/* Measured partial layout; original type and game roles are unknown. */
typedef struct L00_0037F7E8_Object {
    L00_0037F7E8_u8 pad00[0x20];
    signed char field20;
    L00_0037F7E8_u8 pad21[7];
    struct L00_0037F7E8_Object *next;
    L00_0037F7E8_u8 pad2C[8];
    L00_0037F7E8_u16 field34;
    L00_0037F7E8_u8 pad36[0x2e];
    void (*field64)(struct L00_0037F7E8_Object *);
    L00_0037F7E8_u8 pad68[0x42];
    short fieldAA;
} L00_0037F7E8_Object;

/* The level's own data: each place resolves its own value from the retail
 * bytes. The two adjacent flags stay one array so both sites keep the retail's
 * in-place absolute load, which the sda attribute selects. */
extern int ref_L00_001A8F00_00 __attribute__((sda));

extern L00_0037F7E8_u8 ref_L00_001B267C_00[] __attribute__((sda));

void func_L00_0037F7E8(void)
{
    L00_0037F7E8_u8 *head;
    L00_0037F7E8_Object *object;
    short id;

    func_00284E18(0, 0, 0);
    if (ref_L00_001A8F00_00 == 0 && (D_001A7424 & 1)) {
        if (func_002B49E0(4, 1, 1, 0, 0) == 0)
            return;
    }
    if (ref_L00_0036F480_00() != 0)
        return;
    func_002F5220();
    func_002E4AC8();
    D_L00_001B1EA8 = head = func_002A08A0();
    object = (L00_0037F7E8_Object *)head;
    if (object != 0) {
        while (object != 0) {
            if (object->field20 >= 0) {
                id = object->fieldAA;
                if (id == 0xDC1) {
                    object->field64(object);
                    if ((object->field34 & 4) == 0)
                        func_002A0F48(object);
                } else if (id == 0xCB2 || id == 0xB02 || id == 0x259
                           || id == 0xA5C || id == 0x1192) {
                    func_002A05A8((char *)object);
                    func_002A0F48(object);
                }
            }
            object = object->next;
        }
    }
    if (ref_L00_001B267C_00[0])
        func_L00_002A6DD0();
    if (ref_L00_001B267C_00[1])
        func_00272368();
    func_002FF450();
    func_002B9390();
    func_002F52B8();
}

INCLUDE_ASM("asm/levels", func_L00_0037FA90);

typedef struct {
    char *entry[64][4];
    int count;
    int best;
} L00_0037FE08_Pool;

extern L00_0037FE08_Pool D_L00_00231540;

extern char D_00189EA0[];

extern float func_00282EB0(char *origin, char *point);

/* Keep the pool entry nearest to the fixed origin: the count is re-read every
 * pass, so the loop bound stays live in memory. */
void func_L00_0037FE08(void)
{
    float best;
    int i;

    if (D_L00_00231540.count != 0) {
        best = 100000000.0f;
        for (i = 0; i < D_L00_00231540.count; i++) {
            float distance = func_00282EB0(D_00189EA0, D_L00_00231540.entry[i][0] + 0x10);

            if (distance < best) {
                D_L00_00231540.best = i;
                best = distance;
            }
        }
    }
}

INCLUDE_ASM("asm/levels", func_L00_0037FF98);

extern int func_00303E20(int object, char *name, int first, int second);

/* 16-slot object table searched for the first entry the name resolver accepts.
 * The `sda` attribute (SYMBOL_REF_FLAG) suppresses gcc's HIGH/LO_SUM split: its
 * `lui`/`addiu` pair is one RTL insn, so the address stays in the single
 * callee-saved register that retail uses instead of taking an extra one. */
extern int *D_L00_001B26B8[] __attribute__((sda));

int func_L00_00380058(char *name)
{
    int i = 0;
    int **slot = D_L00_001B26B8;

    do {
        int object = *slot;
        if (object == 0)
            break;
        if (func_00303E20(object, name, 0, 0))
            return 1;
        i++;
        slot++;
    } while (i < 16);
    return 0;
}

void func_L00_00380A88(void)
{
}

void func_L00_00381F78(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003825C8);

extern void ref_L00_003827C0_00(void);

void func_L00_003827C0(void)
{
    ref_L00_003827C0_00();
}

INCLUDE_ASM("asm/levels", func_L00_00382910);

INCLUDE_ASM("asm/levels", func_L00_00382AE8);

INCLUDE_ASM("asm/levels", func_L00_00382BF0);

INCLUDE_ASM("asm/levels", func_L00_00382CE8);

INCLUDE_ASM("asm/levels", func_L00_00382F28);

INCLUDE_ASM("asm/levels", func_L00_00383B90);

INCLUDE_ASM("asm/levels", func_L00_00384070);

INCLUDE_ASM("asm/levels", func_L00_00384248);

INCLUDE_ASM("asm/levels", func_L00_00384678);

/* func_L00_00384C20 is matched in L00_00384C20.c (built with -fno-strength-reduce). */

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

extern void ref_L00_00387F98_00(void *arg);

void func_L00_00387F98(void)
{
    ref_L00_00387F98_00(D_L00_00238580);
}

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

extern void ref_L00_0038A9E0_00(void *arg);

void func_L00_0038A9E0(void)
{
    ref_L00_0038A9E0_00(D_L00_00238DF0);
}

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

extern void *D_L00_001BF218;

extern void *ref_L00_0038B848_00(void *arg);

int func_L00_0038B7F8(void)
{
    int s0 = 0;
    unsigned char *r = 0;
    if (D_L00_001BF218) {
        r = (unsigned char *)ref_L00_0038B848_00(D_L00_001BF218);
    }
    if (r) {
        s0 = (r[9] == 1);
    }
    return s0;
}

extern void *D_L00_001BF218;

extern void *ref_L00_0038B848_00(void *arg);

int func_L00_0038B848(void)
{
    unsigned short *r = (unsigned short *)ref_L00_0038B848_00(D_L00_001BF218);
    if (r) {
        if (r[15] & 2) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0038B888);

INCLUDE_ASM("asm/levels", func_L00_0038BA80);

extern unsigned char D_001A7B2A __attribute__((sda));

/* Measured partial layouts; the original types and game roles are unknown. */
typedef struct L00_0038BD00_Kind {
    unsigned char pad00[0x46];
    short type;
} L00_0038BD00_Kind;

typedef struct L00_0038BD00_Node {
    unsigned char pad00[0x20];
    unsigned char mode;
    unsigned char pad21[3];
    L00_0038BD00_Kind *kind;
    struct L00_0038BD00_Node *next;
} L00_0038BD00_Node;

typedef struct {
    unsigned char pad[0x2290];
    int value;
} D_00189E20_Struct;

extern D_00189E20_Struct D_00189E20;

extern float func_00282E48(char *object, char *other);

extern int func_003022A0(char *object, int value, float first, float second);

extern int func_L00_0031F2B0(char *object);

extern int func_L00_003D1B50(char *value);

/* Walk the level's node list from D_L00_001B1EA8: each node's own measure is
 * compared with the limit (2.7, or 10.0 when D_001A7B2A is set); a node of kind
 * 0x13 is then handed to func_003022A0, any other node to its own handler. A mode
 * of 0xFE or 0xFD, or a null next pointer, ends the walk. */
void func_L00_0038BD00(char *object)
{
    L00_0038BD00_Node *s0;
    float limit;
    float step;
    float value;
    int mode;
    int result;
    L00_0038BD00_Kind *kind;

    limit = 2.7f;
    if (D_001A7B2A != 0)
        limit = 10.0f;
    s0 = (L00_0038BD00_Node *)D_L00_001B1EA8;
    if (s0 == 0)
        return;
    step = 0.01745329424738884f;
    while (1) {
        mode = s0->mode;
        if (mode == 0xFE)
            return;
        if (mode == 0xFD)
            return;
        if (s0 != 0 && (kind = s0->kind) != 0 && kind->type == 0x13) {
            value = func_00282E48((char *)s0 + 0x10, object + 0x10);
            if (value < limit) {
                value = func_002A7878_1(-30.0f, 30.0f);
                result = func_003022A0((char *)s0, D_00189E20.value, value * step, 0.0f);
            } else {
                s0 = s0->next;
                goto next;
            }
        } else {
            if (func_L00_0031F2B0((char *)s0) == 0) {
                s0 = s0->next;
                goto next;
            }
            value = func_00282E48((char *)s0 + 0x10, object + 0x10);
            if (value < limit) {
                result = func_L00_003D1B50((char *)s0);
            } else {
                s0 = s0->next;
                goto next;
            }
        }
        if (result != 0)
            return;
        s0 = s0->next;
next:
        /* The walk's end test: the goto above reaches it with the next pointer
         * already loaded, which is the shape retail's code has. */
        if (s0 == 0)
            return;
    }
}

extern float D_00189EB8;

extern float func_00283248(float first, float second);

extern float func_00283C80(float value, float angle);

extern float func_00282DF0(char *object);

extern void func_00282F20(char *object, char *other, float value);

/* Fold an angle into the measured 1.8325959 ceiling, ease the remaining gap and
 * hand the result to the setter. */
void func_L00_0038BE70(char *object)
{
    float limit = 1.83259594440460205078125f;
    float base = func_00283248(*(float *)object, *(float *)(object + 4));
    float scaled = func_00283C80(D_00189EB8, base);
    float angle;

    if (limit < scaled)
        scaled = limit;
    angle = func_00282DF0(object);
    func_00282F20(object, object,
        angle * ((limit - scaled) * 0.1637022197246551513671875f + 0.699999988079071044921875f));
}

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
