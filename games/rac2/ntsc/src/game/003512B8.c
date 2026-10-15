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

/* func_00351720 is matched in 00351720.c. */

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

extern void func_00350B70(void *);

void func_00351868(char *object) {
    func_00350B70(object + 0x48);
}

/* Measured target: byte-identical across all 27 levels. */
void func_00351888(u8 *a0) { *(s32 *)(a0 + 0xa8) = 0; }

extern void func_00351210(void *);
extern int func_0012F940(void *);

int func_00351890(u8 *a0) {
    func_00351210(a0 + 0x48);
    func_0012F940(a0);
    return 1;
}

void func_003518C8(u8 *a0) {s32 v=1;*(s32 *)(a0+0xa8)=v;}

/* Measured target: byte-identical across all 27 levels. */
s32 func_003518D8(u8 *a0) { return *(s32 *)(a0 + 0xa8); }

s32 func_003518E0(u8 *a0, s32 a1) { s32 vieux = *(s32 *)(a0 + 0xa8); *(s32 *)(a0 + 0xa8) = a1; return vieux; }

typedef struct { unsigned long long first, second; int offset, count; } Transfer_3518F0;
extern char *D_001B23CC[] __attribute__((sda));
extern int func_00351468(void *, Transfer_3518F0 *, int);

int func_003518F0(char *object, unsigned long long first, unsigned long long second, int end, int count)
{
    Transfer_3518F0 transfer;
    transfer.first = first;
    transfer.second = second;
    transfer.offset = end - *(int *)(object + 0x48);
    transfer.count = count;
    return func_00351468(D_001B23CC[0] + 0xD9090, &transfer, *(int *)(object + 0x48));
}

int func_00351938(unsigned char *owner)
{
    return func_00351268(owner + 0x48);
}

INCLUDE_ASM("asm/game", func_00351958);

INCLUDE_ASM("asm/game", func_00351960);

extern int func_0012F9B8(void *);



extern void func_00350918(void *);
extern void func_00351E48(u8 *);
extern void func_00351B20(void *);
void func_00351A80(unsigned char *object)
{
    char *base;
    func_00350918(object + 0x48);
    func_00351E48(D_001B23CC[0] + 0xD9168);
    func_00351B20(object);
    base = D_001B23CC[0];
    while (*(int *)(base + 0xD9174)) {
        if (func_003518D8(object) == 1) break;
        base = D_001B23CC[0];
    }
    func_003518E0(object, 3);
}


INCLUDE_ASM("asm/game", func_00351B20);

extern char D_001AE8A8[];
extern void func_0026FEB8(void *, int);

int func_00351CD8(void *unused, int *object) {
    func_0026FEB8(D_001AE8A8, object[1]);
    return 1;
}

int func_00351D00(void)
{
    extern void func_0034F3B8(void);
    extern char *D_001B23CC[] __attribute__((sda));
    extern void func_00350BC8(void *);
    func_0034F3B8();
    func_00350BC8(D_001B23CC[0] + 0xD9090);
    return 1;
}

extern char *D_001B23CC[] __attribute__((sda));
extern void func_00350DC8(void *);

int func_00351D38(void) {
    func_00350DC8(D_001B23CC[0] + 0xD9090);
    return 1;
}

extern char *D_001B23CC[] __attribute__((sda));
extern void func_00350ED8(void *);

int func_00351D68(void) {
    func_00350ED8(D_001B23CC[0] + 0xD9090);
    return 1;
}

extern char *D_001B23CC[] __attribute__((sda));
extern void func_00351578(void *, void *);

int func_00351D98(void *a0, void *a1) {
    unsigned long long result[4];
    func_00351578(D_001B23CC[0] + 0xD9090, result);
    *(unsigned long long *)((u8 *)a1 + 8) = result[0];
    *(unsigned long long *)((u8 *)a1 + 0x10) = result[1];
    return 1;
}

void func_00351DE8(volatile s32 *a0, s32 a1, s32 a2, s32 a3) {
    s32 i;
    a0[3] = 0;
    i = 0;
    a0[0] = a1;
    a0[1] = a2;
    a0[4] = a3;
    a0[2] = 0;
    if (a3 > 0) {
        a2 = 0x138C0;
        a1 = 0;
        do {
            *(s32 *)(a1 + a0[1]) = 0;
            *(s32 *)(a1 + a0[1] + 4) = i;
            i++;
            a1 += a2;
        } while (i < a3);
    }
}

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
