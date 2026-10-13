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

void func_L20_0039C448(char *arg0) {
    char *s0 = arg0 + 0x10;
    char *s3 = *(char **)(arg0 + 0x68);
    char *s1;
    char buf[0x30];

    qcopy(s3, s0);
    s1 = s3 + 0x10;
    func_00282CC0(s0, s0, s1);
    func_002A8C00(buf, (int)arg0, 0x10001, s1, *(float *)(s3 + 0x2C));
    if (func_002761F8(s3, s0, 0, *(int *)(arg0 + 0xB8), buf) != 0
        || func_00282948(s3 + 0x20) != 0) {
        func_L20_0039C8C8(D_L20_001C0420, 0x14, 1);
        func_002E5DA0(2, 0, arg0, 0x12EF);
        func_0029F338((int)arg0);
    } else {
        func_L20_0039C5D0(arg0);
        func_L00_002E6EF8(func_L20_0039CC10, (int)arg0);
    }
}

extern char *func_0029EFD0(int);

char *func_L20_0039C540(char *arg0, char *arg1, char *arg2, float f12) {
    char *v;
    char *o = func_0029EFD0(0x12F2);

    if (o != 0) {
        *(int *)(o + 0xB8) = (int)arg0;
        qcopy(o + 0x10, arg1);
        v = *(char **)(o + 0x68);
        *(unsigned char *)(o + 0x30) = 0xFF;
        qcopy(v + 0x10, arg2);
        *(float *)(v + 0x2C) = f12;
        *(int *)(v + 0x20) = 0x12C;
    }
    return o;
}

INCLUDE_ASM("asm/levels", func_L20_0039C5D0);

INCLUDE_ASM("asm/levels", func_L20_0039C8C8);

INCLUDE_ASM("asm/levels", func_L20_0039CC10);

INCLUDE_ASM("asm/levels", func_L20_0039CF58);

INCLUDE_ASM("asm/levels", func_L20_0039D340);

INCLUDE_ASM("asm/levels", func_L20_0039D680);

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

INCLUDE_ASM("asm/levels", func_L20_0039DE88);

INCLUDE_ASM("asm/levels", func_L20_0039DF18);

INCLUDE_ASM("asm/levels", func_L20_0039E0A0);

extern void func_002B3BA8(char *, int, float, float, int, int);

extern char *func_L20_004448A8(char *, char *);

void func_L20_0039E158(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *v;

    if (*(int *)(state + 0x27C) != 0) {
        *(int *)(state + 0x280) = 0;
        func_002B3BA8(arg0, *(int *)(state + 0x230), 0.0f, 0.0f, 0, 0);
    } else {
        *(char **)(state + 0x280) = func_L20_004448A8(*(char **)(arg0 + 0xB8), arg0);
        if (*(char **)(state + 0x280) == 0) {
            *(char **)(state + 0x280) = *(char **)(arg0 + 0xB8);
        }
        v = *(char **)(state + 0x280);
        *(int *)(state + 0x200) = (int)v;
        qcopy(state + 0x1F0, v + 0x10);
    }
}

INCLUDE_ASM("asm/levels", func_L20_0039E1E8);

INCLUDE_ASM("asm/levels", func_L20_0039E368);

INCLUDE_ASM("asm/levels", func_L20_0039E5F0);

INCLUDE_ASM("asm/levels", func_L20_0039E770);

INCLUDE_ASM("asm/levels", func_L20_0039E890);

INCLUDE_ASM("asm/levels", func_L20_0039E9F8);

INCLUDE_ASM("asm/levels", func_L20_0039EBB8);

INCLUDE_ASM("asm/levels", func_L20_0039ED78);

INCLUDE_ASM("asm/levels", func_L20_0039EEC8);

void func_L20_0039F210(char *arg0, char *arg1, int arg2) {
    char *state;

    qcopy(arg0 + 0x10, arg1);
    state = *(char **)(arg0 + 0x68);
    *(int *)(state + 0x0) = 5;
    *(int *)(state + 0xC) = arg2;
}

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

INCLUDE_ASM("asm/levels", func_L20_003C0350);

INCLUDE_ASM("asm/levels", func_L20_003E6728);

INCLUDE_ASM("asm/levels", func_L20_003E6FA0);

INCLUDE_ASM("asm/levels", func_L20_00408D98);

INCLUDE_ASM("asm/levels", func_L20_00409480);

INCLUDE_ASM("asm/levels", func_L20_004095D0);

INCLUDE_ASM("asm/levels", func_L20_00435178);

INCLUDE_ASM("asm/levels", func_L20_00438F98);

INCLUDE_ASM("asm/levels", func_L20_004395B0);

void func_L20_0043D470(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(float *)(*(char **)(state + 0x84) + 0x0) = 8.0f;
    *(float *)(*(char **)(state + 0x84) + 0x4) = 2.5f;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x18) = 1;
    *(unsigned short *)(*(char **)(state + 0x84) + 0x1A) = 0;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x1E) = 1;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x1F) = 0;
    qcopy(*(char **)(state + 0x84) + 0x20, state + 0x60);
}

INCLUDE_ASM("asm/levels", func_L20_0043D4D8);
