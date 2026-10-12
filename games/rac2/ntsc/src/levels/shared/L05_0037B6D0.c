#include "include_asm.h"

extern int func_002ED688(char *);

extern int func_L05_0037CEB8(char *);

extern void func_002EE678(char *, int);

extern int func_002A7820_1(int, int);

extern int func_001163B0(void);

extern int D_L05_00221D10[];

/* looks up an object, then sets flags, arms a stored value and calls one of two helper pairs before building a timer */
void func_L05_0037B6D0(char *obj)
{
    char *s;

    s = (char *)func_002ED688(obj);
    if (s == 0)
        return;
    if (*(int *)(s + 288) == 0)
        return;
    if (*(int *)(s + 284) != 0) {
        int *p = D_L05_00221D10;
        int one = 1;

        *(int *)(s + 280) = one;
        p[3] += one;
        if (func_L05_0037CEB8(obj) != 5) {
            func_002EE678(obj, 4);
            *(float *)(s + 120) = (func_001163B0() & 1) ? -1.0f : 1.0f;
            if (func_001163B0() & 1)
                *(float *)(s + 116) = -1.0f;
            else
                *(float *)(s + 116) = 1.0f;
            *(int *)(s + 252) = one;
            *(int *)(s + 112) = 0;
            *(int *)(s + 124) = 0;
        } else {
            func_002EE678(obj, 7);
        }
        *(int *)(s + 304) = func_002A7820_1(120, 500);
    } else {
        int *p = D_L05_00221D10;

        *(int *)(s + 280) = 1;
        p[3] += 1;
        if (func_L05_0037CEB8(obj) != 5)
            func_002EE678(obj, 4);
        else
            func_002EE678(obj, 7);
        *(int *)(s + 304) = func_002A7820_1(120, 500);
        *(int *)(s + 252) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L05_0037B888);

INCLUDE_ASM("asm/levels", func_L05_0037B948);

INCLUDE_ASM("asm/levels", func_L05_0037BB28);

INCLUDE_ASM("asm/levels", func_L05_0037BB78);

INCLUDE_ASM("asm/levels", func_L05_0037BB90);

INCLUDE_ASM("asm/levels", func_L05_0037BBB8);

INCLUDE_ASM("asm/levels", func_L05_0037BC08);

INCLUDE_ASM("asm/levels", func_L05_0037BD40);

INCLUDE_ASM("asm/levels", func_L05_0037BE98);

INCLUDE_ASM("asm/levels", func_L05_0037BF48);

INCLUDE_ASM("asm/levels", func_L05_0037C160);

INCLUDE_ASM("asm/levels", func_L05_0037CDD8);

INCLUDE_ASM("asm/levels", func_L05_0037CE08);

INCLUDE_ASM("asm/levels", func_L05_0037CEB8);

INCLUDE_ASM("asm/levels", func_L05_0037CEE0);

INCLUDE_ASM("asm/levels", func_L05_0037D608);

INCLUDE_ASM("asm/levels", func_L05_0037D6E8);

INCLUDE_ASM("asm/levels", func_L05_0037D768);

INCLUDE_ASM("asm/levels", func_L05_0037D7D8);

INCLUDE_ASM("asm/levels", func_L05_0037DA58);

INCLUDE_ASM("asm/levels", func_L05_003979B0);

INCLUDE_ASM("asm/levels", func_L05_00397B30);

INCLUDE_ASM("asm/levels", func_L05_00397F48);

INCLUDE_ASM("asm/levels", func_L05_00398018);

INCLUDE_ASM("asm/levels", func_L05_00398378);

INCLUDE_ASM("asm/levels", func_L05_00398618);

INCLUDE_ASM("asm/levels", func_L05_00398A38);

INCLUDE_ASM("asm/levels", func_L05_00398F18);

INCLUDE_ASM("asm/levels", func_L05_00399268);

INCLUDE_ASM("asm/levels", func_L05_003CD1A8);

INCLUDE_ASM("asm/levels", func_L05_003CD280);

INCLUDE_ASM("asm/levels", func_L05_003CD4C0);

INCLUDE_ASM("asm/levels", func_L05_003CD598);

INCLUDE_ASM("asm/levels", func_L05_003CEFA0);

INCLUDE_ASM("asm/levels", func_L05_003CF040);

INCLUDE_ASM("asm/levels", func_L05_003CF350);

INCLUDE_ASM("asm/levels", func_L05_003CF9C8);

INCLUDE_ASM("asm/levels", func_L05_003CFAB8);

INCLUDE_ASM("asm/levels", func_L05_003CFB90);

INCLUDE_ASM("asm/levels", func_L05_003CFC28);

INCLUDE_ASM("asm/levels", func_L05_003CFC48);

INCLUDE_ASM("asm/levels", func_L05_003CFCF0);

INCLUDE_ASM("asm/levels", func_L05_003CFF90);

INCLUDE_ASM("asm/levels", func_L05_003D02C8);

INCLUDE_ASM("asm/levels", func_L05_003D0320);

INCLUDE_ASM("asm/levels", func_L05_003D0420);

INCLUDE_ASM("asm/levels", func_L05_003D0538);

INCLUDE_ASM("asm/levels", func_L05_003D0728);

INCLUDE_ASM("asm/levels", func_L05_003D08C0);

INCLUDE_ASM("asm/levels", func_L05_003D0960);

INCLUDE_ASM("asm/levels", func_L05_003D0B20);

INCLUDE_ASM("asm/levels", func_L05_003D1058);

INCLUDE_ASM("asm/levels", func_L05_003D1148);

INCLUDE_ASM("asm/levels", func_L05_003D1368);

INCLUDE_ASM("asm/levels", func_L05_003D15E8);

INCLUDE_ASM("asm/levels", func_L05_003D18E0);

INCLUDE_ASM("asm/levels", func_L05_003D19F0);

INCLUDE_ASM("asm/levels", func_L05_003D1AB8);

INCLUDE_ASM("asm/levels", func_L05_003D1E20);

INCLUDE_ASM("asm/levels", func_L05_003D1E60);

INCLUDE_ASM("asm/levels", func_L05_003D2048);

INCLUDE_ASM("asm/levels", func_L05_003D2618);
