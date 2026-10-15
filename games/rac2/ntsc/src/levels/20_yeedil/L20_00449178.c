#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_002A7750(char *, int, int);

extern void func_0032C700(void *a0, int a1, int a2, int a3, int a4, int a5, int a6,
                          int a7, float f12, float f13, int s1, int s2, int s3,
                          int s4);

extern void func_002A8C28(char *, char *, int, int, float, float, float, int, int);

extern void func_0029F338(char *object);

INCLUDE_ASM("asm/levels", func_L20_00449178);

INCLUDE_ASM("asm/levels", func_L20_00449338);

INCLUDE_ASM("asm/levels", func_L20_00449850);

/* func_L20_004499E8 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

u32 func_L20_00449AD8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    return *(u32 *)(state + 0xAC);
}

void func_L20_00449A78(char *arg0) {
    *(u16 *)(arg0 + 0x34) &= 0xEFFF;
    func_002A7750(arg0, 1, -1);
    *(int *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
}

/* func_L20_00449AE8 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

/* func_L20_00449BB0 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L20_00449CE0);

/* func_L20_00449FD0 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

void func_L20_0044A108(char *arg0, char *arg1) {
    func_0032C700(arg0 + 0x10, 0, 5, 4, 4, 4, 7, 0, 0.333f, 20.0f, 0, 0, 0, 0);
    if (arg1 != 0) {
        func_002A8C28(*(char **)(arg0 + 0xB8), arg0 + 0x10, (int)arg1, 0, 1.5f,
                      4.0f, 1.0f, 1, 0);
    }
    func_0029F338(arg0);
}

INCLUDE_ASM("asm/levels", func_L20_0044A1C8);

INCLUDE_ASM("asm/levels", func_L20_0044A658);

/* func_L20_0044A838 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L20_0044A8F0);

/* func_L20_0044AA88 is defined in L20_00449178-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L20_0044AB68);

INCLUDE_ASM("asm/levels", func_L20_0044AC38);

INCLUDE_ASM("asm/levels", func_L20_0044AE58);

INCLUDE_ASM("asm/levels", func_L20_0044B100);
