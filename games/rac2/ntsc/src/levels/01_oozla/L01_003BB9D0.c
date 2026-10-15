#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern int func_00282948(void *);

extern void func_00291450(int);

extern float func_002A7878(float, float);

extern void func_002A0F48(char *);

extern void *func_0029EFD0(int);

extern float func_00283180(float);

extern float func_00283198(float);

extern void func_00282CC0(void *, void *, void *);

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(void *, void *, float);

extern char D_00189EA0[];

extern void func_L01_003BDBE8(void *);

extern float func_002A7940(void);

extern int func_002A7820(int, int);

extern void func_L01_0032EF38(void *, int, int, int, float, float);

typedef struct L01_003BDF28_BUF {
    float x;
    float y;
    int z;
} L01_003BDF28_BUF;

void func_L01_003BDF28(char *a0)
{
    char *s2 = a0;
    char *s0 = *(char **)(s2 + 0x68);
    char *s1 = s0 + 0x1E0;
    L01_003BDF28_BUF buf;

    *(float *)(s0 + 0x1E0) = func_00283180(func_002A7878(-3.1415927f, 3.1415927f)) * (float)*(unsigned char *)(s0 + 0x201);
    *(float *)(s0 + 0x1E4) = func_00283198(func_002A7878(-3.1415927f, 3.1415927f)) * (float)*(unsigned char *)(s0 + 0x201);
    *(int *)(s0 + 0x1E8) = 0;
    func_00282CC0(s1, s1, s0 + 0x1D0);
    *(float *)(s0 + 0x1E8) = *(float *)(s0 + 0x1E8) + 1.7f;
    if (*(int *)(s0 + 0x23C) != 0) {
        func_00282CF0(&buf, s1, D_00189EA0);
        buf.z = 0;
        func_00282F20(&buf, &buf, 8.0f);
        func_00282CC0(s1, s1, &buf);
    }
    func_L01_003BDBE8(s2);
}

typedef struct L01_003BC490_OBJ {
    char pad[0x10];
    TI128 f10;
} L01_003BC490_OBJ;


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
