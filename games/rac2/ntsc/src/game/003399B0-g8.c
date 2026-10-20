#include "include_asm.h"

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern void func_003361E0(void *, void *, void *);
extern void func_003369A8(void *, void *, void *);
extern void func_00336A30(void *, int);
extern int *func_00335E20(void *);
extern void func_00336390(void *, void *, int);
extern int func_00288FB8(int);
extern void func_00336A38(void *, int);
extern int func_00339F28(void *, int);
extern char D_001ADC10[], D_001ADC18[], D_001ADC20[], D_001ADC28[], D_001ADC30[];
extern char D_001ADC38[], D_001ADC40[], D_001ADC50[];
extern char D_001ADCA8[], D_001ADCB8[], D_001ADCC0[], D_001ADCC8[], D_001ADCD0[];
extern char *D_001A8CFC[] __attribute__((sda));
extern char D_001ADCA0[8] __attribute__((sda));

/* Binds the screen to its owner: allocates the 16-byte position block, initialises the four
   76-byte and nine 72-byte widgets by name ("0".."3", "title", "missions", ...), sets their
   colours, fonts and labels, marks the "  Exit" label with glyph 0x12 and lays them out.
   The block's four zero stores are written 1, 2, 3, 0 for retail's 0, 4, 8, C order. */
void func_00339AD0(char *obj, void *arg)
{
    *(void **)obj = arg;
    *(int *)(obj + 0x400) = 1;
    if (arg) {
        int *p = func_00336DA8(16, func_00336E40(arg));
        *(int **)(obj + 4) = p;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p[0] = 0;
    }
    (*(float **)(obj + 4))[0] = 256.0f;
    (*(float **)(obj + 4))[1] = 198.0f;
    func_003361E0(obj + 0x8, D_001ADC10, arg);
    func_003361E0(obj + 0x54, D_001ADC18, arg);
    func_003361E0(obj + 0xA0, D_001ADC20, arg);
    func_003361E0(obj + 0xEC, D_001ADC28, arg);
    func_003369A8(obj + 0x138, D_001ADC30, arg);
    func_003369A8(obj + 0x180, D_001ADCA8, arg);
    func_003369A8(obj + 0x1C8, D_001ADCB8, arg);
    func_003369A8(obj + 0x210, D_001ADCC0, arg);
    func_003369A8(obj + 0x258, D_001ADCC8, arg);
    func_003369A8(obj + 0x2A0, D_001ADCD0, arg);
    func_003369A8(obj + 0x2E8, D_001ADC38, arg);
    func_003369A8(obj + 0x330, D_001ADC40, arg);
    func_003369A8(obj + 0x378, D_001ADC50, arg);
    func_00336A30(obj + 0x180, 0);
    func_00336A30(obj + 0x1C8, 0);
    func_00336A30(obj + 0x210, 0);
    func_00336A30(obj + 0x258, 0);
    func_00336A30(obj + 0x2A0, 0);
    func_00336A30(obj + 0x2E8, 0);
    func_00336A30(obj + 0x330, 0);
    func_00336A30(obj + 0x378, 2);
    *func_00335E20(obj + 0x8) = 0x60442D00;
    *func_00335E20(obj + 0x54) = 0x60241700;
    *func_00335E20(obj + 0xA0) = 0x55F0C070;
    *func_00335E20(obj + 0xEC) = 0x55F0C070;
    *func_00335E20(obj + 0x138) = 0x80F0F0F0;
    *func_00335E20(obj + 0x180) = 0x80F0F0F0;
    *func_00335E20(obj + 0x1C8) = 0x80F0F0F0;
    *func_00335E20(obj + 0x210) = 0x80F0F0F0;
    *func_00335E20(obj + 0x258) = 0x80F0F0F0;
    *func_00335E20(obj + 0x2A0) = 0x80F0F0F0;
    *func_00335E20(obj + 0x2E8) = 0x80F0F0F0;
    *func_00335E20(obj + 0x330) = 0x80F0F0F0;
    *func_00335E20(obj + 0x378) = 0x80F0F0F0;
    func_00336390(obj + 0x8, D_001A8CFC[0] + 0x8710, 0x5F);
    func_00336390(obj + 0x54, D_001A8CFC[0] + 0x8710, 0x60);
    func_00336390(obj + 0xA0, D_001A8CFC[0] + 0x8710, 0x61);
    func_00336390(obj + 0xEC, D_001A8CFC[0] + 0x8710, 0x62);
    D_001ADCA0[0] = 0x12;
    func_00336A38(obj + 0x138, func_00288FB8(0x2D65));
    func_00336A38(obj + 0x180, func_00288FB8(0x2D5D));
    func_00336A38(obj + 0x1C8, func_00288FB8(0x3293));
    func_00336A38(obj + 0x210, func_00288FB8(0x2D69));
    func_00336A38(obj + 0x258, func_00288FB8(0x2D6A));
    func_00336A38(obj + 0x2A0, func_00288FB8(0x2D6B));
    func_00336A38(obj + 0x2E8, func_00288FB8(0x2D4F));
    func_00336A38(obj + 0x330, (int)(obj + 0x3C0));
    func_00339F28(obj, 0);
}

/* The X bias lives in small data, so this file is built with -G8. */
extern float ref_L00_0033A688_00 __attribute__((sda));
extern float D_001ADD0C;
extern void func_00335E38(char *, float, float, float, float);

/* Offsets the record position by the two resident biases and hands it to the
   shared spawn helper with no rotation. */
void func_0033A688(char *a0)
{
    if (*(int *)(a0 + 0x20C) != 0) {
        float *position = *(float **)(a0 + 0x1FC);

        func_00335E38(a0 + 0x170, position[0] + ref_L00_0033A688_00,
                      position[1] + D_001ADD0C, 0.0f, 0.0f);
    }
}

/* The table of four resident ints is reached through $gp and the resident
   pointer through the small-data unit, so this body needs -G8. */
extern char *D_001A8CFC[] __attribute__((sda));
extern char D_001ADC88[];
extern int D_001ADD30 __attribute__((sda));
extern void func_00335E68(char *, int);

void func_0033AA00(char *obj, int *arr, char *third)
{
    char *p = obj + 4;
    int *q = (int *)&D_001ADD30;
    int i;
    int n;
    int *out;
    int off;

    for (i = 3; i >= 0; i--) {
        if (*arr != 0) {
            func_003361E0(p, D_001ADC88, *(int *)(obj + 0x200));
            *(int *)func_00335E20(p) = *q;
            func_00336390(p, D_001A8CFC[0] + 0x8710, *arr);
        } else {
            func_00336390(p, D_001A8CFC[0] + 0x8710, 0);
            func_00335E68(p, 0);
        }
        p += 76;
        q += 1;
        arr += 1;
    }
    *(int *)(obj + 0x1F8) = 0;
    *(int *)obj = (int)third;
    n = 0;
    if (*(int *)(third + 4) != -1) {
        /* The named -1 makes the compiler keep the loop's comparison constant
           in the preheader, where the retail body has it. */
        int minus = -1;
        off = 0;
        out = (int *)(obj + 0x1B8);
        do {
            *out++ = 0;
            off += 0x18;
            *(int *)(obj + 0x1F8) = *(int *)(obj + 0x1F8) + 1;
            n++;
            if (*(int *)(off + (int)*(int *)obj + 4) == minus)
                break;
        } while (n < 16);
    }
}

extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00335F88(char *, float, float, float, float);
extern int *func_00335E20(void *);
extern void func_00336828(char *);
extern void func_00336920(char *, int, int);
extern int func_00288FB8(int);
extern void func_00336A78(char *);
extern float D_001ADD20 __attribute__((sda));
extern float D_001ADD24;
extern int D_001ADD10 __attribute__((sda));
extern int D_001ADD14;
extern float D_001ADD18 __attribute__((sda));
extern float D_001ADD28 __attribute__((sda));
extern float D_001ADD2C;

typedef struct { int first, second, color; float angle; short id; short pad; int text; } Entry_0033A6D8;
typedef struct { char pad[8]; short n; void (*fn)(char *); } Vtable_0033A6D8;
#define ENTRY_0033A6D8 (*(Entry_0033A6D8 **)self)[i]

/* Draws the selection list: two fixed sprites, then one sprite per entry placed around the stored
   position (the radius at +0x204 scales the cosine and the sine of the entry's angle), and for the
   selected entry the cursor sprite and its label. A mode (the ints from +0x1B8) of 1 gives the
   sprite the tint 0x80808080 instead of the entry's own colour; a mode of 2 clears the flag given
   to func_00335E68 and keeps the cursor from being moved and labelled.
   ENTRY_0033A6D8 is entry i of the array that starts at self + 0. `one` holds the 1 of the mode
   test in a register of its own: written as a bare 1 in the comparison, the constant is not
   hoisted out of the loop. The pointer to the modes is assigned where it is first used, and the
   cursor sprite (self + 4) reaches its virtual call through a variable: these decide which
   induction variable and which register the compiler picks. */
void func_0033A6D8(char *self)
{
    if (*(int *)(self + 0x20C) != 0) {
        char *rec0 = self + 0x50;
        char *rec1 = self + 0x9C;
        float *position = *(float **)(self + 0x1FC);
        int i;

        func_00335E38(rec0, position[0], position[1], 0.0f, 0.0f);
        func_00335F88(rec0, D_001ADD20, D_001ADD24, 0.0f, 0.0f);
        (*(Vtable_0033A6D8 **)(self + 0x80))->fn(rec0 + (*(Vtable_0033A6D8 **)(self + 0x80))->n);
        position = *(float **)(self + 0x1FC);
        func_00335E38(rec1, position[0], position[1], 0.0f, 0.0f);
        func_00335F88(rec1, D_001ADD20, D_001ADD24, 0.0f, 0.0f);
        (*(Vtable_0033A6D8 **)(self + 0xCC))->fn(rec1 + (*(Vtable_0033A6D8 **)(self + 0xCC))->n);
        for (i = 0; i < *(int *)(self + 0x1F8); i++) {
            char *rec = self + 0x134;
            int *mode;
            int one = 1;
            float cs, sn;

            cs = *(float *)(self + 0x204) * func_00283180(ENTRY_0033A6D8.angle);
            sn = *(float *)(self + 0x204) * func_00283198(ENTRY_0033A6D8.angle);
            func_00336920(rec, ENTRY_0033A6D8.first, ENTRY_0033A6D8.second);
            position = *(float **)(self + 0x1FC);
            func_00335E38(rec, position[0] + (float)D_001ADD10 + cs,
                          position[1] + (float)D_001ADD14 + D_001ADD18 * sn, 0.0f, 0.0f);
            mode = (int *)(self + 0x1B8) + i;
            *func_00335E20(rec) = (*mode == one) ? 0x80808080 : ENTRY_0033A6D8.color;
            func_00335E68(rec, *mode != 2);
            func_00336828(rec);
            if (i == *(int *)(self + 0x208)) {
                char *cursor;

                func_00335F88(self + 4, D_001ADD20, D_001ADD24, 0.0f, 0.0f);
                if (*mode != 2) {
                    position = *(float **)(self + 0x1FC);
                    func_00335E38(self + 4, position[0] + D_001ADD28 + cs,
                                  position[1] + D_001ADD2C + D_001ADD18 * sn, 0.0f, 0.0f);
                    {
                        int text = ENTRY_0033A6D8.text;

                        if (text != 0) {
                            func_00336A38(self + 0x170, text);
                        } else {
                            text = func_00288FB8(ENTRY_0033A6D8.id);
                            func_00336A38(self + 0x170, text);
                        }
                        func_00336A78(self + 0x170);
                    }
                }
                cursor = self + 4;
                (*(Vtable_0033A6D8 **)(self + 0x34))->fn(cursor + (*(Vtable_0033A6D8 **)(self + 0x34))->n);
            }
        }
    }
}
#undef ENTRY_0033A6D8

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern void func_003361E0(void *, void *, void *);
extern void func_003369A8(void *, void *, void *);
extern int *func_00335E20(void *);
extern void func_00336390(void *, void *, int);
extern void func_00115484(void *, int, int);
extern char D_001ADC80[], D_001ADC88[];
extern int D_001ADD40[2] __attribute__((sda));

/* Constructor of a gadget made of five sprite slots (76 bytes each from +0xC, switched on by the
   five shorts of LIST, which are also kept at +0x188) and three 72-byte sprites at +0x198, +0x1E0
   and +0x228, each with an 8-byte offset beside it at +0x278, +0x280 and +0x288.  The blank name
   is D_001ADC88; the slot loop reaches it as D_001ADC80 + 8 and the three sprites through the
   spent list pointer, which keeps the compiler from hoisting it across the loop. */
void func_0033ABD8(char *object, char *owner, short *list)
{
    char *record, *rec198, *rec1E0, *rec228, *vel278, *vel280, *vel288;
    short *flags, *p;
    int *color;
    int i;

    *(char **)(object + 8) = owner;
    if (owner) {
        int *allocation = func_00336DA8(16, func_00336E40(owner));
        *(int **)object = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    vel278 = object + 0x278;
    rec198 = object + 0x198;
    vel288 = object + 0x288;
    rec228 = object + 0x228;
    flags = (short *)(object + 0x188);
    rec1E0 = object + 0x1E0;
    color = D_001ADD40;
    record = object + 0xC;
    vel280 = object + 0x280;
    p = list;
    for (i = 4; i >= 0; i--) {
        if (*p != 0) {
            func_003361E0(record, D_001ADC80 + 8, owner);
            *func_00335E20(record) = *color;
            func_00336390(record, D_001A8CFC[0] + 0x8710, *p);
        }
        *flags = *p;
        color++;
        record += 0x4C;
        p++;
        flags++;
    }
    p = (short *)D_001ADC88;
    func_003369A8(rec198, p, owner);
    func_00115484(vel278, 0, 8);
    func_00335E38(rec198, 0.0f, 0.0f, 0.0f, 0.0f);
    *func_00335E20(rec198) = 0x80F0F0F0;
    func_003369A8(rec228, p, owner);
    func_00115484(vel288, 0, 8);
    func_00335E38(rec228, 0.0f, 0.0f, 0.0f, 0.0f);
    *func_00335E20(rec228) = 0x80F0F0F0;
    func_003369A8(rec1E0, p, owner);
    func_00115484(vel280, 0, 8);
    func_00335E38(rec1E0, 0.0f, 0.0f, 0.0f, 0.0f);
    *func_00335E20(rec1E0) = 0x80F0F0F0;
    *(float *)(object + 0x274) = 0.0f;
    *(int *)(object + 0x290) = 1;
    *(int *)(object + 0x294) = 1;
    *(int *)(object + 4) = 0;
    *(int *)(object + 0x29C) = 1;
}
