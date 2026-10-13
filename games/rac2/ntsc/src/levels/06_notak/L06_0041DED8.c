#include "include_asm.h"

extern char D_L06_002AA500[];

extern void func_32D268(char *);
extern void func_366160(char *, char *, float, int, int);
extern void func_36C158();
extern void func_36C1B0(float);
extern void func_3BFBB8(char *, int);

INCLUDE_ASM("asm/levels", func_L06_0041DED8);

INCLUDE_ASM("asm/levels", func_L06_0041EF08);

INCLUDE_ASM("asm/levels", func_L06_0041F2A0);

INCLUDE_ASM("asm/levels", func_L06_0041F558);

INCLUDE_ASM("asm/levels", func_L06_0041F798);

INCLUDE_ASM("asm/levels", func_L06_0041F818);

INCLUDE_ASM("asm/levels", func_L06_0041FAF8);

INCLUDE_ASM("asm/levels", func_L06_0041FC58);

INCLUDE_ASM("asm/levels", func_L06_0041FD60);

INCLUDE_ASM("asm/levels", func_L06_0041FD90);

INCLUDE_ASM("asm/levels", func_L06_00420080);

INCLUDE_ASM("asm/levels", func_L06_00420230);

INCLUDE_ASM("asm/levels", func_L06_00420548);

INCLUDE_ASM("asm/levels", func_L06_00420740);

INCLUDE_ASM("asm/levels", func_L06_00420AE8);

INCLUDE_ASM("asm/levels", func_L06_0042B868);

INCLUDE_ASM("asm/levels", func_L06_00430318);

INCLUDE_ASM("asm/levels", func_L06_00430C60);

INCLUDE_ASM("asm/levels", func_L06_00443280);

INCLUDE_ASM("asm/levels", func_L06_00443A38);

void func_L06_00443DE0(char *a0, int a1, int a2)
{
    char *p = *(char **)(a0 + 0x68);

    if (a2 == 0) {
        func_36C158();
    } else {
        func_36C1B0(*(float *)(p + 0x31C));
    }
}

void func_L06_00443E18(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    unsigned int v;

    func_32D268(s1 + 0x310);
    func_32D268(s1 + 0x312);
    v = *(unsigned short *)(a0 + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(a0 + 0x96) = v + 1;
    func_366160(a0, s1 + 0x260, 2.5f, 0, 1);
}

INCLUDE_ASM("asm/levels", func_L06_00443E88);

INCLUDE_ASM("asm/levels", func_L06_00443F30);

INCLUDE_ASM("asm/levels", func_L06_00444068);
INCLUDE_ASM("asm/levels", func_L06_00444168);

INCLUDE_ASM("asm/levels", func_L06_00444300);

INCLUDE_ASM("asm/levels", func_L06_0044B250);

INCLUDE_ASM("asm/levels", func_L06_0044B2D8);

INCLUDE_ASM("asm/levels", func_L06_0044B350);

INCLUDE_ASM("asm/levels", func_L06_004504C8);

INCLUDE_ASM("asm/levels", func_L06_00455BC0);

INCLUDE_ASM("asm/levels", func_L06_00456278);

INCLUDE_ASM("asm/levels", func_L06_00456BB0);

void func_L06_00456EE0(void)
{
    func_3BFBB8(D_L06_002AA500, 0x30);
}

INCLUDE_ASM("asm/levels", func_L06_00456F08);

INCLUDE_ASM("asm/levels", func_L06_00457100);

INCLUDE_ASM("asm/levels", func_L06_00457270);

INCLUDE_ASM("asm/levels", func_L06_004573E8);

INCLUDE_ASM("asm/levels", func_L06_004576C8);

INCLUDE_ASM("asm/levels", func_L06_004577B0);

INCLUDE_ASM("asm/levels", func_L06_00457920);

INCLUDE_ASM("asm/levels", func_L06_004579C8);

INCLUDE_ASM("asm/levels", func_L06_00457AD0);

INCLUDE_ASM("asm/levels", func_L06_00457C00);

INCLUDE_ASM("asm/levels", func_L06_00457DA0);

INCLUDE_ASM("asm/levels", func_L06_00457ED0);
