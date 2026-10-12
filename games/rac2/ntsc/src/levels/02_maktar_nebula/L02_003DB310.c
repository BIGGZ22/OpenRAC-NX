#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_003DB310);

INCLUDE_ASM("asm/levels", func_L02_003DB6D8);

INCLUDE_ASM("asm/levels", func_L02_003DB870);

INCLUDE_ASM("asm/levels", func_L02_003DB880);

void func_L02_003DB938(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(short *)(state + 0x280) != 0)
        *(short *)(state + 0x280) = 0;
}

extern void func_002A7750();

int func_L02_003DB950(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *other = *(char **)(state + 0x264);
    unsigned char kind;
    int result = 0;

    if (other == 0)
        goto done;
    kind = *(unsigned char *)(other + 0x20);
    if (kind == 0xFE)
        goto done;
    if (kind == 0xFD)
        goto done;
    if (kind < 3)
        goto done;
    if (*(float *)(state + 0x20) <
        (float)*(short *)(state + 0x24) *
            (3.0f - (float)*(short *)(state + 0x282)) * 0.33333334f)
        result = 1;
done:
    return result;
}

INCLUDE_ASM("asm/levels", func_L02_003DBA70);

INCLUDE_ASM("asm/levels", func_L02_003DBB70);

extern char D_L02_001BA500;

extern float func_00282E48();

extern void func_002A93B0();

void func_L02_003DBE60(char *arg0) {
    if (*(unsigned char *)(arg0 + 0x31) != 0) {
        if (func_00282E48(arg0 + 0x10, &D_L02_001BA500) < 64.0f) {
            func_002A93B0(arg0);
            *(unsigned char *)(arg0 + 0x6F) = 60;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L02_003DBF40);

INCLUDE_ASM("asm/levels", func_L02_003DBFD8);

INCLUDE_ASM("asm/levels", func_L02_003DEDA0);

INCLUDE_ASM("asm/levels", func_L02_003E6D70);

INCLUDE_ASM("asm/levels", func_L02_003E7370);

INCLUDE_ASM("asm/levels", func_L02_003E75A8);

INCLUDE_ASM("asm/levels", func_L02_003E77E0);

INCLUDE_ASM("asm/levels", func_L02_003E79F8);

INCLUDE_ASM("asm/levels", func_L02_003E7B58);

INCLUDE_ASM("asm/levels", func_L02_003E7F28);

INCLUDE_ASM("asm/levels", func_L02_003E80F8);

INCLUDE_ASM("asm/levels", func_L02_003E87D8);

INCLUDE_ASM("asm/levels", func_L02_003E8CE8);

INCLUDE_ASM("asm/levels", func_L02_003E8E90);

INCLUDE_ASM("asm/levels", func_L02_003E9040);

INCLUDE_ASM("asm/levels", func_L02_003E91F0);

INCLUDE_ASM("asm/levels", func_L02_003E98A8);

INCLUDE_ASM("asm/levels", func_L02_003E9BB0);

INCLUDE_ASM("asm/levels", func_L02_003EA180);

INCLUDE_ASM("asm/levels", func_L02_003EA488);

INCLUDE_ASM("asm/levels", func_L02_003EA4E0);

INCLUDE_ASM("asm/levels", func_L02_003EAA68);

INCLUDE_ASM("asm/levels", func_L02_003EAD38);

INCLUDE_ASM("asm/levels", func_L02_003EAE10);

INCLUDE_ASM("asm/levels", func_L02_003EAFC8);

INCLUDE_ASM("asm/levels", func_L02_003EB098);

INCLUDE_ASM("asm/levels", func_L02_003EB138);

INCLUDE_ASM("asm/levels", func_L02_003EB258);

INCLUDE_ASM("asm/levels", func_L02_003EB648);

INCLUDE_ASM("asm/levels", func_L02_003EE938);

INCLUDE_ASM("asm/levels", func_L02_003EEAD8);

INCLUDE_ASM("asm/levels", func_L02_003EEC58);

INCLUDE_ASM("asm/levels", func_L02_003EEDD0);

INCLUDE_ASM("asm/levels", func_L02_003EEFB8);

INCLUDE_ASM("asm/levels", func_L02_003F0798);

INCLUDE_ASM("asm/levels", func_L02_003F1298);

INCLUDE_ASM("asm/levels", func_L02_003F1578);

INCLUDE_ASM("asm/levels", func_L02_003F1608);

INCLUDE_ASM("asm/levels", func_L02_003F16A8);

INCLUDE_ASM("asm/levels", func_L02_003F16F0);

INCLUDE_ASM("asm/levels", func_L02_003F17B0);

INCLUDE_ASM("asm/levels", func_L02_003F18E8);

INCLUDE_ASM("asm/levels", func_L02_003F1A88);

void func_L02_003F1B70(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(short *)(state + 0x4EC) = *(unsigned char *)(arg0 + 0x20);
    *(short *)(state + 0x4EE) = *(unsigned char *)(arg0 + 0x95);
    func_002A7750(arg0, 6, 4);
}

INCLUDE_ASM("asm/levels", func_L02_003F1BA8);

INCLUDE_ASM("asm/levels", func_L02_003F52B0);

INCLUDE_ASM("asm/levels", func_L02_003F5390);
