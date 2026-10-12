#include "include_asm.h"

extern void func_002A05A8(char *);

extern void func_002A7470(char *, int, int, int);

extern void func_002A7750(char *, int, int);

extern int func_002A77E0(int);

extern void func_002B9180(char *);

extern void func_L02_003D5C20(char *, int, int, int);

extern void func_L02_003D5DF8(char *);

extern void func_L02_003D6348(char *);

extern void func_L02_003D6538(char *);

extern void func_L02_003D6388(char *, int, int);

extern void func_L02_003D6570(char *, int);

INCLUDE_ASM("asm/levels", func_L02_003D26B0);

INCLUDE_ASM("asm/levels", func_L02_003D2DE0);

INCLUDE_ASM("asm/levels", func_L02_003D4308);

INCLUDE_ASM("asm/levels", func_L02_003D46B8);

INCLUDE_ASM("asm/levels", func_L02_003D47B0);

INCLUDE_ASM("asm/levels", func_L02_003D4900);

INCLUDE_ASM("asm/levels", func_L02_003D4980);

INCLUDE_ASM("asm/levels", func_L02_003D4990);

void func_L02_003D4A48(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(short *)(state + 0x318) != 0) {
        *(short *)(state + 0x318) = 0;
    }
}

int func_L02_003D4A60(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *holder = *(char **)(state + 0x310);
    unsigned char mode;
    int hit = 0;

    if (holder != 0) {
        mode = *(unsigned char *)(holder + 0x20);
        if (mode != 0xFE) {
            if (mode != 0xFD) {
                if (mode >= 3) {
                    if (*(float *)(state + 0x20) <
                        (float)*(short *)(state + 0x24) *
                            (3.0f - (float)*(short *)(state + 0x31A)) * 0.33333334f) {
                        hit = 1;
                    }
                }
            }
        }
    }
    return hit;
}

INCLUDE_ASM("asm/levels", func_L02_003D4AF0);

int func_L02_003D4D00(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    short timer = *(short *)(state + 0x32E);
    int delta;

    if (timer < 0x12C) {
        return 0;
    }
    if (timer < 0x1F5) {
        delta = 0x1F4 - *(short *)(state + 0x32A);
        if (delta > 0) {
            if (func_002A77E0(delta) != 0) {
                return 0;
            }
        }
    }
    *(short *)(state + 0x32A) = 0;
    *(short *)(state + 0x32E) = 0;
    return 1;
}

INCLUDE_ASM("asm/levels", func_L02_003D4E00);

INCLUDE_ASM("asm/levels", func_L02_003D4ED0);

INCLUDE_ASM("asm/levels", func_L02_003D4FD0);

INCLUDE_ASM("asm/levels", func_L02_003D5118);

INCLUDE_ASM("asm/levels", func_L02_003D52A8);

INCLUDE_ASM("asm/levels", func_L02_003D5570);

INCLUDE_ASM("asm/levels", func_L02_003D5660);

INCLUDE_ASM("asm/levels", func_L02_003D5748);

void func_L02_003D5B90(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *holder = *(char **)(state + 0x300);
    char **slots;
    char *node;
    int i;

    if (holder != 0) {
        func_L02_003D5DF8(holder);
        *(int *)(state + 0x300) = 0;
    }
    *(unsigned char *)(state + 0x222) = 0;
    *(float *)(state + 0x248) = 1.0f;
    *(float *)(state + 0x244) = 1.0f;
    *(float *)(state + 0x240) = 1.0f;
    *(float *)(state + 0x288) = 1.0f;
    *(float *)(state + 0x284) = 1.0f;
    *(float *)(state + 0x280) = 1.0f;
    *(unsigned char *)(state + 0x262) = 0;
    slots = (char **)(state + 0x2A0);
    for (i = 4; i >= 0; i--) {
        node = slots[0];
        if (node != 0) {
            *(unsigned short *)(node + 0x34) = *(unsigned short *)(node + 0x34) | 1;
            *(unsigned char *)(slots[0] + 0x31) = 0;
        }
        slots++;
    }
}

INCLUDE_ASM("asm/levels", func_L02_003D5C20);

INCLUDE_ASM("asm/levels", func_L02_003D5CF0);

INCLUDE_ASM("asm/levels", func_L02_003D5DF8);

INCLUDE_ASM("asm/levels", func_L02_003D5E40);

INCLUDE_ASM("asm/levels", func_L02_003D5F50);

void func_L02_003D62F0(char *arg0) {
    char *state = arg0;
    unsigned char mode;

    func_L02_003D6538(state);
    mode = *(unsigned char *)(state + 0x95);
    if (mode == 2) {
        func_L02_003D6348(state);
    }
    mode = *(unsigned char *)(state + 0x42);
    if (mode == 2) {
        func_L02_003D6A20(state);
    }
}

void func_L02_003D6348(char *arg0) {
    func_L02_003D6388(arg0, 2, 0);
    func_L02_003D6388(arg0, 3, 1);
}

INCLUDE_ASM("asm/levels", func_L02_003D6388);

void func_L02_003D6538(char *arg0) {
    func_L02_003D6570(arg0, 4);
    func_L02_003D6570(arg0, 5);
}

INCLUDE_ASM("asm/levels", func_L02_003D6570);

INCLUDE_ASM("asm/levels", func_L02_003D6690);

INCLUDE_ASM("asm/levels", func_L02_003D69F0);

INCLUDE_ASM("asm/levels", func_L02_003D6A20);

/* Whether the object's variant byte is one of the later ones. */
int func_L02_003D6BB0(char *arg0) {
    return *(unsigned char *)(arg0 + 0x20) >= 3;
}

INCLUDE_ASM("asm/levels", func_L02_003D7330);

INCLUDE_ASM("asm/levels", func_L02_003D7900);

INCLUDE_ASM("asm/levels", func_L02_003D7C20);

INCLUDE_ASM("asm/levels", func_L02_003D91A0);

INCLUDE_ASM("asm/levels", func_L02_003D9608);

INCLUDE_ASM("asm/levels", func_L02_003D9728);

INCLUDE_ASM("asm/levels", func_L02_003D9E68);

INCLUDE_ASM("asm/levels", func_L02_003DAB68);

INCLUDE_ASM("asm/levels", func_L02_003DAD88);

INCLUDE_ASM("asm/levels", func_L02_003DAE60);

INCLUDE_ASM("asm/levels", func_L02_003DAF90);



INCLUDE_ASM("asm/levels", func_L02_003DB030);

INCLUDE_ASM("asm/levels", func_L02_003DB158);



INCLUDE_ASM("asm/levels", func_L02_003DB200);

void func_L02_003DB2F0(char *arg0) {
    func_002A7750(arg0, 7, -1);
}
