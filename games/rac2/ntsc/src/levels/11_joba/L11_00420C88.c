#include "include_asm.h"

typedef signed char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

INCLUDE_ASM("asm/levels", func_L11_00420C88);

INCLUDE_ASM("asm/levels", func_L11_00424A60);

INCLUDE_ASM("asm/levels", func_L11_00425788);

extern void func_003315A0(char *object, char *p);
extern void func_00300180(char *p);
extern void func_003357E0(char *object, char *p, int kind, float f12, float f13);
extern void func_00338398(char *object, char *p, int kind, int a3, float f12);

/* re-arms the object's two guard nodes and restarts its measurement window */
void func_L11_00425A50(char *object)
{
    char *p = *(char **)(object + 0x68);

    func_003315A0(object, p + 0x50);
    func_00300180(p + 0x260);
    func_00300180(p + 0x26);
    if (*(u16 *)(object + 0x96) <= 0xFFFE)
        *(u16 *)(object + 0x96) = *(u16 *)(object + 0x96) + 1;
    func_003357E0(object, p + 0x2A0, 4, 0.03f, 0.3f);
    func_00338398(object, p + 0x2A0, 4, 1, 3.5f);
}

extern char *D_L11_001FC420[] __attribute__((sda));
extern int D_L11_001B359C __attribute__((sda));

/* walks the object's linked entry list and bumps the hit counters on the two kinds it watches */
INCLUDE_ASM("asm/levels", func_L11_00425AF8);

extern char *func_0032F000(char *p, int a1, int a2);

/* ticks the object's clock down by the rate the linked node states */
int func_L11_00425EC0(char *object)
{
    char *p = *(char **)(object + 0x68);
    char *r = func_0032F000(*(char **)(p + 0x280), 0x10000, 0);

    if (r != 0) {
        *(float *)(p + 0x20) = *(float *)(p + 0x20) - *(float *)(r + 0x2C);
        return 1;
    }
    return 0;
}
