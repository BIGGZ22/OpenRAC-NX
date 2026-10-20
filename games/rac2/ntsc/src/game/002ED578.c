#include "include_asm.h"

extern float func_00283CE0_6(int a0);

extern int func_00283CF0_9(float x);

typedef int s32;

typedef unsigned char u8;

/* func_002EE6A8: owner ABI/declaration review retained. */
extern int func_002ED688(unsigned char *owner);

/* func_002ED870: owner ABI/declaration review retained. */
extern unsigned int D_00214FCC;

/* Decomposes a float returned by a bound producer into hours, minutes and hundredths written through three pointer arguments, rounding each step with a bound helper and clamping the last field at 99. */
int func_002ED578(int a0, int *p0, int *p1, int *p2)
{
    float t;
    int a;
    int b;
    int c;
    int d;
    int e;

    t = func_00283CE0_6(a0);
    a = func_00283CF0_9(t * (1.0f / 3600.0f));
    *p0 = a;
    b = func_00283CF0_9((float)a * 3600.0f);
    t = t - (float)b;
    c = func_00283CF0_9(t * 0.01666667f);
    *p1 = c;
    d = func_00283CF0_9((float)c * 60.0f);
    t = t - (float)d;
    e = func_00283CF0_9(t * (5.0f / 3.0f));
    e = e > 99 ? 0 : e;
    *p2 = e;
    return e;
}

INCLUDE_ASM("asm/game", func_002ED670);

s32 func_002ED688(u8 *a0) {
 if((*(unsigned short *)(a0+0x34)&0x20)==0) return 0;
 return (*(s32 **)(a0+0x68))[5];
}

INCLUDE_ASM("asm/game", func_002ED6B0);

INCLUDE_ASM("asm/game", func_002ED848);

unsigned int func_002ED870(void)
{
    return D_00214FCC;
}

INCLUDE_ASM("asm/game", func_002ED880);


INCLUDE_ASM("asm/game", func_002ED8F0);

INCLUDE_ASM("asm/game", func_002ED948);

INCLUDE_ASM("asm/game", func_002ED9A8);

extern int D_00214F90[];

void func_002ED9C0(unsigned char *owner)
{
    char *object = (char *)func_002ED688(owner);
    if (object != 0 && *(int *)(object + 0x118) == 1) {
        *(int *)(object + 0x118) = 0;
        D_00214F90[3]--;
    }
}

INCLUDE_ASM("asm/game", func_002EDA10);

INCLUDE_ASM("asm/game", func_002EE000);

INCLUDE_ASM("asm/game", func_002EE238);

INCLUDE_ASM("asm/game", func_002EE3B0);
