#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

INCLUDE_ASM("asm/levels", func_L23_0041E418);

INCLUDE_ASM("asm/levels", func_L23_0041E538);

INCLUDE_ASM("asm/levels", func_L23_0041E570);

INCLUDE_ASM("asm/levels", func_L23_0041E5B0);

INCLUDE_ASM("asm/levels", func_L23_00422540);

INCLUDE_ASM("asm/levels", func_L23_00422668);

INCLUDE_ASM("asm/levels", func_L23_00422798);

INCLUDE_ASM("asm/levels", func_L23_00422A30);

INCLUDE_ASM("asm/levels", func_L23_00422E08);

INCLUDE_ASM("asm/levels", func_L23_00423378);

INCLUDE_ASM("asm/levels", func_L23_00423380);

INCLUDE_ASM("asm/levels", func_L23_004234C0);

INCLUDE_ASM("asm/levels", func_L23_00423588);

INCLUDE_ASM("asm/levels", func_L23_004236B8);

INCLUDE_ASM("asm/levels", func_L23_00425568);

INCLUDE_ASM("asm/levels", func_L23_004256F0);

INCLUDE_ASM("asm/levels", func_L23_00425848);

INCLUDE_ASM("asm/levels", func_L23_004259E0);

INCLUDE_ASM("asm/levels", func_L23_00425AF8);

INCLUDE_ASM("asm/levels", func_L23_004260F0);

INCLUDE_ASM("asm/levels", func_L23_00426248);

INCLUDE_ASM("asm/levels", func_L23_004265D0);

INCLUDE_ASM("asm/levels", func_L23_004277B0);

INCLUDE_ASM("asm/levels", func_L23_004277E8);


/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_004278B0(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_004278D8);

INCLUDE_ASM("asm/levels", func_L23_00427CE8);

INCLUDE_ASM("asm/levels", func_L23_004280D8);

INCLUDE_ASM("asm/levels", func_L23_00428490);

INCLUDE_ASM("asm/levels", func_L23_00428CF8);

INCLUDE_ASM("asm/levels", func_L23_00429098);

INCLUDE_ASM("asm/levels", func_L23_00429430);


INCLUDE_ASM("asm/levels", func_L23_00429DC8);

/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_00429FF8(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_0042A018);

INCLUDE_ASM("asm/levels", func_L23_0042A438);

INCLUDE_ASM("asm/levels", func_L23_0042A8C0);

INCLUDE_ASM("asm/levels", func_L23_0042AD10);

/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_0042AD90(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_0042ADB0);

extern void func_L00_004287E8(void);

extern void func_L00_00428CE0(s32);

/* prepares the shared object then hands it to the shared starter */
void func_L23_0043F930(s32 arg0) {
    func_L00_004287E8();
    func_L00_00428CE0(arg0);
}

INCLUDE_ASM("asm/levels", func_L23_004402D8);
