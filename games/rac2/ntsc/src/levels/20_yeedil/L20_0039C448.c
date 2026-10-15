#include "common.h"

extern void func_00282CC0(char *, char *, char *);

extern void func_002A8C00(char *, int, int, void *, float);

extern int func_002761F8(void *, void *, int, int, void *);

extern int func_00282948(void *);

extern void func_002E5DA0(int, int, void *, int);

extern void func_L00_002E6EF8(void *, int);

extern void func_L20_0039C5D0(char *);

extern void func_L20_0039C8C8(char *, int, int);

extern void func_L20_0039CC10(void);

extern char D_L20_001C0420[];

INCLUDE_ASM("asm/levels", func_L20_0039C448);

extern char *func_0029EFD0(int);

INCLUDE_ASM("asm/levels", func_L20_0039C540);

INCLUDE_ASM("asm/levels", func_L20_0039C5D0);

INCLUDE_ASM("asm/levels", func_L20_0039C8C8);

INCLUDE_ASM("asm/levels", func_L20_0039CC10);

INCLUDE_ASM("asm/levels", func_L20_0039CF58);

INCLUDE_ASM("asm/levels", func_L20_0039D340);

INCLUDE_ASM("asm/levels", func_L20_0039D680);

extern int D_L20_001B2AC8;

extern void func_L20_0039DF18(char *, int);

extern void func_L20_0039D880(char *);

extern void func_L20_0039D680(char *);

extern float func_00283BE0(float, float);

extern void func_002AAA00(char *, char *, int, float, float, float, float);

INCLUDE_ASM("asm/levels", func_L20_0039D7B0);

INCLUDE_ASM("asm/levels", func_L20_0039D880);

extern void func_002A7470(void *, int, int, int);

void func_L20_0039DB50(char *arg0, int arg1, int arg2, float f12) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x43) != arg1) {
        func_002A7470(arg0, arg1, (int)f12, arg2);
    }
    func_002A7470(*(char **)(state + 0x254), arg1, (int)f12, arg2);
}

INCLUDE_ASM("asm/levels", func_L20_0039DBC8);

INCLUDE_ASM("asm/levels", func_L20_0039DCF8);

extern void func_0029F338(int);

extern void func_002A7750(char *, int, int);

void func_L20_0039DE88(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char **p;
    int i;

    func_002A7750(arg0, 6, 2);
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    *(int *)(arg0 + 0x98) = 0;
    if (*(int *)(state + 0x254) != 0) {
        func_0029F338(*(int *)(state + 0x254));
        *(int *)(state + 0x254) = 0;
    }
    p = (char **)(state + 0x258);
    for (i = 1; i >= 0; i--) {
        if (*p != 0) {
            func_0029F338(*p);
            *p = 0;
        }
        p++;
    }
}

INCLUDE_ASM("asm/levels", func_L20_0039DF18);

INCLUDE_ASM("asm/levels", func_L20_0039E0A0);

extern void func_002B3BA8(char *, int, float, float, int, int);

extern char *func_L20_004448A8(char *, char *);

INCLUDE_ASM("asm/levels", func_L20_0039E158);

INCLUDE_ASM("asm/levels", func_L20_0039E1E8);

INCLUDE_ASM("asm/levels", func_L20_0039E368);

INCLUDE_ASM("asm/levels", func_L20_0039E5F0);
INCLUDE_ASM("asm/levels", func_L20_0039E770);

INCLUDE_ASM("asm/levels", func_L20_0039E890);

INCLUDE_ASM("asm/levels", func_L20_0039E9F8);

INCLUDE_ASM("asm/levels", func_L20_0039EBB8);

INCLUDE_ASM("asm/levels", func_L20_0039ED78);

INCLUDE_ASM("asm/levels", func_L20_0039EEC8);

INCLUDE_ASM("asm/levels", func_L20_0039F210);

extern void func_L20_0039F268(char *);

extern void func_L20_0039F560(char *);

extern void func_L20_0039F968(char *);

void func_L20_0039F230(char *arg0) {
    func_L20_0039F268(arg0);
    func_L20_0039F560(arg0);
    func_L20_0039F968(arg0);
}

INCLUDE_ASM("asm/levels", func_L20_0039F268);

INCLUDE_ASM("asm/levels", func_L20_0039F560);

INCLUDE_ASM("asm/levels", func_L20_0039F968);

INCLUDE_ASM("asm/levels", func_L20_003A0320);

INCLUDE_ASM("asm/levels", func_L20_003AB090);

INCLUDE_ASM("asm/levels", func_L20_003AB4A8);

INCLUDE_ASM("asm/levels", func_L20_003B0A88);

INCLUDE_ASM("asm/levels", func_L20_003BFEB8);

INCLUDE_ASM("asm/levels", func_L20_003C0048);

INCLUDE_ASM("asm/levels", func_L20_003C02D0);

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern float D_L20_0023E670[];

extern float func_00283CE0(int);

extern float func_002A7AA8(float, float, float, float, float);

INCLUDE_ASM("asm/levels", func_L20_003C0350);

INCLUDE_ASM("asm/levels", func_L20_003E6728);

INCLUDE_ASM("asm/levels", func_L20_003E6FA0);

INCLUDE_ASM("asm/levels", func_L20_00408D98);

INCLUDE_ASM("asm/levels", func_L20_00409480);

/* func_L20_004095D0 is defined in L20_0039C448-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L20_00435178);

INCLUDE_ASM("asm/levels", func_L20_00438F98);

INCLUDE_ASM("asm/levels", func_L20_004395B0);
INCLUDE_ASM("asm/levels", func_L20_0043D470);

INCLUDE_ASM("asm/levels", func_L20_0043D4D8);
