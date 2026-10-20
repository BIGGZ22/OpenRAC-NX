#include "include_asm.h"

/* Resident (boot) bodies that are placed in every level program: their addresses differ
   from level to level, so they are named by their boot catalogue name. */
extern int func_00282978(unsigned short *);
extern void func_002829A8(unsigned short *);

/* same resident body, called here with three arguments */
extern void func_002A7750_3(unsigned char *, int, int);

extern void func_002A7470(unsigned char *, int, int, int);
extern int func_002A7820(int, int);
extern int func_002A77E0(int);
extern float func_00283B98(float, float);
extern void func_002AEE70(unsigned char *, unsigned char *, unsigned char *, float, float, float);
extern void func_002AEF28(unsigned char *, unsigned char *, unsigned char *, float, float, float);
extern float func_002AA3A8(unsigned char *, unsigned char *, float, float, float, float);

/* Shared level-runtime bodies this file reaches: the eight-argument record
   starter and the two-argument record stepper. */
extern void func_L00_00323448();
extern void func_L00_00323958(unsigned char *, unsigned char *);
/* the sixteen-float record starter, called here for its live-record flag */
extern int func_L00_00341820(char *, char *, char *, char *, int, float);

extern unsigned int D_L09_001FE6C0[];

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

/* Two-state gate on the record's byte 0x20. In state 0, as long as the live
   word the sub-object opens with is not -1, the record is stamped 0xFF, the
   shared eight-argument starter is run on the sub-object's track block and the
   record flips to state 1. In state 1 the sub-object's first float is scaled by
   2*pi/21600 (a per-frame 1/100 s turn rate) and folded into the record's own
   angle before its track block is stepped again. */
void func_L09_003C8CD8(unsigned char *a0) {
    unsigned char *s0 = a0;
    unsigned char *s1 = *(unsigned char **)(s0 + 0x68);
    switch (s0[0x20]) {
    case 0:
        if (*(long long *)s1 == -1) {
            break;
        }
        {
            unsigned char stamp = 0xFF;
            *(unsigned char *)(s0 + 0x30) = stamp;
        }
        func_L00_00323448(s0, s1 + 0x30, 0x64, *(int *)(s1 + 4), *(int *)s1,
                          -1, 7, 0);
        s1[0xC5A] = 1;
        s0[0x20] = 1;
        break;
    case 1:
        *(float *)(s0 + 0xF0) = func_00283B98(
            *(float *)(s0 + 0xF0), *(float *)(s1 + 8) * 0.000290888245f);
        func_L00_00323958(s0, s1 + 0x30);
        break;
    }
}

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

/* Arms the sub-object: the halfword 10 at 0x24, the angle 10.0f, the word
   at 0xA4 copied to 0x8C, the byte 3, the scale 1.0f and the live byte 0x2F,
   then the resident four-argument starter on the record. */
void func_L09_003F02D0(unsigned char *a0) {
    unsigned char *state = *(unsigned char **)(a0 + 0x68);

    *(float *)(state + 0x20) = 10.0f;
    *(short *)(state + 0x24) = 10;
    *(unsigned char *)(state + 0x28) = 3;
    *(float *)(state + 0x30) = 1.0f;
    *(unsigned char *)(state + 0x2F) = 1;
    *(int *)(state + 0x8C) = *(int *)(state + 0xA4);
    func_002A7750_3(a0, 0, 1);
}


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

/* func_L09_003F6AB8 lives in the -g8.c brother: it loads a global through $gp. */

INCLUDE_ASM("asm/levels", func_L09_003F6B28);

/* Two-state step of the record-slot table: state 0 clears all sixteen slots
   and stamps the record's byte 0x30; state 1 walks them -- a slot whose live
   object has gone negative is released, a slot in state 4 is handed to
   func_L09_003FB860, any other live slot hands its object to
   func_L09_003FB9F8 and moves to state 4. When at least one slot was handed
   over, the level's teardown runs on the record. */
INCLUDE_ASM("asm/levels", func_L09_003FB4D0);


/* Starts a record slot: the first pass returns 1 when the id is already live
   in either half of a slot, the second takes the first free slot, fills its
   eight parallel tables and returns 1; 0 when every slot is taken. */
INCLUDE_ASM("asm/levels", func_L09_003FB650);


/* Releases the first slot holding the id in either half: both halves and the
   slot's live word are cleared, and the two handles at 0x1B36E0 and 0x1B3720
   are given back to the shared release body. 1 when a slot was found. */
INCLUDE_ASM("asm/levels", func_L09_003FB788);


/* Same release at its own place in the level, on the id byte of a slot rather
   than a live id: both halves and the slot's live word are cleared and the two
   handles are given back. */
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

/* Samples the sub-object through the shared record starter and ramps the
   three floats it wrote into the object's 0x390/0x394/0x398 channels at 0.02
   per frame. When the sub-object's halfword 0x32A is clear and the starter
   reported no live record, the saved 0x324 word and 0x3EC float are put back
   and the 0x388 angle is negated. */
void func_L09_003FDC58(unsigned char *a0) {
    unsigned char *s0 = *(unsigned char **)(a0 + 0x68);
    float buf[4];
    float step = 0.02f;
    float keep = *(float *)(s0 + 0x3EC);
    int save = *(int *)(s0 + 0x324);
    int live = func_L00_00341820(
        (char *)D_L09_001FE6C0[*(int *)(s0 + 0x318)], (char *)buf,
        (char *)(s0 + 0x324), (char *)(s0 + 0x3EC), *(short *)(s0 + 0x32A),
        *(float *)(s0 + 0x388));
    func_002AA3A8((char *)(s0 + 0x390), (char *)(s0 + 0x3A0), buf[0], step, 0.2f,
                  0.0f);
    func_002AA3A8((char *)(s0 + 0x394), (char *)(s0 + 0x3A4), buf[1], step, 0.2f,
                  0.0f);
    func_002AA3A8((char *)(s0 + 0x398), (char *)(s0 + 0x3A8), buf[2], step, 0.2f,
                  0.0f);
    if (*(short *)(s0 + 0x32A) == 0 && live != 0) {
        *(float *)(s0 + 0x3EC) = keep;
        *(int *)(s0 + 0x324) = save;
        *(float *)(s0 + 0x388) = -*(float *)(s0 + 0x388);
    }
}

/* Sums the floats of a sixteen-byte-strided sample block, scales the total by
   the object's 0x38C and 0x3B0, and hands it to the shared record starter
   together with a zero pair it fills in; the three floats it wrote then ramp
   the object's 0x10/0x14/0x18 channels at 0.02 per frame. */
INCLUDE_ASM("asm/levels", func_L09_003FDD80);


/* Feeds the sub-object two measured channel ramps, then, only in state 1 and
   only while the sub-object's halfword 0x3D4 is live, stamps it 0x104 and
   folds a 2.0943954 radian turn into its 0xF8 angle -- sign drawn from the
   resident two-way draw -- before the shared four-float ramp body runs on the
   block at 0x3D8. */
void func_L09_003FDED8(unsigned char *a0) {
    unsigned char *s1 = a0;
    unsigned char *s0 = *(unsigned char **)(s1 + 0x68);
    func_002AEE70(a0, s0 + 0x3C0, s0 + 0x3C4, 0.25f, 0.06f, -1.0f);
    func_002AEF28(s1, s0 + 0x3C8, s0 + 0x3CC, 0.15f, 0.05f, 0.025f);
    if (s1[0x20] != 1)
        return;
    if (func_00282978((unsigned short *)(s0 + 0x3D4)) != 0) {
        float t = 2.0943954f;
        *(short *)(s0 + 0x3D4) = 0x104;
        if (func_002A77E0(2) != 0)
            t = -t;
        *(float *)(s0 + 0x3D0) = func_00283B98(*(float *)(s1 + 0xF8), t);
    }
    func_002AA3A8(s1 + 0xF8, s0 + 0x3D8, *(float *)(s0 + 0x3D0), 0.003f, 0.2f,
                  0.02f);
}

INCLUDE_ASM("asm/levels", func_L09_003FDFF0);

INCLUDE_ASM("asm/levels", func_L09_003FE0C0);

INCLUDE_ASM("asm/levels", func_L09_003FE2D8);

/* Walks the record's track while its signed halfwords stay non-negative and
   returns the entry whose measured value is largest, skipping the entries
   whose field 0xB2 is in the twenty-value screen table. */
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
