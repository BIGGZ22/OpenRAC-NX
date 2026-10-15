#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_00282978(void *);

extern void func_00282CF0(float *, const float *, const float *);

extern void func_002AFDD8(unsigned char *, unsigned char *, unsigned char *, int);

extern float func_00283248(float, float);

extern float func_00282E20(char *);

extern float func_002A7878(void *, void *, float, float);

extern float func_002A7940(void);

extern float func_00283180(float);

extern float func_00283198(float);

extern void func_00282CC0(void *, void *, void *);

extern void func_002AEC48(void *, void *, int, float, float);

extern void func_002B0990(void *, void *, int, float, int);

extern void func_L02_002F1FB8(void *, void *, void *);

extern void func_00282F20(char *, char *, float);

extern void func_00282DB0(void *, void *);

extern char *D_L02_001FF1C0[];

extern char *D_L02_001B359C __attribute__((sda));

extern char *D_L02_001FC060[];

extern float func_00283C80_3(float, float);

extern float func_00283B98_12(float, float);

extern float func_00283BE0_7(float, float);

extern void func_002833F0_2(void *, void *);

extern void func_002E5DA0(int, int, void *, int);

extern void func_002A6C58(void *, int, int);

extern void func_00288CB0(int, int);

extern void func_L00_0037E5F0(float);

extern int func_L00_0032EF10(int, int, int);

extern void func_002A7750(void *, int, int);

extern void func_002B6D28(void);

extern void func_002A7470(void *, int, int, int);

extern void func_L00_003DD2B8(void);

extern void func_L00_002E6EF8(void *, void *);

extern void func_L02_003D28E0(void);

extern void func_L02_00333438(void);

extern void func_L01_00321178(void);

extern void func_002B4FF0(float);

extern struct S_001A63A8 D_001A63A8;

extern int D_L02_001A8FF0 __attribute__((sda));

extern void func_002B60B0(void *, float, float, float, float);

struct S_001A63A8 {
    char pad0[0x6C];
    short f6C;
    char pad1[4];
    short f72;
};

struct Sub {
    float f0;
    int f4;
    int p8;
    int p12;
    int p16;
    int f20;
};

struct Elem {
    char pad[104];
    struct Sub *sub;
    char pad2[140];
    float f248;
};

struct G {
    char pad[192];
    struct Elem *arr[22];
    int count;
};

struct Outer {
    char pad[104];
    struct G *g;
};

typedef int BatchA_A17_s32;

typedef short BatchA_A17_BatchAs16;

INCLUDE_ASM("asm/levels", func_L02_003C43F0);

/* Walks an inline array of element pointers; for each element whose sub-object flag at +4 is zero it rewrites a 270-degree angle to -90, scales it by pi/180, compares a helper result against 0.001 and routes it through one of two pi/2 helpers, toggles the flag, then calls a two-pointer helper with element+192 and element+240. */
void func_L02_003C46E0(struct Outer *p)
{
    struct G *g = p->g;
    int i;

    for (i = 0; i < g->count; i++) {
        struct Sub *s = g->arr[i]->sub;

        if (s->f4 == 0) {
            if (s->f0 == 270.0f)
                s->f0 = -90.0f;
            if (func_00283C80_3(g->arr[i]->f248, s->f0 * 1.7453294247388839721680e-02f)
                < 1.0000000474974513053894e-03f)
                g->arr[i]->f248 = func_00283B98_12(g->arr[i]->f248, 1.5707964897155761718750e+00f);
            else
                g->arr[i]->f248 = func_00283BE0_7(g->arr[i]->f248, 1.5707964897155761718750e+00f);
            s->f20 = !s->f20;
            func_002833F0_2((char *)g->arr[i] + 192, (char *)g->arr[i] + 240);
        }
    }
    if (g->count != 0)
        func_002E5DA0(1, 0, g->arr[0], 2198);
}

/* Clears the sub-object flags of three inline pointer arrays, turns a frame counter into a
   rotation, and pushes each element's 128-bit block one object forward. */
INCLUDE_ASM("asm/levels", func_L02_003C4848);

INCLUDE_ASM("asm/levels", func_L02_003C49A0);

INCLUDE_ASM("asm/levels", func_L02_003C4C80);

/* Reads three angles through a helper, converts them, and hands them to the placement call
   together with the object's own angle. */
INCLUDE_ASM("asm/levels", func_L02_003C4E18);

INCLUDE_ASM("asm/levels", func_L02_003C4ED8);

INCLUDE_ASM("asm/levels", func_L02_003C5098);

INCLUDE_ASM("asm/levels", func_L02_003C51F8);

INCLUDE_ASM("asm/levels", func_L02_003C5438);

INCLUDE_ASM("asm/levels", func_L02_003C9D78);

INCLUDE_ASM("asm/levels", func_L02_003C9ED0);

INCLUDE_ASM("asm/levels", func_L02_003CA0A8);

INCLUDE_ASM("asm/levels", func_L02_003CA280);

INCLUDE_ASM("asm/levels", func_L02_003CC7B0);

INCLUDE_ASM("asm/levels", func_L02_003CCCC8);

INCLUDE_ASM("asm/levels", func_L02_003CCEA0);

INCLUDE_ASM("asm/levels", func_L02_003CCFE8);

INCLUDE_ASM("asm/levels", func_L02_003CD1E8);

INCLUDE_ASM("asm/levels", func_L02_003CD298);

INCLUDE_ASM("asm/levels", func_L02_003CD4A8);

INCLUDE_ASM("asm/levels", func_L02_003CD758);

INCLUDE_ASM("asm/levels", func_L02_003CD8C0);

INCLUDE_ASM("asm/levels", func_L02_003CD970);

INCLUDE_ASM("asm/levels", func_L02_003CE4D0);

/* Runs the kind's entry hook: the first when the level flag is set, the second when the third
   argument is clear, the third with the object's own angle otherwise. */
void func_L02_003CE8B0(char *a0, int a1, int a2) {
    char *p = *(char **)(a0 + 0x68);

    if (D_L02_001A8FF0)
        func_L02_00333438();
    else if (a2 == 0)
        func_L01_00321178();
    else
        func_002B4FF0(*(float *)(p + 0x31C));
}

/* Clears the two angle words at +0x310 and +0x312 and counts up the +0x96 word, then runs the
   two placement helpers on the sub-object at +0x260. */
void func_L02_003CE908(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    char *q;
    unsigned short n;

    func_00282978(p + 0x310);
    func_00282978(p + 0x312);
    n = *(unsigned short *)(a0 + 0x96);
    if (n <= 0xFFFE)
        *(unsigned short *)(a0 + 0x96) = n + 1;
    q = p + 0x260;
    func_002AEC48(a0, q, 0, 0.03f, 0.3000000119209289550781250f);
    func_002B0990(a0, q, 0, 1.79999995231628417968750f, 1);
}

/* Near miss (see work/wave7-20261013/w04/drafts): the loop, both branch-likely tests and the
   duplicated `lh` delay-slot loads are retail's; two register copies are missing because this
   compiler gives the kind byte the other register. 116 bytes against 124. */
INCLUDE_ASM("asm/levels", func_L02_003CE9A8);

INCLUDE_ASM("asm/levels", func_L02_003CEA28);

INCLUDE_ASM("asm/levels", func_L02_003CEBB8);

/* Reads the object's basis, clamps its pitch and yaw, and stores them back. */
void func_L02_003CEC70(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    float v[4];
    float x;
    float y;

    func_00282CF0(v, (const float *)(p + 0x1F0), (const float *)(a0 + 0x10));
    func_002AFDD8((unsigned char *)a0, (unsigned char *)v, (unsigned char *)v, 0);
    x = func_00283248(v[0], v[1]);
    *(float *)(p + 0x2C0) = x;
    if (x > 0.7853982448577881f)
        *(float *)(p + 0x2C0) = 0.7853982448577881f;
    else if (x < -0.7853982448577881f)
        *(float *)(p + 0x2C0) = -0.7853982448577881f;
    y = -func_00283248(func_00282E20((char *)v), v[2]);
    *(float *)(p + 0x2C8) = y;
    if (y > 0.523598849773407f)
        *(float *)(p + 0x2C8) = 0.523598849773407f;
    else if (y < -0.523598849773407f)
        *(float *)(p + 0x2C8) = -0.523598849773407f;
}

INCLUDE_ASM("asm/levels", func_L02_003CED70);

INCLUDE_ASM("asm/levels", func_L02_003CEE40);

INCLUDE_ASM("asm/levels", func_L02_003CEFD8);

INCLUDE_ASM("asm/levels", func_L02_003CF1B8);

/* Positions A0 with four angles, then moves it kind six to kind two of length ten. */
void func_L02_003D1588(char *a0) {
    func_002B60B0(a0, 0.0055555556900799274f, 0.03166666626930237f,
                  0.006981317885220051f, 0.8377581834793091f);
    func_002A7750(a0, 6, 3);
    if (*(unsigned char *)(a0 + 0x43) != 2)
        func_002A7470(a0, 2, 0, 10);
}

/* Positions A0 with four angles, then moves it kind ten to kind six of length five. */
void func_L02_003D1610(char *a0) {
    func_002B60B0(a0, 0.0055555556900799274f, 0.06666667014360428f,
                  0.0034906589426100254f, 0.41887909173965454f);
    func_002A7750(a0, 10, 4);
    if (*(unsigned char *)(a0 + 0x43) != 6)
        func_002A7470(a0, 6, 0, 5);
}

/* Builds three quadwords on the stack from A1 and A0+0xD0, then normalises and rotates them. */
void func_L02_003D1820(char *a0, char *a1) {
    TI128 v[3];

    v[0] = *(TI128 *)a1;
    v[1] = *(TI128 *)(a0 + 0xD0);
    func_L02_002F1FB8((char *)&v[2], (char *)&v[0], a0 + 0x10);
    func_00282F20((char *)&v[2], (char *)&v[2], 1.0f);
    func_00282DB0((char *)&v[1], (char *)&v[2]);
}

INCLUDE_ASM("asm/levels", func_L02_003D1FA0);

/* func_L02_003D28E0 lives in the -g8 sibling: it reads five $gp-relative scalars. */

/* When the +0x3C counter of the object at +0x68 is clear, runs the reset, then the dispatch. */
void func_L02_003D29D0(char *a0) {
    char *s0 = a0;

    if (*(BatchA_A17_s32 *)(*(BatchA_A17_s32 *)(s0 + 0x68) + 0x3C) == 0)
        func_L00_003DD2B8();
    func_L00_002E6EF8((void *)func_L02_003D28E0, s0);
}

INCLUDE_ASM("asm/levels", func_L02_003D6BC0);

INCLUDE_ASM("asm/levels", func_L02_003D6DF8);

INCLUDE_ASM("asm/levels", func_L02_003D6EE0);

INCLUDE_ASM("asm/levels", func_L02_003D7040);

INCLUDE_ASM("asm/levels", func_L02_003D7140);

int func_L02_003D7300(char *a0) {
    return *(int *)(*(char **)(a0 + 0x68) + 0xBC);
}

void func_L02_003D7310(char *a0) {
    BatchA_A17_s32 p = (BatchA_A17_s32)*(BatchA_A17_s32 *)(a0 + 0x68);
    *(BatchA_A17_s32 *)((char *)p + 0xBC) = 1;
}

void func_L02_003D7320(char *a0) {
    *(int *)(*(char **)(a0 + 0x68) + 0xBC) = 0;
}

/* True when any of the three words at +0x10, +0x14, +0x18 of the object at +0x68 is set. */
int func_L02_003D77B8(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    int r = 0;

    if (*(unsigned char *)(a0 + 0x20) == 2) {
        if (*(int *)(p + 0x10) == 0 || *(int *)(p + 0x14) == 0 || *(int *)(p + 0x18) == 0)
            r = 1;
    }
    return r;
}

/* True when any of the three words at +0x10, +0x14, +0x18 of the object at +0x68 is set. */
int func_L02_003D77F8(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    int r = 0;

    if (*(int *)(p + 0x10) || *(int *)(p + 0x14) || *(int *)(p + 0x18))
        r = 1;
    return r;
}

/* Of kind two, reports whether the helper found nothing. */
int func_L02_003D7830(char *a0) {
    int r = 0;

    if (*(unsigned char *)(a0 + 0x20) == 2)
        r = func_L02_003D77F8(a0) == 0;
    return r;
}

/* True when the first free of the three words at +0x10, +0x14, +0x18 of the object at +0x68
   exists; A1 goes there and its kind byte is set to two. */
void func_L02_003D7868(char *a0, char *a1) {
    char *p = *(char **)(a0 + 0x68);

    if (*(int *)(p + 0x10) == 0)
        *(int *)(p + 0x10) = (int)a1;
    else if (*(int *)(p + 0x14) == 0)
        *(int *)(p + 0x14) = (int)a1;
    else if (*(int *)(p + 0x18) == 0)
        *(int *)(p + 0x18) = (int)a1;
    *(char *)(a1 + 0x20) = 2;
}

/* Resets the +0x48 float to one, then moves A0 from kind two to kind three. */
void func_L02_003D78A8(char *a0) {
    *(float *)(a0 + 0x48) = 1.0f;
    if (*(unsigned char *)(a0 + 0x43) != 0)
        func_002A7470(a0, 0, 0, 0);
    func_002A7750(a0, 3, -1);
}

/* Near miss (see work/wave7-20261013/w04/drafts): retail sequence and store order reproduced,
   same register-permutation wall as func_L02_003DBEC8. 152 bytes against 148. */
INCLUDE_ASM("asm/levels", func_L02_003D7B88);

/* Near miss (see work/wave7-20261013/w04/drafts): the whole body compiles to the retail
   instruction sequence; only the register permutation differs (the parameter copy and p land in
   a2/a3 the other way round). 124 bytes against 120. */
INCLUDE_ASM("asm/levels", func_L02_003DBEC8);

void func_L02_003DED08(void) {
    func_L00_0037E5F0(10000.0f);
}

INCLUDE_ASM("asm/levels", func_L02_003E1A40);

INCLUDE_ASM("asm/levels", func_L02_003E2150);

INCLUDE_ASM("asm/levels", func_L02_003EB8F8);

/* Copies the byte at +0x40 of the object held at +0x68 over its byte at +0x25. */
void func_L02_003F0628(char *a0) {
    char *p = (char *)*(BatchA_A17_s32 *)(a0 + 0x68);

    *(unsigned char *)(p + 0x25) = *(unsigned char *)(p + 0x40);
}

/* Boundary suspect: the retail body at this row is an epilogue (`addiu $sp,$sp,0x50; nop;
   jr $ra; nop`), so the catalogue row starts inside the function before it. Not a function. */
INCLUDE_ASM("asm/levels", func_L02_003F0788);

/* Forwards the first argument with a null second and a third of three. */
void func_L02_003FC248(void *a0) {
    func_002A6C58(a0, 0, 3);
}

/* The two entry scalers live in the -g8 sibling: retail reads the table through $gp. */

INCLUDE_ASM("asm/levels", func_L02_003FE768);

/* Allocates sixteen of sixty with no owner; on success attaches A0 as kind two. */
void func_L02_003FF698(void *a0) {
    if (func_L00_0032EF10(16, 60, 0))
        func_002A7750(a0, 2, -1);
}

INCLUDE_ASM("asm/levels", func_L02_003FFC48);

/* Below 6000 in the low half-word, while the high one reads three. */
void func_L02_004006D8(void) {
    if (D_001A63A8.f72 == 3 && D_001A63A8.f6C < 0x1770)
        func_002B6D28();
}

/* True when the byte at +0x20 is two or more. */
int func_L02_00400B00(unsigned char *a0) {
    return *(unsigned char *)(a0 + 0x20) >= 2;
}

INCLUDE_ASM("asm/levels", func_L02_00400D08);

INCLUDE_ASM("asm/levels", func_L02_004012A0);

/* func_L02_004013C0 lives in the -g8 sibling: it reads a $gp-relative scalar. */

INCLUDE_ASM("asm/levels", func_L02_00401480);

INCLUDE_ASM("asm/levels", func_L02_00401530);

INCLUDE_ASM("asm/levels", func_L02_00401ED8);

INCLUDE_ASM("asm/levels", func_L02_00402300);

/* func_L02_00402440 lives in the -g8 sibling: it reads four $gp-relative scalars. */

int func_L02_004024A0(char *a0) {
    return *(short *)(*(char **)(a0 + 0x68) + 0x30);
}

/* A17-ptr-store-16 freshly exact-verified under family reservation */
void func_L02_004024B0(char *a0) {
    BatchA_A17_s32 p = (BatchA_A17_s32)*(BatchA_A17_s32 *)(a0 + 0x68);
    *(BatchA_A17_BatchAs16 *)((char *)p + 0x30) = 1;
}

void func_L02_004024C0(char *a0) {
    *(short *)(*(char **)(a0 + 0x68) + 0x30) = 0;
}

INCLUDE_ASM("asm/levels", func_L02_00404410);

void func_L02_00404778(void) {
    func_00288CB0(2, 1);
}
