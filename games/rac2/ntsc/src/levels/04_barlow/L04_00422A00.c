#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

INCLUDE_ASM("asm/levels", func_L04_00422A00);

extern u8 D_L04_001395C7;
extern u8 D_L04_002CA518;

extern void func_2CC1F0(void *a0);
extern void func_30EA08(void *a0);
extern void func_33B9B0(void *a0, s32 a1, s32 a2);
extern void func_3317E8(void *a0);
extern void func_340BB8(void *a0, void *a1);
extern void func_345328(void *a0, void *a1, s32 a2, f32 f12, f32 f13);
extern void func_348708(void *a0, void *a1, s32 a2, s32 a3, f32 f12);
extern int func_34C4C8(void);
extern void *func_33E6B0(void *a0, s32 a1, s32 a2);
extern void func_40A090(int a0, u8 *a1, u8 *a2, u8 *a3, u8 *a4, u8 *a5);

INCLUDE_ASM("asm/levels", func_L04_00422D40);
INCLUDE_ASM("asm/levels", func_L04_00422DA8);
INCLUDE_ASM("asm/levels", func_L04_00423258);
INCLUDE_ASM("asm/levels", func_L04_0042BF58);
INCLUDE_ASM("asm/levels", func_L04_00434BB0);
INCLUDE_ASM("asm/levels", func_L04_004389C0);
INCLUDE_ASM("asm/levels", func_L04_00438B50);
INCLUDE_ASM("asm/levels", func_L04_00439870);
INCLUDE_ASM("asm/levels", func_L04_00439950);
extern void func_40A090(int a0, u8 *a1, u8 *a2, u8 *a3, u8 *a4, u8 *a5);

extern u8 D_L04_001B12B0[] __attribute__((sda));
extern u8 D_L04_001B12E8[] __attribute__((sda));
extern u8 D_L04_001B1300[] __attribute__((sda));
extern u8 D_L04_001B1330[] __attribute__((sda));
extern u8 D_L04_001B1348[] __attribute__((sda));
extern u8 D_L04_001B3280[] __attribute__((sda));
extern u8 D_L04_001B32B0[] __attribute__((sda));
extern u8 D_L04_001B32E0[] __attribute__((sda));
extern u8 D_L04_001B3300[] __attribute__((sda));
extern u8 D_L04_001B3320[] __attribute__((sda));
extern u8 D_L04_001B3330[] __attribute__((sda));
extern u8 D_L04_001B3360[] __attribute__((sda));
extern u8 D_L04_001B3378[] __attribute__((sda));
extern u8 D_L04_001B33A0[] __attribute__((sda));
extern u8 D_L04_001B33B0[] __attribute__((sda));
extern u8 D_L04_001B33B8[] __attribute__((sda));
extern u8 D_L04_001B33D8[] __attribute__((sda));
extern u8 D_L04_001B33E8[] __attribute__((sda));
extern u8 D_L04_001B3418[] __attribute__((sda));
extern u8 D_L04_001B3430[] __attribute__((sda));
extern u8 D_L04_001B3458[] __attribute__((sda));
extern u8 D_L04_001B3468[] __attribute__((sda));
extern u8 D_L04_001B3470[] __attribute__((sda));
extern u8 D_L04_001B3474[] __attribute__((sda));
extern u8 D_L04_001B3478[] __attribute__((sda));
extern u8 D_L04_001B347C[] __attribute__((sda));
extern u8 D_L04_001B3480[] __attribute__((sda));
extern u8 D_L04_001B3484[] __attribute__((sda));
extern u8 D_L04_001B3488[] __attribute__((sda));
extern u8 D_L04_0026D370[] __attribute__((sda));
extern u8 D_L04_0026D3E0[] __attribute__((sda));
extern u8 D_L04_0026D490[] __attribute__((sda));
extern u8 D_L04_0026D4F0[] __attribute__((sda));
extern u8 D_L04_002CA3E0[] __attribute__((sda));
extern u8 D_L04_002CA438[] __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L04_0043ABE0);

INCLUDE_ASM("asm/levels", func_L04_0043AD48);
INCLUDE_ASM("asm/levels", func_L04_00441100);

void func_L04_00441178(u8 *a0) {
    if (a0[0x20] != 0)
        return;
    if (func_34C4C8() != 0) {
        func_2CC1F0(&D_L04_002CA518);
        D_L04_001395C7 = 1;
        func_33B9B0(a0, 1, -1);
    }
}
INCLUDE_ASM("asm/levels", func_L04_0044AAC8);
INCLUDE_ASM("asm/levels", func_L04_0044BC90);
INCLUDE_ASM("asm/levels", func_L04_0044C598);
INCLUDE_ASM("asm/levels", func_L04_0044D0C0);
INCLUDE_ASM("asm/levels", func_L04_0044D428);
INCLUDE_ASM("asm/levels", func_L04_0044D6A0);
INCLUDE_ASM("asm/levels", func_L04_0044D860);

/* subtracts the sub-structure's field from the owner's timer and reports it */
int func_L04_0044DAC8(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    char *r = func_33E6B0(*(char **)(s0 + 0x290), 0x10000, 0);

    if (r != 0) {
        *(f32 *)(s0 + 0x20) -= *(f32 *)(r + 0x2C);
        return 1;
    }
    return 0;
}
INCLUDE_ASM("asm/levels", func_L04_0044DB18);

void func_L04_00450268(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    void *p;

    p = *(void **)(s0 + 0x2AC);
    if (p != 0) {
        func_3317E8(p);
        *(void **)(s0 + 0x2AC) = 0;
    }
    p = *(void **)(s0 + 0x288);
    if (p != 0) {
        func_3317E8(p);
        *(void **)(s0 + 0x288) = 0;
    }
    p = *(void **)(s0 + 0x290);
    if (p != 0) {
        func_3317E8(p);
        *(void **)(s0 + 0x290) = 0;
    }
}
INCLUDE_ASM("asm/levels", func_L04_004502D0);
INCLUDE_ASM("asm/levels", func_L04_00450D50);

void func_L04_00451018(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    u16 v;

    func_340BB8(a0, s0 + 0x50);
    func_30EA08(s0 + 0x270);
    func_30EA08(s0 + 0x272);
    func_30EA08(s0 + 0x26);
    v = *(u16 *)(a0 + 0x96);
    if (v <= 0xFFFE) {
        *(u16 *)(a0 + 0x96) = v + 1;
    }
    s0 += 0x2C0;
    func_345328(a0, s0, 4, 0.03f, 0.3f);
    func_348708(a0, s0, 4, 1, 2.5f);
}
INCLUDE_ASM("asm/levels", func_L04_00455520);
INCLUDE_ASM("asm/levels", func_L04_00456628);
