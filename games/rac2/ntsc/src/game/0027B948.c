#include "include_asm.h"

/* Write the measured GS privileged 64-bit register configuration in order. */
typedef unsigned long long GsRegisterValue;

extern GsRegisterValue D_001A6488[3];

/* Scalar GS setup wrapper; pinned standard integer arguments and helper calls. */
extern void func_00285030(int width, int height, int address, int mode);

extern void func_002FC560(unsigned int reg, unsigned long value);

extern void func_00285398(int width, int height);

/* Qwen draft (branch qwen/autonomy, 00dc80b4), checked by tools/check.py */
/* Wrapper function that calls func_00284EA8 and returns.
 * Stack frame is used only to save/restore the return address (ra).
 * The function follows standard MIPS ABI conventions:
 * - addiu sp,sp,-0x10 allocates 16 bytes on the stack
 * - sd ra,0x(sp) saves the return address
 * - jal calls the external helper
 * - delay slot is a nop
 * - ld ra,0x(sp) restores the return address
 * - jr ra returns to caller
 * - _addiu sp,sp,0x10 deallocates stack
 */
void func_0027BA48(void);

/* Qwen draft (branch qwen/autonomy, b9c5326c), checked by tools/check.py */
void func_002CBC58(int);

/* Family 489bf28ebf58470c (120 B, 2 placements): call every registered
 * callback with its paired argument, bounded by a resident count word.
 *
 * The three resident globals are reached with `lui`+`lw` / `lui`+`addiu`
 * pairs that the ASSEMBLER macro expands from a single RTL insn; this only
 * happens when the externs carry the `sda` attribute (SYMBOL_REF_FLAG), which
 * suppresses gcc's HIGH/LO_SUM address split.  Without it the compiler keeps
 * the `%hi` in an extra callee-saved register and the body is 3 words long.
 */
typedef void (*fn_t)(int);

extern int D_L02_001B3078 __attribute__((sda));

   /* resident callback count */
extern fn_t D_L02_001B3058[] __attribute__((sda));

 /* callback table        */
extern int D_L02_001B3068[] __attribute__((sda));

extern int Ff67aa9df_D_001A7350 __attribute__((sda));

extern int Ff67aa9df_D_001A7354 __attribute__((sda));

extern void Ff67aa9df_FUN_0027EA28(void *v, void *x);

void func_0027B948(void)
{
    GsRegisterValue framebuffer, display;
    *(volatile GsRegisterValue *)0x120000e0 = 0;
    *(volatile GsRegisterValue *)0x12000000 = 0xffa1;
    *(volatile GsRegisterValue *)0x12000020 = D_001A6488[0];
    framebuffer = D_001A6488[1];
    *(volatile GsRegisterValue *)0x12000070 = framebuffer;
    *(volatile GsRegisterValue *)0x12000090 = framebuffer;
    display = D_001A6488[2];
    *(volatile GsRegisterValue *)0x12000080 = display;
    *(volatile GsRegisterValue *)0x120000a0 = display;
    *(volatile GsRegisterValue *)0x120000d0 = 0;
}

/* Set up aligned GS buffer state and emit pinned register/clear helper calls; source identity and matching require complete-unit qualification */
void func_0027B9C0(int width, int height)
{
    int sum = width + height;
    int address;

    if (sum > 16)
        sum = 16;
    address = 0x3ff000 - (4 << sum);
    address = (address >> 13) << 13;
    func_00285030(width, height, address, 1);
    func_002FC560(0x47, 0x30000UL);
    func_002FC560(0x42, 0x8000000044UL);
    func_00285398(0x100, 0x100);
    func_002FC560(0x42, 0x8000000044UL);
}

void func_0027BA48(void)
{
  func_00284EA8();
  return;
}

INCLUDE_ASM("asm/game", func_0027BA68);

INCLUDE_ASM("asm/game", func_0027BD70);

INCLUDE_ASM("asm/game", func_0027C478);

void func_0027C480(void) {
    extern void func_00284F40(int);
    extern void func_0027BD70(void);
    extern int D_001B1594 __attribute__((sda));
    func_00284F40(0);
    D_001B1594 = 0x7F;
    func_0027BD70();
}

INCLUDE_ASM("asm/game", func_0027C4A8);

INCLUDE_ASM("asm/game", func_0027C510);

void func_0027C520(void)
{
  func_002CBC58(0);
  return;
}

INCLUDE_ASM("asm/game", func_0027C540);

INCLUDE_ASM("asm/game", func_0027C660);

INCLUDE_ASM("asm/game", func_0027C768);

INCLUDE_ASM("asm/game", func_0027C878);

extern int D_001B1608[] __attribute__((sda));
extern fn_t D_001B9E40[];
extern int D_001B9F40[];

void func_0027CDC8(fn_t first, int second) {
    int index = D_001B1608[0];
    if (index < 64) {
        D_001B9E40[index] = first;
        D_001B9F40[index] = second;
        D_001B1608[0] = index + 1;
    }
}

void func_0027CE18(void) {
    int index;
    for (index = 0; index < D_001B1608[0]; ++index)
        D_001B9E40[index](D_001B9F40[index]);
}

extern int D_001B1610[] __attribute__((sda));
extern fn_t D_001BA240[];
extern int D_001BA340[];

void func_0027CE90(fn_t first, int second) {
    int index = D_001B1610[0];
    if (index < 64) {
        D_001BA240[index] = first;
        D_001BA340[index] = second;
        D_001B1610[0] = index + 1;
    }
}

void func_0027CEE0(void) {
    int index;
    for (index = 0; index < D_001B1610[0]; ++index)
        D_001BA240[index](D_001BA340[index]);
}

extern int D_001B1614[] __attribute__((sda));
extern fn_t D_001BA440[];
extern int D_001BA540[];

void func_0027CF58(void) {
    int index;
    for (index = 0; index < D_001B1614[0]; ++index)
        D_001BA440[index](D_001BA540[index]);
}

extern int D_001B160C[] __attribute__((sda));
extern fn_t D_001BA040[];
extern int D_001BA140[];

void func_0027CFD0(fn_t first, int second) {
    int index = D_001B160C[0];
    if (index < 64) {
        D_001BA040[index] = first;
        D_001BA140[index] = second;
        D_001B160C[0] = index + 1;
    }
}

void func_0027D020(void) {
    int index;
    for (index = 0; index < D_001B160C[0]; ++index)
        D_001BA040[index](D_001BA140[index]);
}

extern int D_001B1638[] __attribute__((sda));
extern fn_t D_001B1618[] __attribute__((sda));
extern int D_001B1628[] __attribute__((sda));

void func_0027D098(fn_t callback, int argument) {
    int index = D_001B1638[0];
    if (index < 4) {
        D_001B1618[index] = callback;
        D_001B1628[index] = argument;
        D_001B1638[0] = index + 1;
    }
}

/* shared native body of family 489bf28ebf58470c */
  /* argument table        */

void func_0027D0E8(void) {
    int i;
    for (i = 0; i < D_L02_001B3078; i++) {
        D_L02_001B3058[i](D_L02_001B3068[i]);
    }
}

INCLUDE_ASM("asm/game", func_0027D160);

INCLUDE_ASM("asm/game", func_0027D330);

INCLUDE_ASM("asm/game", func_0027D4B8);

INCLUDE_ASM("asm/game", func_0027D5C0);

INCLUDE_ASM("asm/game", func_0027D900);

INCLUDE_ASM("asm/game", func_0027DB68);

INCLUDE_ASM("asm/game", func_0027DCE8);

INCLUDE_ASM("asm/game", func_0027DDC0);

INCLUDE_ASM("asm/game", func_0027DE50);

INCLUDE_ASM("asm/game", func_0027E010);

INCLUDE_ASM("asm/game", func_0027E018);

INCLUDE_ASM("asm/game", func_0027E198);

INCLUDE_ASM("asm/game", func_0027E1A0);

INCLUDE_ASM("asm/game", func_0027E4A0);

INCLUDE_ASM("asm/game", func_0027E4A8);

INCLUDE_ASM("asm/game", func_0027E920);

INCLUDE_ASM("asm/game", func_0027EA28);

void func_0027EAE8(int i0, int i1, int i2, int i3, int tag, void *x)
{
    long long v[4];

    v[0] = (((i0 << 4) + Ff67aa9df_D_001A7350) - 8)
         | ((long long)(((i1 << 4) + Ff67aa9df_D_001A7354) - 8) << 16)
         | ((long long)tag << 32);
    v[1] = (((i2 << 4) + Ff67aa9df_D_001A7350) - 8)
         | ((long long)(((i1 << 4) + Ff67aa9df_D_001A7354) - 8) << 16)
         | ((long long)tag << 32);
    v[2] = (((i0 << 4) + Ff67aa9df_D_001A7350) - 8)
         | ((long long)(((i3 << 4) + Ff67aa9df_D_001A7354) - 8) << 16)
         | ((long long)tag << 32);
    v[3] = (((i2 << 4) + Ff67aa9df_D_001A7350) - 8)
         | ((long long)(((i3 << 4) + Ff67aa9df_D_001A7354) - 8) << 16)
         | ((long long)tag << 32);
    Ff67aa9df_FUN_0027EA28(v, x);
}

INCLUDE_ASM("asm/game", func_0027EB88);

INCLUDE_ASM("asm/game", func_0027ECC8);

INCLUDE_ASM("asm/game", func_0027EE50);

INCLUDE_ASM("asm/game", func_0027EE58);
