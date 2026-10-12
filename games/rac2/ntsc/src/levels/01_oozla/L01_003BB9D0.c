#include "include_asm.h"

extern int func_00282948(void *);

extern void func_00291450(int);


extern void func_002A7470();

INCLUDE_ASM("asm/levels", func_L01_003BB9D0);

INCLUDE_ASM("asm/levels", func_L01_003BBF68);

INCLUDE_ASM("asm/levels", func_L01_003BC490);

INCLUDE_ASM("asm/levels", func_L01_003BC540);

INCLUDE_ASM("asm/levels", func_L01_003BCA38);

INCLUDE_ASM("asm/levels", func_L01_003BCEC8);

INCLUDE_ASM("asm/levels", func_L01_003BD150);

INCLUDE_ASM("asm/levels", func_L01_003BD410);

INCLUDE_ASM("asm/levels", func_L01_003BD560);

INCLUDE_ASM("asm/levels", func_L01_003BD5E8);

INCLUDE_ASM("asm/levels", func_L01_003BD6B0);

INCLUDE_ASM("asm/levels", func_L01_003BDB28);

/* func_L01_003BDBB0 */
typedef struct L01_003BDBB0_OWNED {
    char pad0[0x250];
    void *f250;
    int f254;
} L01_003BDBB0_OWNED;

typedef struct L01_003BDBB0_OBJ {
    char pad0[0x68];
    L01_003BDBB0_OWNED *f68;
} L01_003BDBB0_OBJ;

void func_L01_003BDBB0(L01_003BDBB0_OBJ *a0)
{
    L01_003BDBB0_OWNED *p = a0->f68;

    if (p->f254 != 0) {
        func_00282948(p->f250);
        p->f254 = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003BDBE8);

INCLUDE_ASM("asm/levels", func_L01_003BDF28);

INCLUDE_ASM("asm/levels", func_L01_003BE058);

/* func_L01_003BE488 */
typedef struct L01_003BE488_OWNED {
    char pad0[0x202];
    short f202;
    int f204;
} L01_003BE488_OWNED;

typedef struct L01_003BE488_OBJ {
    char pad0[0x68];
    L01_003BE488_OWNED *f68;
} L01_003BE488_OBJ;

void func_L01_003BE488(L01_003BE488_OBJ *a0)
{
    L01_003BE488_OWNED *p = a0->f68;

    if (p->f202 != -1) {
        p->f204 = 0;
        func_00291450(p->f202);
        p->f202 = -1;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003BE4D0);

INCLUDE_ASM("asm/levels", func_L01_003BE628);

INCLUDE_ASM("asm/levels", func_L01_003BEA08);

/* func_L01_003BFDC8 */
typedef struct L01_003BFDC8_OBJ {
    char pad0[0x43];
    unsigned char f43;
    char pad1[0x48 - 0x44];
    float f48;
} L01_003BFDC8_OBJ;

void func_L01_003BFDC8(L01_003BFDC8_OBJ *a0, int a1, int a2, int a3)
{
    L01_003BFDC8_OBJ *p = a0;

    if (p->f43 != a1) {
        func_002A7470(a0);
    }
    if (a1 != 12) {
        p->f48 = 0.6666667f;
    }
    if ((unsigned int)(a1 - 6) < 3) {
        p->f48 = p->f48 * 1.25f;
    }
}

INCLUDE_ASM("asm/levels", func_L01_003BFE48);


INCLUDE_ASM("asm/levels", func_L01_003C01C0);

INCLUDE_ASM("asm/levels", func_L01_003C0408);

INCLUDE_ASM("asm/levels", func_L01_003C08A0);


INCLUDE_ASM("asm/levels", func_L01_003C09C0);

INCLUDE_ASM("asm/levels", func_L01_003C2680);

INCLUDE_ASM("asm/levels", func_L01_003C53D8);

INCLUDE_ASM("asm/levels", func_L01_003C97D8);

INCLUDE_ASM("asm/levels", func_L01_003C9D00);
