#include "include_asm.h"

typedef signed int s32;
typedef signed short s16;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern s32 func_003062A0(u8 *a0);
extern s32 func_0030CBA8(s32 a0);
extern void func_00326198(s32 a0, s32 a1);
extern void func_002FF9F8(s32 a0);
extern void func_0037B210(void);
extern void func_003404B8(void);
extern void func_00379F18(s32 a0);
extern u8 **func_00379EB8(void);
extern s32 func_00379EC8(void);
extern void func_0037B280(u8 *a0, s32 a1);
extern void func_0037CEE0(u8 *a0, u8 *a1, f32 f12);
extern void func_0037D608(u8 *a0, u8 *a1, f32 f12);
extern s32 func_00379860(u8 *a0);
extern void func_00379E60(u8 *a0, s32 a1, s32 a2, s32 a3);
extern void func_0037B538(u8 *a0, s32 a1, s32 a2, s32 a3);
extern s32 func_0037B3A0(u8 *a0);
extern s32 func_00332AF8(s32 a0, s32 a1);
extern u32 *func_00379CA0(u8 *a0);
extern void func_0037AEC0(u8 *a0, s32 a1);
extern void func_00379C68(u8 *a0, s32 a1);
extern u8 *func_0037B440(s32 a0);
extern void func_0037D210(u8 *a0);
extern void func_00306748(u8 *a0, u8 *a1, u8 *a2);
extern void func_003068F8(u8 *a0, u8 *a1, f32 f12);
extern void func_00329960(u8 *a0, u8 *a1, u8 *a2);
extern void func_003298C0(u8 *a0, u8 *a1, u8 *a2);
extern void func_L05_00420788(u8 *a0, s32 a1);
extern void func_L05_004209D8(u8 *a0, s32 a1);
extern void func_L05_0041FC60(u8 *a0);
extern void func_L05_004224C8(u8 *a0);
extern void func_L05_00422540(u8 *a0);

extern s32 D_001B1364 __attribute__((sda));
extern s32 D_001B13E4 __attribute__((sda));
extern u8 D_L05_00221D10[];
extern u8 D_L05_001B3550[];
extern s32 D_L05_001B2048;
extern u8 *D_001B2614;
extern u8 D_L05_001B1354[];
extern u8 D_L05_001B1358[];
extern u8 D_L05_001B135C[];
extern u8 D_L05_001495B8[];
extern u8 *D_L05_0020B220[];
extern u8 *D_001B251C;
extern s32 D_001B1350 __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L05_00420360);

INCLUDE_ASM("asm/levels", func_L05_00420788);

/* func_L05_004209D8 lives in the -g8 sibling: its reset flag is reached through $gp. */

INCLUDE_ASM("asm/levels", func_L05_00420A88);

INCLUDE_ASM("asm/levels", func_L05_00420E28);

INCLUDE_ASM("asm/levels", func_L05_00420FB0);

INCLUDE_ASM("asm/levels", func_L05_00421100);

INCLUDE_ASM("asm/levels", func_L05_00421B98);

INCLUDE_ASM("asm/levels", func_L05_004222D8);

INCLUDE_ASM("asm/levels", func_L05_00422380);

void func_L05_004224C8(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);

    *(u16 *)(arg0 + 0x34) |= 0x100;
    arg0[0x20] = 1;
    *(s32 *)((u8 *)func_00379CA0(arg0) + 0x148) = 0xA;
    func_0037AEC0(arg0, 1);
    *(s32 *)(s1 + 0x26C) = 0;
    *(s32 *)(s1 + 0x268) = 0;
    *(f32 *)(s1 + 0x230) = 10.0f;
    func_00379C68(arg0, 0xFFFF);
}

INCLUDE_ASM("asm/levels", func_L05_00422540);

/* shared native body of family 5a97ab89858c */
void func_L05_00433400(char *p)
{
    *(long long *)(p + 112) = 0;
    *(float *)(p + 8) = -1.0f;
    *(float *)(p + 24) = 1.0f;
    *(float *)(p + 40) = -1.0f;
    *(float *)(p + 56) = 1.0f;
    *(float *)(p + 4) = -1.0f;
    *(float *)(p + 36) = 1.0f;
    *(float *)(p + 52) = 1.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -1.0f;
    *(float *)(p + 60) = 1.0f;
}
