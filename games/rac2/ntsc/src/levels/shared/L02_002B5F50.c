#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_002B5F50);

INCLUDE_ASM("asm/levels", func_L02_002DCEA0);

INCLUDE_ASM("asm/levels", func_L02_002EF388);

INCLUDE_ASM("asm/levels", func_L02_002EF968);

INCLUDE_ASM("asm/levels", func_L02_002EFB60);

INCLUDE_ASM("asm/levels", func_L02_002F1650);

INCLUDE_ASM("asm/levels", func_L02_002F1FB8);

INCLUDE_ASM("asm/levels", func_L02_002F21C0);

INCLUDE_ASM("asm/levels", func_L02_0031E918);

INCLUDE_ASM("asm/levels", func_L02_0031E9E8);

INCLUDE_ASM("asm/levels", func_L02_0031F858);

INCLUDE_ASM("asm/levels", func_L02_00321C30);

INCLUDE_ASM("asm/levels", func_L02_003272D8);

INCLUDE_ASM("asm/levels", func_L02_00327350);

INCLUDE_ASM("asm/levels", func_L02_00327390);

INCLUDE_ASM("asm/levels", func_L02_00329410);

INCLUDE_ASM("asm/levels", func_L02_003294D8);

INCLUDE_ASM("asm/levels", func_L02_00329C48);

INCLUDE_ASM("asm/levels", func_L02_0032B468);

extern void func_L02_0032E3B0(void *a0, void *a1);

/* Forward the call with the second pointer advanced by one record. */
void func_L02_0032E390(void *a0, void *a1) {
    func_L02_0032E3B0(a0, (char *)a1 + 0x10);
}

INCLUDE_ASM("asm/levels", func_L02_0032E3B0);

INCLUDE_ASM("asm/levels", func_L02_0032E508);

INCLUDE_ASM("asm/levels", func_L02_0032E998);

INCLUDE_ASM("asm/levels", func_L02_0032EC98);

INCLUDE_ASM("asm/levels", func_L02_0032EDD8);

INCLUDE_ASM("asm/levels", func_L02_0032EF10);

INCLUDE_ASM("asm/levels", func_L02_0032F008);

INCLUDE_ASM("asm/levels", func_L02_00331440);

INCLUDE_ASM("asm/levels", func_L02_00332630);

INCLUDE_ASM("asm/levels", func_L02_00332668);

INCLUDE_ASM("asm/levels", func_L02_00332900);

INCLUDE_ASM("asm/levels", func_L02_003329C8);

INCLUDE_ASM("asm/levels", func_L02_00332A80);

INCLUDE_ASM("asm/levels", func_L02_00332D28);

INCLUDE_ASM("asm/levels", func_L02_00332EF0);

INCLUDE_ASM("asm/levels", func_L02_00333218);

INCLUDE_ASM("asm/levels", func_L02_00333300);

INCLUDE_ASM("asm/levels", func_L02_00333438);

INCLUDE_ASM("asm/levels", func_L02_00333480);

INCLUDE_ASM("asm/levels", func_L02_003449D8);

INCLUDE_ASM("asm/levels", func_L02_00348420);

INCLUDE_ASM("asm/levels", func_L02_00349BD8);

INCLUDE_ASM("asm/levels", func_L02_00389F70);

INCLUDE_ASM("asm/levels", func_L02_0038A008);

INCLUDE_ASM("asm/levels", func_L02_0038BA28);

INCLUDE_ASM("asm/levels", func_L02_0038BD00);

INCLUDE_ASM("asm/levels", func_L02_0038C998);

INCLUDE_ASM("asm/levels", func_L02_0038CDE8);

INCLUDE_ASM("asm/levels", func_L02_0038D978);

INCLUDE_ASM("asm/levels", func_L02_0038DA00);

INCLUDE_ASM("asm/levels", func_L02_0038DA60);

INCLUDE_ASM("asm/levels", func_L02_0038DC68);

INCLUDE_ASM("asm/levels", func_L02_0038DE58);

INCLUDE_ASM("asm/levels", func_L02_0038E048);

INCLUDE_ASM("asm/levels", func_L02_0038E250);

INCLUDE_ASM("asm/levels", func_L02_0038E430);

INCLUDE_ASM("asm/levels", func_L02_0038E610);

INCLUDE_ASM("asm/levels", func_L02_0038E838);

/* Raise the byte field to eight when it is below. */
void func_L02_00392C70(unsigned char *a0) {
    if (a0[0x20] < 8) {
        a0[0x20] = 8;
    }
}

/* Raise the byte field to eight when it is below. */
void func_L02_003948D0(unsigned char *a0) {
    if (a0[0x20] < 8) {
        a0[0x20] = 8;
    }
}

INCLUDE_ASM("asm/levels", func_L02_003A6F78);

INCLUDE_ASM("asm/levels", func_L02_003A6FE8);

INCLUDE_ASM("asm/levels", func_L02_003A8770);

INCLUDE_ASM("asm/levels", func_L02_003AA650);

INCLUDE_ASM("asm/levels", func_L02_003B6548);

INCLUDE_ASM("asm/levels", func_L02_003C0890);

INCLUDE_ASM("asm/levels", func_L02_003C26F0);

INCLUDE_ASM("asm/levels", func_L02_003C4068);

INCLUDE_ASM("asm/levels", func_L02_003C4280);
