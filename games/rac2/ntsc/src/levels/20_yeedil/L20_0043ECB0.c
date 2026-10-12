#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L20_0043ECB0);

INCLUDE_ASM("asm/levels", func_L20_0043F318);

INCLUDE_ASM("asm/levels", func_L20_0043F460);

INCLUDE_ASM("asm/levels", func_L20_0043F510);

INCLUDE_ASM("asm/levels", func_L20_0043F570);

INCLUDE_ASM("asm/levels", func_L20_0043FC70);

INCLUDE_ASM("asm/levels", func_L20_0043FFF0);

INCLUDE_ASM("asm/levels", func_L20_00440150);

INCLUDE_ASM("asm/levels", func_L20_00440198);

INCLUDE_ASM("asm/levels", func_L20_00440398);

extern void func_002A7750(int, int, int);

void func_L20_00440460(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(unsigned short *)(state + 0x178) = 0;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(arg0, 6, 3);
}

INCLUDE_ASM("asm/levels", func_L20_004404B8);

extern void func_0027DDC0(int, int, int, int);

void func_L20_00440570(void) {
    func_0027DDC0(0, 0, 0, 0x80);
}

INCLUDE_ASM("asm/levels", func_L20_00440598);

INCLUDE_ASM("asm/levels", func_L20_00441CB8);

INCLUDE_ASM("asm/levels", func_L20_004421A8);

INCLUDE_ASM("asm/levels", func_L20_004424A0);

INCLUDE_ASM("asm/levels", func_L20_00442550);

INCLUDE_ASM("asm/levels", func_L20_004427C8);

INCLUDE_ASM("asm/levels", func_L20_00442890);

INCLUDE_ASM("asm/levels", func_L20_004429F0);

INCLUDE_ASM("asm/levels", func_L20_00442C10);

INCLUDE_ASM("asm/levels", func_L20_00442CD8);

INCLUDE_ASM("asm/levels", func_L20_00442E68);

extern int func_002ABB78(char *);

extern void func_L02_003A6F78(char *);

extern void func_L20_00449A78(char *);

void func_L20_00442F30(char *arg0) {
    if (func_002ABB78(arg0) != 0) {
        func_L02_003A6F78(arg0);
    } else if (*(short *)(arg0 + 0xAA) == 0x12D3) {
        func_L20_00449A78(arg0);
    }
}

INCLUDE_ASM("asm/levels", func_L20_00442F80);

INCLUDE_ASM("asm/levels", func_L20_004431E0);

INCLUDE_ASM("asm/levels", func_L20_00443328);

INCLUDE_ASM("asm/levels", func_L20_004433B0);

INCLUDE_ASM("asm/levels", func_L20_00443508);

INCLUDE_ASM("asm/levels", func_L20_004435A0);

INCLUDE_ASM("asm/levels", func_L20_00443790);

INCLUDE_ASM("asm/levels", func_L20_00443908);

INCLUDE_ASM("asm/levels", func_L20_00443A10);

INCLUDE_ASM("asm/levels", func_L20_00443B60);

INCLUDE_ASM("asm/levels", func_L20_00443CF0);

INCLUDE_ASM("asm/levels", func_L20_00443D58);

INCLUDE_ASM("asm/levels", func_L20_00443DF0);

INCLUDE_ASM("asm/levels", func_L20_00444018);

INCLUDE_ASM("asm/levels", func_L20_00444088);

INCLUDE_ASM("asm/levels", func_L20_00444130);

INCLUDE_ASM("asm/levels", func_L20_00444138);

INCLUDE_ASM("asm/levels", func_L20_004441D8);

INCLUDE_ASM("asm/levels", func_L20_004448A8);

INCLUDE_ASM("asm/levels", func_L20_00444AA8);

INCLUDE_ASM("asm/levels", func_L20_00444B48);

extern void func_L00_002E6EF8(void (*)(void), int);

void func_L20_00444C68(int arg0) {
    func_002A7750(arg0, 0x12, -1);
    func_L00_002E6EF8(func_L20_00440570, arg0);
}

INCLUDE_ASM("asm/levels", func_L20_00444CA8);

INCLUDE_ASM("asm/levels", func_L20_004450D8);

INCLUDE_ASM("asm/levels", func_L20_004451C0);

INCLUDE_ASM("asm/levels", func_L20_00445738);

INCLUDE_ASM("asm/levels", func_L20_00445890);

INCLUDE_ASM("asm/levels", func_L20_00446E70);

INCLUDE_ASM("asm/levels", func_L20_00446FA8);

INCLUDE_ASM("asm/levels", func_L20_004473B0);

INCLUDE_ASM("asm/levels", func_L20_00447640);

INCLUDE_ASM("asm/levels", func_L20_00447748);

extern char D_L20_002AB980[];

extern void func_L04_003A21F8();

void func_L20_00447908(void) {
    func_L04_003A21F8(D_L20_002AB980, 6);
}

INCLUDE_ASM("asm/levels", func_L20_00447930);

INCLUDE_ASM("asm/levels", func_L20_00448858);

INCLUDE_ASM("asm/levels", func_L20_00448A18);
