#include "include_asm.h"

typedef unsigned char u8;

extern void func_0032B890(int, int, int);

extern void func_0031E6A0(void *, int, int);
extern int func_0031E770(int);
extern void func_0031E3C0(void *, int, int, int);
extern int func_002F1BE0(void *);
extern float func_0031E808(float, float);
extern int func_002F3158(float);

extern void func_002F1F70(void *, void *, void *);
extern void func_002F20A0(void *, void *, void *);
extern void func_002F2270(void *, void *, float);
extern float func_0031E8D0(void);

INCLUDE_ASM("asm/levels", func_L02_00410450);

INCLUDE_ASM("asm/levels", func_L02_00410D00);

INCLUDE_ASM("asm/levels", func_L02_00410E58);

INCLUDE_ASM("asm/levels", func_L02_00410F68);

/* Plays one of nine resident messages, chosen by the sub-object's state word. */
INCLUDE_ASM("asm/levels", func_L02_004114A0);

INCLUDE_ASM("asm/levels", func_L02_00411530);

/* The float clamp at 00411530 stores through $gp; its best attempt is in
   the draft notes (w40) and it stays an INCLUDE_ASM for now. */

/* True while the record's state byte still says 2. */
int func_L02_004127F8(u8 *a0) {
    return *(u8 *)(a0 + 0x20) == 2;
}

/* Copies the sub-object's byte 0x40 down to its byte 0x25. */
void func_L02_00414D48(u8 *a0) {
    u8 *v1 = *(u8 **)(a0 + 0x68);
    *(u8 *)(v1 + 0x25) = *(u8 *)(v1 + 0x40);
}

INCLUDE_ASM("asm/levels", func_L02_004164A8);

INCLUDE_ASM("asm/levels", func_L02_00416720);

/* True when the record is in state 1 and its sub-object holds a cleared origin. */
INCLUDE_ASM("asm/levels", func_L02_00416800);

INCLUDE_ASM("asm/levels", func_L02_00421860);

INCLUDE_ASM("asm/levels", func_L02_00421DA0);

INCLUDE_ASM("asm/levels", func_L02_00422270);

/* Rebuilds the sub-object's frame from the record's own transform. */
void func_L02_004224C8(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);
    u8 *s0 = a0 + 0x10;

    func_002F1F70(s1, s0, s1 + 0x40);
    func_002F20A0(s1, s1, s0);
    func_002F2270(s1, s1, 0.25f);
    *(float *)(s1 + 0x30) = func_0031E8D0();
}

/* func_L02_00422538 is defined in L02_00410450-g8.c (this unit has no $gp access for it). */

extern void func_L02_00422720(u8 *);
extern void func_L02_00422BB8(u8 *);
extern void func_L02_00422D68(u8 *);

/* One setup pass, then the five body passes of the resident effect. */
void func_L02_00422638(u8 *a0) {
    int i;

    func_L02_00422720(a0);
    i = 4;
    func_L02_00422BB8(a0);
    do {
        func_L02_00422D68(a0);
    } while (--i >= 0);
}

INCLUDE_ASM("asm/levels", func_L02_00422720);

INCLUDE_ASM("asm/levels", func_L02_00422BB8);

INCLUDE_ASM("asm/levels", func_L02_00422D68);

INCLUDE_ASM("asm/levels", func_L02_004230B0);

/* Restarts the record's resident effect once it has settled, picking the
   variant by the resident query and the record's own byte 0x43. */
void func_L02_0042ED48(u8 *a0) {
    u8 *s0 = a0;

    if (*(u8 *)(s0 + 0x20) != 0)
        return;
    func_0031E6A0(s0, 1, -1);
    if (func_0031E770(2) != 0) {
        if (*(u8 *)(s0 + 0x43) != 0)
            func_0031E3C0(s0, 0, 0, 0xA);
    } else {
        if (*(u8 *)(s0 + 0x43) != 1)
            func_0031E3C0(s0, 1, 0, 0xA);
    }
}

INCLUDE_ASM("asm/levels", func_L02_0042EDD0);

INCLUDE_ASM("asm/levels", func_L02_0043F4C8);

/* Picks the evasive state the object should take while the resident timer runs. */
void func_L02_0043FA98(u8 *a0) {
    u8 *s0 = a0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    int m;

    if (func_002F1BE0(s1 + 0x48) == 0)
        return;
    if (*(int *)(s1 + 0x44) != 0 || func_0031E808(0.0f, 1.0f) < 0.8f) {
        if (*(u8 *)(s0 + 0x42) != 1)
            m = 1;
        else
            m = 2;
        if (*(u8 *)(s0 + 0x43) != m)
            func_0031E3C0(s0, m, 0, 0x1E);
    } else {
        if (*(u8 *)(s0 + 0x43) != 0)
            func_0031E3C0(s0, 0, 0, 0x1E);
    }
    *(short *)(s1 + 0x48) = func_002F3158(func_0031E808(135.0f, 225.0f));
}

INCLUDE_ASM("asm/levels", func_L02_004547D0);

INCLUDE_ASM("asm/levels", func_L02_00457068);

INCLUDE_ASM("asm/levels", func_L02_0045B280);

INCLUDE_ASM("asm/levels", func_L02_004692D0);

INCLUDE_ASM("asm/levels", func_L02_00469748);
