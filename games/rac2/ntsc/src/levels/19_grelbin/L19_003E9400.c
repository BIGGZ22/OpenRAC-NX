#include "include_asm.h"
#include "common.h"

INCLUDE_ASM("asm/levels", func_L19_003E9400);

INCLUDE_ASM("asm/levels", func_L19_00411890);

INCLUDE_ASM("asm/levels", func_L19_00411EF0);

INCLUDE_ASM("asm/levels", func_L19_00412140);

INCLUDE_ASM("asm/levels", func_L19_004160A0);

INCLUDE_ASM("asm/levels", func_L19_004170D8);

extern void func_2EB590();

extern void func_2EB5C0();

extern float func_3183D8(float, float);

extern float func_2EBF18(float);

extern float func_2EBF30(float);

void func_L19_00417A00(char *a0)
{
    int obj = *(int *)(a0 + 0x68);

    func_2EB590(obj + 0x218);
    func_2EB5C0(obj + 0x21C);
    if (*(unsigned short *)(a0 + 0x96) < 0xFFFF) {
        *(unsigned short *)(a0 + 0x96) += 1;
    }
}

INCLUDE_ASM("asm/levels", func_L19_00417A58);

INCLUDE_ASM("asm/levels", func_L19_00417CA0);

INCLUDE_ASM("asm/levels", func_L19_00417F88);

INCLUDE_ASM("asm/levels", func_L19_0041B078);

INCLUDE_ASM("asm/levels", func_L19_0041BFD0);

INCLUDE_ASM("asm/levels", func_L19_0041C328);

INCLUDE_ASM("asm/levels", func_L19_0041C3D8);

INCLUDE_ASM("asm/levels", func_L19_0041E5B0);

INCLUDE_ASM("asm/levels", func_L19_0041EE60);

INCLUDE_ASM("asm/levels", func_L19_004203F0);

INCLUDE_ASM("asm/levels", func_L19_00420818);

INCLUDE_ASM("asm/levels", func_L19_00420B48);

INCLUDE_ASM("asm/levels", func_L19_00420EF0);

INCLUDE_ASM("asm/levels", func_L19_00421E80);

INCLUDE_ASM("asm/levels", func_L19_00422178);

INCLUDE_ASM("asm/levels", func_L19_00422238);

INCLUDE_ASM("asm/levels", func_L19_00422810);

INCLUDE_ASM("asm/levels", func_L19_00422A98);

INCLUDE_ASM("asm/levels", func_L19_00423270);

INCLUDE_ASM("asm/levels", func_L19_00423548);

INCLUDE_ASM("asm/levels", func_L19_00423A98);

INCLUDE_ASM("asm/levels", func_L19_00423CC0);

INCLUDE_ASM("asm/levels", func_L19_00423EC0);
