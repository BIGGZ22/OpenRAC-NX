#include "include_asm.h"

/* Resident (boot) bodies that are placed in every level program: their addresses differ
   from level to level, so they are named by their boot catalogue name. */
extern void func_00282978(unsigned short *);
extern void func_002829A8(unsigned short *);

/* same resident body, called here with three arguments */
extern void func_002A7750_3(unsigned char *, int, int);

extern void func_002A7470(unsigned char *, int, int, int);
extern int func_002A7820(int, int);
extern int func_002A77E0(int);
extern float func_00283B98(float, float);

/* Level 09 table of track pointers, indexed by a record byte. */
extern short *D_L09_001FB560[];

INCLUDE_ASM("asm/levels", func_L09_002F8BA0);

INCLUDE_ASM("asm/levels", func_L09_002F8FD8);

INCLUDE_ASM("asm/levels", func_L09_00326E88);

INCLUDE_ASM("asm/levels", func_L09_0032B3A0);

INCLUDE_ASM("asm/levels", func_L09_00352150);

INCLUDE_ASM("asm/levels", func_L09_0038B700);

INCLUDE_ASM("asm/levels", func_L09_0038BB18);

INCLUDE_ASM("asm/levels", func_L09_003910F8);

INCLUDE_ASM("asm/levels", func_L09_0039BA18);

INCLUDE_ASM("asm/levels", func_L09_003C0DF8);

INCLUDE_ASM("asm/levels", func_L09_003C8CD8);

INCLUDE_ASM("asm/levels", func_L09_003CBB18);

/* Two-state step of a scripted record: while the state byte is clear a random
   duration in 150..200 is drawn and the record is marked active; in the active
   state the stored duration becomes a signed angle (0.000290888245f is
   2*pi/21600) which the resident step body folds into the record's angle. */
void func_L09_003DDAA0(unsigned char *a0) {
    int v1;
    unsigned char *a2;
    if (a0[0x20] != 0)
        return;
    v1 = *(int *)(a0 + 0x24);
    a2 = *(unsigned char **)(v1 + 0x48);
    func_002A7470(a0, 0,
                  (int)((float)a2[0x10] * ((float)((int)a0 % 3) * 0.333f)), 1);
    func_002A7750_3(a0, 1, -1);
}

/* Same measured body as func_L09_003DDAA0, at its own place in the level. */
void func_L09_003DDB40(unsigned char *a0) {
    int v1;
    unsigned char *a2;
    if (a0[0x20] != 0)
        return;
    v1 = *(int *)(a0 + 0x24);
    a2 = *(unsigned char **)(v1 + 0x48);
    func_002A7470(a0, 0,
                  (int)((float)a2[0x10] * ((float)((int)a0 % 3) * 0.333f)), 1);
    func_002A7750_3(a0, 1, -1);
}

/* The record's state byte is a small switch, not an if/else chain: the retail
   compares it against 0 and 1 in turn and falls straight out for anything else.
   State 0 draws a random duration in 150..200 and marks the record active;
   state 1 converts the drawn duration to a signed angle and folds it into the
   record's angle through the resident step body. */
void func_L09_003E31C0(unsigned char *a0) {
    switch (a0[0x20]) {
    case 0:
        a0[0xBC] = func_002A7820(0x96, 0xC8);
        if (func_002A77E0(2) != 0) {
            a0[0x95] = 1;
        }
        a0[0x20] = 1;
        break;
    case 1:
        if (a0[0x95] != 0) {
            *(float *)(a0 + 0xF4) = func_00283B98(
                *(float *)(a0 + 0xF4), -((float)a0[0xBC] * 0.000290888245f));
        } else {
            *(float *)(a0 + 0xF4) = func_00283B98(
                *(float *)(a0 + 0xF4), (float)a0[0xBC] * 0.000290888245f);
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L09_003E3288);

INCLUDE_ASM("asm/levels", func_L09_003F02D0);

/* Advances the object's saturation counter while it is below 0xFFFF and runs
   the resident two-call teardown on the sub-object; the counter's guard is an
   unsigned `x < 0xFFFFu`, which cc1 prints as `0xFFFE < x`. */
void func_L09_003F0330(unsigned char *a0) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    if (*(unsigned short *)(a0 + 0x96) < 0xFFFFu) {
        *(unsigned short *)(a0 + 0x96) = *(unsigned short *)(a0 + 0x96) + 1;
    }
    func_00282978((unsigned short *)(s0 + 0x92));
    func_002829A8((unsigned short *)(s0 + 0x90));
}

INCLUDE_ASM("asm/levels", func_L09_003F0378);

INCLUDE_ASM("asm/levels", func_L09_003F04D0);

INCLUDE_ASM("asm/levels", func_L09_003F6820);

INCLUDE_ASM("asm/levels", func_L09_003F69A0);

INCLUDE_ASM("asm/levels", func_L09_003F6AB8);

INCLUDE_ASM("asm/levels", func_L09_003F6B28);

INCLUDE_ASM("asm/levels", func_L09_003FB4D0);

INCLUDE_ASM("asm/levels", func_L09_003FB650);

INCLUDE_ASM("asm/levels", func_L09_003FB788);

INCLUDE_ASM("asm/levels", func_L09_003FB860);

INCLUDE_ASM("asm/levels", func_L09_003FB908);

INCLUDE_ASM("asm/levels", func_L09_003FB9F8);

INCLUDE_ASM("asm/levels", func_L09_003FBF98);

INCLUDE_ASM("asm/levels", func_L09_003FC1E8);

INCLUDE_ASM("asm/levels", func_L09_003FCA58);

INCLUDE_ASM("asm/levels", func_L09_003FCF68);

INCLUDE_ASM("asm/levels", func_L09_003FD128);

INCLUDE_ASM("asm/levels", func_L09_003FD460);

INCLUDE_ASM("asm/levels", func_L09_003FD7D0);

INCLUDE_ASM("asm/levels", func_L09_003FDA38);

INCLUDE_ASM("asm/levels", func_L09_003FDC58);

INCLUDE_ASM("asm/levels", func_L09_003FDD80);

INCLUDE_ASM("asm/levels", func_L09_003FDED8);

INCLUDE_ASM("asm/levels", func_L09_003FDFF0);

INCLUDE_ASM("asm/levels", func_L09_003FE0C0);

INCLUDE_ASM("asm/levels", func_L09_003FE2D8);

INCLUDE_ASM("asm/levels", func_L09_003FE3B0);

INCLUDE_ASM("asm/levels", func_L09_003FE4D0);

/* Number of leading non-negative halfwords of the track the record's byte 0x21
   selects, counting the first negative one. The pointer has to walk itself
   (`v = *q; q++;`) inside a do/while: the indexed spelling rotates the two
   body instructions the other way and grows an induction-variable copy. */
int func_L09_003FE620(unsigned char *a0) {
    short *q = D_L09_001FB560[a0[0x21]];
    int i = 0;
    int v;
    if (q == 0)
        return 0;
    do {
        v = *q;
        q++;
        i++;
    } while (v >= 0);
    return i;
}

INCLUDE_ASM("asm/levels", func_L09_003FE670);

/* Same counter step as func_L09_003F0330, followed by the sub-object's own
   two-track restart: track 0x412 only when its live word at 0x154 is clear,
   then tracks 0x428 and 0x426 unconditionally. */
void func_L09_003FE918(unsigned char *a0) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    if (*(unsigned short *)(a0 + 0x96) < 0xFFFFu) {
        *(unsigned short *)(a0 + 0x96) = *(unsigned short *)(a0 + 0x96) + 1;
    }
    if (*(int *)(s0 + 0x154) == 0) {
        func_00282978((unsigned short *)(s0 + 0x412));
    }
    func_002829A8((unsigned short *)(s0 + 0x428));
    func_00282978((unsigned short *)(s0 + 0x426));
}

INCLUDE_ASM("asm/levels", func_L09_003FE978);
