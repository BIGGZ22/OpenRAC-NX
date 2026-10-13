#include "include_asm.h"

extern void func_321F58();
extern char *func_328768();
extern void func_328DA8();
extern void func_3322F0();
extern void func_3DECE0();
extern void func_3DF6A0();
extern float func_3E22A8();
extern void func_2F5C38();
extern void func_2F5C68();
extern void func_2F6498();
extern void func_2F5CC0();
extern void func_31A618();
INCLUDE_ASM("asm/levels", func_L14_00304190);

INCLUDE_ASM("asm/levels", func_L14_00304280);

INCLUDE_ASM("asm/levels", func_L14_00304588);

INCLUDE_ASM("asm/levels", func_L14_00328E70);

void func_L14_00328E90(char *arg0, char *arg1, char *arg2) {
    char buf[16];
    char *s1 = func_328768(arg0);

    if (s1 != 0) {
        func_2F5C68(buf, arg0 + 0x10, arg1);
        func_328DA8(s1, buf, arg2, arg0 + 0xF0);
    }
}

INCLUDE_ASM("asm/levels", func_L14_0035F510);

INCLUDE_ASM("asm/levels", func_L14_00390DF8);

INCLUDE_ASM("asm/levels", func_L14_00390EC0);

INCLUDE_ASM("asm/levels", func_L14_00390FE8);

void func_L14_00391B58(char *arg0) {
    char buf[16];

    func_2F5CC0(buf, arg0);
    func_2F5C38(arg0 + 0x30, arg0 + 0x30, buf);
}

INCLUDE_ASM("asm/levels", func_L14_00399E28);

INCLUDE_ASM("asm/levels", func_L14_0039A240);

INCLUDE_ASM("asm/levels", func_L14_0039F820);

int func_L14_003C9548(char *arg0) {
    return (arg0[0x95] ^ 1) == 0;
}

INCLUDE_ASM("asm/levels", func_L14_003C9658);

INCLUDE_ASM("asm/levels", func_L14_003C96E0);

INCLUDE_ASM("asm/levels", func_L14_003CB258);

INCLUDE_ASM("asm/levels", func_L14_003CB4D8);

INCLUDE_ASM("asm/levels", func_L14_003CB608);

INCLUDE_ASM("asm/levels", func_L14_003CB868);

INCLUDE_ASM("asm/levels", func_L14_003D18C8);

INCLUDE_ASM("asm/levels", func_L14_003D54A8);

INCLUDE_ASM("asm/levels", func_L14_003DDD38);

int func_L14_003DE2D8(char *arg0) {
    int *p = (int *)(*(int *)(arg0 + 0x68) + 0xE0);
    int count = 0;
    int i;

    for (i = 3; i >= 0; i--) {
        if (*p != 0) {
            count++;
        }
        p++;
    }
    return count;
}

INCLUDE_ASM("asm/levels", func_L14_003DE310);

INCLUDE_ASM("asm/levels", func_L14_003DE360);

void func_L14_003DE488(char *arg0) {
    char **p = (char **)(*(int *)(arg0 + 0x68) + 0xE0);
    int i;

    for (i = 3; i >= 0; i--) {
        char *v = *p;

        if (v != 0 && *(int *)(*(int *)(v + 0x68) + 0x2E0) != 0) {
            func_3322F0(v, 1);
        }
        p++;
    }
}

INCLUDE_ASM("asm/levels", func_L14_003DE4E8);

int func_L14_003DE8E8(unsigned char *arg0) {
    return (unsigned int)(arg0[0x20] - 2) < 2;
}

void func_L14_003DE8F8(char *arg0) {
    *(int *)(*(int *)(arg0 + 0x68) + 0x84) = 1;
}

void func_L14_003DE990(char *arg0) {
    func_321F58(arg0, 2, -1);
}

INCLUDE_ASM("asm/levels", func_L14_003DE9B0);

INCLUDE_ASM("asm/levels", func_L14_003DECE0);

INCLUDE_ASM("asm/levels", func_L14_003DF100);

INCLUDE_ASM("asm/levels", func_L14_003DF200);

INCLUDE_ASM("asm/levels", func_L14_003DF440);

INCLUDE_ASM("asm/levels", func_L14_003DF540);

void func_L14_003DF650(unsigned char *arg0) {
    if (arg0[0x20] == 0) {
        func_3DECE0();
    }
    *(int *)(arg0 + 0x64) = (int)func_3DF6A0;
}

INCLUDE_ASM("asm/levels", func_L14_003DF690);

INCLUDE_ASM("asm/levels", func_L14_003DF6A0);

INCLUDE_ASM("asm/levels", func_L14_003DF7F8);

INCLUDE_ASM("asm/levels", func_L14_003DFA10);

INCLUDE_ASM("asm/levels", func_L14_003DFA60);

INCLUDE_ASM("asm/levels", func_L14_003E1DD0);

INCLUDE_ASM("asm/levels", func_L14_003E1E78);

void func_L14_003E2238(char *arg0) {
    float f = func_3E22A8(arg0);

    func_2F6498(arg0 + 0xC0, arg0 + 0xF0);
    ((void (*)(float *, float *, float))func_2F5CC0)((float *)(arg0 + 0xC0),
                                                     (float *)(arg0 + 0xC0), f);
    ((void (*)(float *, float *, float))func_2F5CC0)((float *)(arg0 + 0xD0),
                                                     (float *)(arg0 + 0xD0), f);
    func_31A618(arg0);
}

INCLUDE_ASM("asm/levels", func_L14_003E22A8);

INCLUDE_ASM("asm/levels", func_L14_003E2328);

INCLUDE_ASM("asm/levels", func_L14_003E2688);

INCLUDE_ASM("asm/levels", func_L14_003E28E8);

INCLUDE_ASM("asm/levels", func_L14_003E2980);

INCLUDE_ASM("asm/levels", func_L14_003E4208);

INCLUDE_ASM("asm/levels", func_L14_003E4A40);

INCLUDE_ASM("asm/levels", func_L14_003E4BA0);

INCLUDE_ASM("asm/levels", func_L14_003E4EA0);

INCLUDE_ASM("asm/levels", func_L14_003E5030);

INCLUDE_ASM("asm/levels", func_L14_003E50B8);

void func_L14_003E5190(char *arg0) {
    char *v0 = *(char **)(arg0 + 0x68);
    v0 = *(char **)(v0 + 0x30);
    if (v0 != 0) {
        v0 = *(char **)(v0 + 0x68);
        *(int *)(v0 + 0x3C) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L14_003E51B0);

INCLUDE_ASM("asm/levels", func_L14_003E5218);

INCLUDE_ASM("asm/levels", func_L14_003F0D50);

INCLUDE_ASM("asm/levels", func_L14_003F0E98);

INCLUDE_ASM("asm/levels", func_L14_003F0F38);
