#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

extern s32 func_001253A8(s32, s32, u8 *, u8 *);

extern s32 func_00124B88(s32);

typedef unsigned int u32;

extern s32 func_0011F5E0(void);

extern s32 func_0011F628(void);

extern void F0b028b34_FUN_00350878(char *a, unsigned int b, int c, int d);

extern void F0b028b34_FUN_00350808(int a);

/* func_003512B8 : rounds a field to a multiple of 2048 under a semaphore */
extern s32 func_0011AC60(s32);

extern s32 func_0011AC40(s32);

/* func_00351268 : waits, then combines two fields shifted by 11 */
extern s32 func_0011AC60(s32 a0);

extern s32 func_0011AC40(s32 a0);

extern void func_00350A78(int *owner, int *first_address, int *first_count,
                         int *second_address, int *second_count);

s32 func_003506B0(u8 *a0, u8 *a1, s32 a2, s32 a3) {
    u8 mode[3];
    s32 sectors = a2 >> 11;
    s32 out = 0;
    mode[0] = 100; mode[1] = 1; mode[2] = 0;
    func_001253A8(*(s32 *)(a0+4), sectors, a1, mode);
    if (a3 == 0) {
        *(s32 *)(a0+4) += sectors;
        func_00124B88(0);
        out = a2;
    }
    return out;
}

s32 func_00350750(u8 *a2, s32 a1) {
    u32 v1 = ((*(u32 *)(a2 + 8) << 4) + *(u32 *)(a2 + 4) + 0x10) & 0x0FFFFFFF;
    if (a1 == v1) return 0;
    return (u32)(a1 - *(u32 *)a2) >> 11;
}

void func_00350798(u32 a0) {
    func_0011F5E0();
    *(volatile u32 *)0x1000f590 = *(volatile u32 *)0x1000f520 | 0x10000;
    *(volatile u32 *)0x1000b000 = a0;
    *(volatile u32 *)0x1000f590 = *(volatile u32 *)0x1000f520 & 0xfffeffff;
    func_0011F628();
}

void func_00350808(u32 a0) {
    func_0011F5E0();
    *(volatile u32 *)0x1000f590 = *(volatile u32 *)0x1000f520 | 0x10000;
    *(volatile u32 *)0x1000b400 = a0;
    *(volatile u32 *)0x1000f590 = *(volatile u32 *)0x1000f520 & 0xfffeffff;
    func_0011F628();
}

void func_00350878(unsigned long *a0,unsigned long a1,unsigned long a2,unsigned long a3) {
 *a0=(a1<<32)|((a2<<32)>>4)|((a3<<32)>>32);
}

extern int func_0011AC20(int *);
extern int func_00350918(int *);

int func_003508A0(int *object, int address, unsigned int buffer, int count, int *records, int capacity)
{
    int semaphore[5];
    object[0] = address;
    object[1] = (buffer & 0x0FFFFFFF) | 0x20000000;
    object[20] = (int)records;
    object[21] = capacity;
    object[2] = count;
    object[6] = count << 11;
    semaphore[2] = 1;
    semaphore[1] = 1;
    object[16] = func_0011AC20(semaphore);
    func_00350918(object);
    *(long long *)(object + 18) = 0;
    return 1;
}

int func_00350918(int *p)
{
    int i;

    p[17] = 1;
    p[3] = 0;
    p[4] = 0;
    p[5] = 0;
    p[22] = 0;
    p[23] = 0;
    for (i = 0; i < p[21]; i++) {
        *(long long *)((char *)p[20] + i * 24) = -1;
        *(long long *)((char *)p[20] + i * 24 + 8) = -1;
        *(int *)((char *)p[20] + i * 24 + 16) = 0;
        *(int *)((char *)p[20] + i * 24 + 20) = 0;
    }
    for (i = 0; i < p[2]; i++) {
        F0b028b34_FUN_00350878((char *)(p[1] + i * 16), (i * 2048 + p[0]) & 0x0FFFFFFF, 3, 128);
    }
    F0b028b34_FUN_00350878((char *)(p[1] + i * 16), p[1] & 0x0FFFFFFF, 2, 0);
    *(volatile unsigned int *)0x1000B420 = 0;
    *(volatile unsigned int *)0x1000B410 = p[0] & 0x0FFFFFFF;
    *(volatile unsigned int *)0x1000B430 = p[1] & 0x0FFFFFFF;
    F0b028b34_FUN_00350808(5);
    return 1;
}

void func_00350A78(s32 *a0, s32 *a1, s32 *a2, s32 *a3, s32 *a4) {
    s32 pos, left;
    func_0011AC60(a0[16]);
    left = (s32)((long)a0[2] - (long)(a0[4]+2)) * 2048 - a0[5];
    pos = ((a0[3] + a0[4]) * 2048 + a0[5]) % a0[6];
    if (left <= a0[6]-pos) {
        *a1 = a0[0]+pos;
        *a2 = left;
        *a3 = 0;
        *a4 = 0;
    } else {
        *a1 = a0[0]+pos;
        *a2 = a0[6]-pos;
        *a3 = a0[0];
        *a4 = left - (a0[6]-pos);
    }
    func_0011AC40(a0[16]);
}
