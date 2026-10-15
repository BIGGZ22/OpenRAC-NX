#include "include_asm.h"

extern int D_L11_001B359C __attribute__((sda));

extern void func_32C700();

INCLUDE_ASM("asm/levels", func_L11_00432628);

INCLUDE_ASM("asm/levels", func_L11_00434580);

INCLUDE_ASM("asm/levels", func_L11_004349F8);

typedef int TI128 __attribute__((mode(TI)));

extern void func_L11_00434AF8(char *p, void *q);

extern void func_L11_00434D00(char *p, void *q);

/* Hands two reads of the state's quadword to the two measured stage helpers. */
void func_L11_00434AA8(char *p)
{
    char *state = *(char **)(p + 0x68);
    TI128 v = *(TI128 *)state;

    func_L11_00434AF8(p, &v);
    v = *(TI128 *)state;
    func_L11_00434D00(p, &v);
}

INCLUDE_ASM("asm/levels", func_L11_00434AF8);

INCLUDE_ASM("asm/levels", func_L11_00434D00);

INCLUDE_ASM("asm/levels", func_L11_00434F48);

INCLUDE_ASM("asm/levels", func_L11_00435228);

INCLUDE_ASM("asm/levels", func_L11_00435600);

INCLUDE_ASM("asm/levels", func_L11_00436350);

INCLUDE_ASM("asm/levels", func_L11_00436768);

/* Re-arms the measured stage guard, then clears it. */
INCLUDE_ASM("asm/levels", func_L11_00436850);

INCLUDE_ASM("asm/levels", func_L11_00436910);

extern void func_L11_00441978(unsigned char *object, int class_id);

extern void func_0032C700(char *, int, int, int, int, int, int, int, float, float, int, int, int, int);

extern void func_002E5DA0(int, int, void *, int);

extern void func_002A7470(void *, int, int, int);

extern void func_002A05A8(char *);

extern void func_002A7750(void *, int, int);

/* Retires the measured stage-46 record: clears the guard, drops the effect and re-tags the object. */
void func_L11_00436A78(char *obj)
{
    char *state = *(char **)(obj + 0x68);

    if (*(short *)(state + 0x282) == 1) {
        func_L11_00441978(*(unsigned char **)(obj + 0xB8), 0xD37);
        *(int *)(state + 0x270) = 0;
    } else {
        *(int *)(state + 0x270) = 0;
    }
    *(short *)(state + 0x282) = 0;
    if (*(unsigned char *)(obj + 0x31)) {
        func_0032C700(obj + 0x10, 0, 0, 0x1E, 5, 0, 0x32, 1, 0.3f, 0.0f, 0, 0, 0, 0);
        func_002E5DA0(0, 0, obj, 0xDF7);
    }
    func_002A7470(obj, 0, 0, 3);
    func_002A05A8(obj);
    func_002A7750(obj, 1, 0);
    *(unsigned char *)(obj + 0x31) = 0;
    *(int *)(obj + 0x98) = 0;
    *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) & 0xEFFF) | 0x47;
}

INCLUDE_ASM("asm/levels", func_L11_00436B78);

INCLUDE_ASM("asm/levels", func_L11_00436D08);

INCLUDE_ASM("asm/levels", func_L11_00436F80);

INCLUDE_ASM("asm/levels", func_L11_004375B8);

extern unsigned char D_L11_00438880[];

extern void func_802F8AD0(void *a0, void *a1);
extern void func_80338398(void *a0, void *a1, int a2, int a3, float f12);
extern void func_L11_004395C8(char *p);
extern void func_L11_00437EA8(char *p);
extern void func_L11_00437FD0(char *p);
extern void func_L11_00438460(char *p);
extern void func_L11_004386B8(char *p);

/* Runs the ten rotors' update, one frame each, and restarts the entry timer. */
void func_L11_00437CF0(char *p)
{
    char *q = *(char **)(p + 0x68);
    int i;

    func_802F8AD0((void *)D_L11_00438880, p);

    for (i = 12; i >= 0; i--)
        ;

    if (*(unsigned char *)(p + 0x20) >= 2) {
        func_L11_004395C8(p);
        func_L11_00437EA8(p);
        func_L11_00437FD0(p);
        func_L11_00438460(p);
        func_L11_004386B8(p);
    }
    if (*(unsigned short *)(p + 0x96) <= 0xFFFE) {
        (*(unsigned short *)(p + 0x96))++;
    }
    func_80338398(p, q + 0x80, 1, 1, 2.5f);
}

/* Walks the ten measured indices, clearing each record selected through 0x330. */
void func_L11_00437DC0(char *p, int a1, int a2, int a3)
{
    int i;

    for (i = 0; i < 10; i++) {
        int offset = i * 4;
        int index = *(int *)(*(char **)(p + 0x68) + offset + 0x330);
        char *record = (char *)(D_L11_001B359C + (index << 8));

        if (*(unsigned char *)(record + 0x43) != a1)
            func_32C700(record, a1, a2, a3);
    }
}

/* Invalidates the measured slot at 0x43 of the 256-byte record indexed by 0x358. */
void func_L11_00437E68(char *p, int a1)
{
    int record = (*(int *)(*(char **)(p + 0x68) + 0x358) << 8) + D_L11_001B359C;

    if (*(unsigned char *)(record + 0x43) != a1)
        func_32C700((char *)record, a1);
}

INCLUDE_ASM("asm/levels", func_L11_00437EA8);

INCLUDE_ASM("asm/levels", func_L11_00437FD0);

extern void func_002833F0(char *, char *);

extern void func_00282D30(char *, char *, float);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(char *, char *, char *);

INCLUDE_ASM("asm/levels", func_L11_00438248);

extern void func_00282F20_2(float *p, float x);

extern void func_00282DD8(char *a, char *b, char *c);

extern void func_002AB6D8(char *a, char *b);

extern void func_002AAA00(char *a, char *b, int c, float d, float e, float f, float g);

/* Builds the measured three-axis basis of the object and hands it to the three slots. */
void func_L11_00438320(char *obj, float a, float b, float c)
{
    char *state = *(char **)(obj + 0x68);
    float m[4][4];
    float out[4];

    func_00282F20_2(m[0], 1.0f);
    func_00282C88((TI128 *)m[2]);
    m[2][2] = 1.0f;
    func_00282DD8((char *)m[1], (char *)m[0], (char *)m[2]);
    func_00282F20((char *)m[1], (char *)m[1], 1.0f);
    func_00282DD8((char *)m[2], (char *)m[1], (char *)m[0]);
    func_00282F20((char *)m[2], (char *)m[2], 1.0f);
    func_002AB6D8((char *)m[0], (char *)out);
    func_002AAA00(obj + 0xF0, state + 0x2A0, 0, out[0], a, b, c);
    func_002AAA00(obj + 0xF4, state + 0x2A4, 0, out[1], a, b, c);
    func_002AAA00(obj + 0xF8, state + 0x2A8, 0, out[2], a, b, c);
}

INCLUDE_ASM("asm/levels", func_L11_00438460);

/* func_L11_004386B8: attempt kept in drafts (532 bytes, expected 452). */
INCLUDE_ASM("asm/levels", func_L11_004386B8);

INCLUDE_ASM("asm/levels", func_L11_00438880);

/* Clears the two 16-bit flags at 0x374/0x376 of the object held at 0x68. */
void func_L11_00438CD8(char *p)
{
    char *q = *(char **)(p + 0x68);
    *(short *)(q + 0x374) = 0;
    *(short *)(q + 0x376) = 0;
}

INCLUDE_ASM("asm/levels", func_L11_00438CE8);

INCLUDE_ASM("asm/levels", func_L11_00438F18);

INCLUDE_ASM("asm/levels", func_L11_004390D0);

INCLUDE_ASM("asm/levels", func_L11_004393D8);

/* func_L11_004395C8: attempt kept in drafts (112 bytes, expected 116). */
/* func_L11_004395C8 is defined in L11_00432628-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L11_00439640);

INCLUDE_ASM("asm/levels", func_L11_00439950);

extern int func_373978(char *obj, int a);

extern void func_3739C0(int a);

extern char D_00188660[];

#define REC_L11_359C(o, t) ((*(int *)(*(char **)((o) + 0x68) + 0x358) << 8) + (int)(t))

/* Releases the object's measured slot and clears the six flags of its record. */
void func_L11_00439BF0(char *obj)
{
    char *state;
    char *records;

    func_002A7750(obj, 1, 0);
    state = *(char **)(obj + 0x68);
    *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) & 0xEFFF) | 0x47;
    *(unsigned char *)(obj + 0x31) = 0;
    *(int *)(obj + 0x98) = 0;
    if (func_373978(obj, *(int *)(state + 0x38C)) != 0) {
        if (*(int *)(state + 0x38C) != -1) {
            char *entry = D_00188660 + *(int *)(state + 0x38C) * 0x70;

            if (*(char **)(entry + 0x88) == obj && *(unsigned char *)(entry + 0x74) != 0)
                func_3739C0(*(int *)(state + 0x38C));
        }
        *(int *)(state + 0x38C) = -1;
    }
    records = (char *)D_L11_001B359C;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x2;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x4;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x40;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) &= 0xEFFF;
    *(unsigned short *)(REC_L11_359C(obj, records) + 0x34) |= 0x1;
    *(unsigned char *)(REC_L11_359C(obj, records) + 0x31) = 0;
    *(int *)(REC_L11_359C(obj, (char *)D_L11_001B359C) + 0x98) = 0;
}


/* Picks the measured 3/4 code from the counter at 0x364 of the object at 0x68. */
int func_L11_00439D60(char *p)
{
        if (*(int *)(*(char **)(p + 0x68) + 0x364))
        return 3;
    return 4;
}

INCLUDE_ASM("asm/levels", func_L11_00439DD0);

INCLUDE_ASM("asm/levels", func_L11_0043A3F8);

extern void func_32E480(char *p, char *q, int a, float b, float c, float d, int e, int f, int g);

/* Bumps the 16-bit counter at 0x96 while it is under the measured ceiling, then re-arms. */
void func_L11_0043A620(char *p)
{
    unsigned short count = *(unsigned short *)(p + 0x96);

    if (count <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = count + 1;
    func_32E480(p, p + 0x10, 0x4000000, 1.2f, 1.0f, 1.0f, 0, 1, 0);
}

INCLUDE_ASM("asm/levels", func_L11_0043A680);

/* True when the byte at 0x20 is above the measured small-id range. */
int func_L11_0043A800(char *p)
{
    return *(unsigned char *)(p + 0x20) >= 4;
}

INCLUDE_ASM("asm/levels", func_L11_0043A810);

INCLUDE_ASM("asm/levels", func_L11_0043A9D8);

INCLUDE_ASM("asm/levels", func_L11_0043AB28);

INCLUDE_ASM("asm/levels", func_L11_0043ACB0);

/* Maps the measured selector onto its small index. */
INCLUDE_ASM("asm/levels", func_L11_0043E090);

/* Maps the measured selector onto its large index. */
INCLUDE_ASM("asm/levels", func_L11_0043E108);
