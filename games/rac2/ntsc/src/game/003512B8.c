#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

/* func_003512B8 : rounds a field to a multiple of 2048 under a semaphore */
extern s32 func_0011AC60(s32);

extern s32 func_0011AC40(s32);

/* func_00351268 : waits, then combines two fields shifted by 11 */
extern s32 func_0011AC60(s32 a0);

extern s32 func_0011AC40(s32 a0);

extern void func_0012EE28();

extern void func_00350A78(int *owner, int *first_address, int *first_count,
                         int *second_address, int *second_count);

extern int func_00351268(unsigned char *owner);

typedef unsigned int u32;

extern int func_00351E58(unsigned char *);

extern s32 func_0011F5E0(void);

extern s32 func_0011F628(void);

void func_003512B8(u8 *a0) {
    func_0011AC60(*(s32 *)(a0 + 0x40));
    *(s32 *)(a0 + 0x14) = (*(s32 *)(a0 + 0x14) + 0x7ff) / 0x800 * 0x800;
    func_0011AC40(*(s32 *)(a0 + 0x40));
}

INCLUDE_ASM("asm/game", func_00351310);

INCLUDE_ASM("asm/game", func_00351468);

INCLUDE_ASM("asm/game", func_00351578);

INCLUDE_ASM("asm/game", func_00351720);

INCLUDE_ASM("asm/game", func_00351820);

s32 func_00351828(void) {
    func_0012EE28();
    return 1;
}

void func_00351848(unsigned char *owner, int *first_address, int *first_count,
                  int *second_address, int *second_count)
{
    func_00350A78((int *)(owner + 0x48), first_address, first_count,
                 second_address, second_count);
}

INCLUDE_ASM("asm/game", func_00351868);

/* Measured target: byte-identical across all 27 levels. */
void func_00351888(u8 *a0) { *(s32 *)(a0 + 0xa8) = 0; }

INCLUDE_ASM("asm/game", func_00351890);

void func_003518C8(u8 *a0) {s32 v=1;*(s32 *)(a0+0xa8)=v;}

/* Measured target: byte-identical across all 27 levels. */
s32 func_003518D8(u8 *a0) { return *(s32 *)(a0 + 0xa8); }

s32 func_003518E0(u8 *a0, s32 a1) { s32 vieux = *(s32 *)(a0 + 0xa8); *(s32 *)(a0 + 0xa8) = a1; return vieux; }

INCLUDE_ASM("asm/game", func_003518F0);

int func_00351938(unsigned char *owner)
{
    return func_00351268(owner + 0x48);
}

INCLUDE_ASM("asm/game", func_00351958);

INCLUDE_ASM("asm/game", func_00351960);

INCLUDE_ASM("asm/game", func_00351A38);

INCLUDE_ASM("asm/game", func_00351A80);

INCLUDE_ASM("asm/game", func_00351B20);

INCLUDE_ASM("asm/game", func_00351CD8);

INCLUDE_ASM("asm/game", func_00351D00);

INCLUDE_ASM("asm/game", func_00351D38);

INCLUDE_ASM("asm/game", func_00351D68);

INCLUDE_ASM("asm/game", func_00351D98);

INCLUDE_ASM("asm/game", func_00351DE8);

void func_00351E40(void)
{
}

void func_00351E48(u8 *a0) {
    volatile s32 *p = (volatile s32 *)a0;
    p[3] = 0;
    p[2] = 0;
}

/* lot13 -- measured bodies with sizes recorded in the catalogue. */
s32 func_00351E58(u8 *a0) {
    return (*(u32 *)(a0 + 0xC) ^ *(u32 *)(a0 + 0x10)) < 1;
}

void func_00351E70(u8 *a0) {
    func_0011F5E0();
    *(s32 *)(*(u8 **)(a0+4) + *(s32 *)(a0+8) * 0x138c0) = 2;
    *(volatile s32 *)(a0+0xc) += 1;
    *(volatile s32 *)(a0+8) = (*(volatile s32 *)(a0+8) + 1) % *(s32 *)(a0+0x10);
    func_0011F628();
}
