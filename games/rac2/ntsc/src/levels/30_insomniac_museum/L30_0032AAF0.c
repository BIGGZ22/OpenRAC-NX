#include "include_asm.h"

extern char D_00189EA0[];
extern char D_00233A60[];
extern char D_00235560[];

extern void func_00115484(char *, int, int);
extern int func_002ACBC8(char *);
extern void func_002A7470(char *, int, int, int);
extern void func_L00_0042A500(int);

INCLUDE_ASM("asm/levels", func_L30_0032AAF0);

INCLUDE_ASM("asm/levels", func_L30_00342FF8);

INCLUDE_ASM("asm/levels", func_L30_003557F0);

float func_L30_00382EF0(float a0, float a1, float a2) {
    return a0 * (1.0f - a2) + a1 * a2;
}

INCLUDE_ASM("asm/levels", func_L30_00382F10);

INCLUDE_ASM("asm/levels", func_L30_00382FB8);

INCLUDE_ASM("asm/levels", func_L30_003830D8);

INCLUDE_ASM("asm/levels", func_L30_003831E8);

INCLUDE_ASM("asm/levels", func_L30_00383398);

INCLUDE_ASM("asm/levels", func_L30_00383928);

INCLUDE_ASM("asm/levels", func_L30_003840E0);

INCLUDE_ASM("asm/levels", func_L30_003842E0);

INCLUDE_ASM("asm/levels", func_L30_003844C8);

INCLUDE_ASM("asm/levels", func_L30_00384768);

INCLUDE_ASM("asm/levels", func_L30_003847F8);

INCLUDE_ASM("asm/levels", func_L30_003848B8);

INCLUDE_ASM("asm/levels", func_L30_00384980);

INCLUDE_ASM("asm/levels", func_L30_00386300);

INCLUDE_ASM("asm/levels", func_L30_0038E7E0);

INCLUDE_ASM("asm/levels", func_L30_0038EBF8);

INCLUDE_ASM("asm/levels", func_L30_003941D8);

INCLUDE_ASM("asm/levels", func_L30_003B3C18);

INCLUDE_ASM("asm/levels", func_L30_003B3E60);

INCLUDE_ASM("asm/levels", func_L30_003B3F70);

INCLUDE_ASM("asm/levels", func_L30_003BE478);

void func_L30_003BE678(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        if (*(unsigned char *)(a0 + 0x43) != 1) {
            func_002A7470(a0, 1, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 2;
    } else {
        if (*(unsigned char *)(a0 + 0x43) != 2) {
            func_002A7470(a0, 2, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 5;
        if (*(int *)(st + 0xC) != -1) {
            func_L00_0042A500(*(int *)(st + 0xC));
        }
    }
}

INCLUDE_ASM("asm/levels", func_L30_003C7180);

INCLUDE_ASM("asm/levels", func_L30_003D9CA8);

INCLUDE_ASM("asm/levels", func_L30_003D9E60);

INCLUDE_ASM("asm/levels", func_L30_003DA040);

INCLUDE_ASM("asm/levels", func_L30_003E1BA0);

INCLUDE_ASM("asm/levels", func_L30_003E1E78);

INCLUDE_ASM("asm/levels", func_L30_00404608);

INCLUDE_ASM("asm/levels", func_L30_0040AE38);

INCLUDE_ASM("asm/levels", func_L30_0040B4E0);

INCLUDE_ASM("asm/levels", func_L30_0040B6B0);

INCLUDE_ASM("asm/levels", func_L30_0040B9D8);

INCLUDE_ASM("asm/levels", func_L30_0040C270);
