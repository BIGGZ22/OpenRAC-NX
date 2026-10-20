#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern float D_L01_001AF628 __attribute__((sda));

extern float func_00283B98(float, float);

extern void func_00283898(char *, int, float);

extern void func_0029F978(char *, int, char *);

extern float func_002A7940(void);

/* grows a moby out of the ground the first time, then spins it down */
void func_L01_00399170(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        *(unsigned char *)(a0 + 0x20) = 1;
        *(unsigned char *)(a0 + 0x63) = 0x20;
        *(float *)(s1 + 0x40) = func_002A7940();
        func_0029F978(a0, 1, s1);
        break;
    case 1: {
        float f = D_L01_001AF628 * 0.000290888245f;

        *(float *)(a0 + 0xF8) = func_00283B98(*(float *)(a0 + 0xF8), f);
        *(float *)(s1 + 0x40) = func_00283B98(*(float *)(s1 + 0x40), f * -2.0f);
        func_00283898(s1 + 0x10, 2, *(float *)(s1 + 0x40));
        break;
    }
    }
}

/* spins the moby's wheels down and leans them, then steps the frame counter */

extern float D_L01_001AF50C __attribute__((sda));

extern float D_L01_001AF510 __attribute__((sda));

extern float D_L01_001AF514 __attribute__((sda));

extern float D_L01_001AF528 __attribute__((sda));

extern int D_L01_001AF54C __attribute__((sda));

extern float func_002E3A60(float);

extern float func_002E4460(float, float);

extern int func_003104E8(int);

extern void func_L01_00397A28(char *);

INCLUDE_ASM("asm/levels", func_L01_00397EB0);

extern void func_002E3470(void *, void *, void *);

extern void func_002E3CB8(char *, char *);

extern void func_002E3738(void *, char *, float);

extern void func_00308938(char *);

/* turns each of the three carried objects with a swinging pitch */
INCLUDE_ASM("asm/levels", func_L01_00398938);

extern float D_L01_001B1A9C __attribute__((sda));

extern float D_L01_001B1AA0 __attribute__((sda));

extern float D_L01_001B1AA4 __attribute__((sda));

extern float D_L01_001B1AA8 __attribute__((sda));

extern unsigned char D_L01_001B1AAC __attribute__((sda));

extern int D_L01_001B1AB0 __attribute__((sda));

extern int D_L01_001B1AB4 __attribute__((sda));

extern float D_L01_001B1AB8 __attribute__((sda));

extern float D_L01_001B1ABC __attribute__((sda));

extern float D_L01_001B1B24 __attribute__((sda));

extern void func_00306250(char *);

extern void func_00282D30(void *, void *, float);

extern void func_0029FCC0(void *, int, void *);

extern void func_002A7A00(void *, float, float);

extern int func_002A77E0(int);

extern void func_002C3150(void *, void *, int, unsigned char, unsigned char, int, float);

extern void func_00282CC0(void *, void *, void *);

/* builds the tilt of a hanging chain over three steps and draws it */


extern float D_L01_001AF638 __attribute__((sda));

extern float D_L01_001AF63C __attribute__((sda));

extern float D_L01_001B2EF0 __attribute__((sda));

extern float D_L01_001B2F00 __attribute__((sda));

extern float D_001B2EF4 __attribute__((nosda));

extern int D_001B2F04 __attribute__((nosda));

extern float func_00310580(float, float);

extern float func_002E35D0(char *);

extern float func_002E35A0(float *);

/* sets up the swing band of the level's moving part */
INCLUDE_ASM("asm/levels", func_L01_003993C8);

extern char *D_001B326C __attribute__((nosda));

extern int D_001B3270 __attribute__((nosda));

extern int D_L01_001B3270 __attribute__((sda));

extern char *func_00305EE8(int);

extern void func_00307EF8(char *, int *, int *, int *);

/* registers a new particle slot of class 0xC41, up to 0x40 of them */
INCLUDE_ASM("asm/levels", func_L01_003E9CF8);

/* clears the three child slots, holds the wheels and counts the lifetime down */
INCLUDE_ASM("asm/levels", func_L01_0041C890);

extern void func_L01_003EE8E0(char *);

extern void func_002A7470(char *, int, int, int);

extern void func_002A7398(char *, int, int);

extern int func_00282948(char *);

extern const int D_L01_001B0D10 __attribute__((sda));

extern const int D_L01_001B0D14 __attribute__((sda));

/* brings the ring out of the ground, then lets it spin and hand itself over */
void func_L01_003EE6F0(char *a0)
{
    char *s1;

    s1 = *(char **)(a0 + 0x68);
    func_L01_003EE8E0(a0);

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        *(float *)(a0 + 0x18) = *(float *)(a0 + 0x18) + 1.0f;
        *(float *)(s1 + 0x14) = *(float *)(a0 + 0xF0);
        *(unsigned char *)(a0 + 0x20) = 1;
        if (*(unsigned char *)(a0 + 0x43) != 0)
            func_002A7470(a0, 0, 0, 0xC);
        *(int *)(s1 + 0x50) = 0;
        *(unsigned short *)(a0 + 0x34) |= 0x1000;
        break;
    case 1:
        if ((*(unsigned char *)(a0 + 0x60) & 2) != 0) {
            if (*(unsigned char *)(a0 + 0x43) != 0)
                func_002A7398(a0, 0, 0);
        }
        if (*(unsigned char *)(a0 + 0xBC) == 0)
            break;
        *(unsigned short *)(a0 + 0x34) &= 0xEFFF;
        func_002A7398(a0, 1, 0);
        if (*(int *)(s1 + 0x0) != 0) {
            *(int *)(s1 + 0x10) = 10;
            *(unsigned char *)(a0 + 0x20) = 2;
            *(int *)(s1 + 0xC) = *(int *)(s1 + 0x4);
        } else {
            *(unsigned char *)(a0 + 0x20) = 2;
            *(int *)(a0 + 0x78) = D_L01_001B0D14;
            *(int *)(s1 + 0xC) = 0;
        }
        break;
    case 2:
        if (*(int *)(s1 + 0x0) != 0) {
            if (func_00282948(s1 + 0x10) != 0) {
                if (*(int *)(a0 + 0x78) == D_L01_001B0D10)
                    *(int *)(a0 + 0x78) = D_L01_001B0D14;
                else
                    *(int *)(a0 + 0x78) = D_L01_001B0D10;
                *(int *)(s1 + 0x10) = 10;
            }
            if (func_00282948(s1 + 0xC) != 0)
                *(unsigned char *)(a0 + 0xBC) = 0;
        }
        if ((*(unsigned char *)(a0 + 0x60) & 2) != 0) {
            if (*(unsigned char *)(a0 + 0x43) == 1)
                func_002A7398(a0, 2, 0);
        }
        if (*(unsigned char *)(a0 + 0xBC) != 0)
            break;
        if ((*(unsigned char *)(a0 + 0x60) & 2) != 0) {
            *(unsigned short *)(a0 + 0x34) |= 0x1000;
            func_002A7398(a0, 3, 0);
            *(unsigned char *)(a0 + 0x20) = 1;
            *(int *)(a0 + 0x78) = D_L01_001B0D10;
        }
        break;
    }
}
