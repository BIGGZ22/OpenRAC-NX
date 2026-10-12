#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

extern u8 D_L24_0025B710[];

extern void func_L00_002A5048(u8 *);

/* Measured partial layouts; original type and game roles are unknown. */
typedef int Rac2Native_6b0741c38bf00fee_s32;

typedef unsigned char Rac2Native_6b0741c38bf00fee_u8;

typedef long Rac2Native_6b0741c38bf00fee_s64;

typedef unsigned long Rac2Native_6b0741c38bf00fee_u64;

extern Rac2Native_6b0741c38bf00fee_u8 D_0018B2BC[];

extern void func_L00_002A61C8_1(Rac2Native_6b0741c38bf00fee_u64, Rac2Native_6b0741c38bf00fee_s64, Rac2Native_6b0741c38bf00fee_s64, Rac2Native_6b0741c38bf00fee_s32);

extern void func_L00_002B9C58(short, short, short);

/* calls the measured element helper over a 0x1b80-byte array in 0xb0-byte steps */
void func_L00_002A5368(void) {
 u8 *p=D_L24_0025B710;
 u8 *end=p+0x1b80;
 do {func_L00_002A5048(p);p+=0xb0;}while((s32)p<(s32)end);
}

INCLUDE_ASM("asm/levels", func_L00_002A53B8);

INCLUDE_ASM("asm/levels", func_L00_002A5480);

INCLUDE_ASM("asm/levels", func_L00_002A5518);

INCLUDE_ASM("asm/levels", func_L00_002A5840);

INCLUDE_ASM("asm/levels", func_L00_002A5CB0);

INCLUDE_ASM("asm/levels", func_L00_002A6008);

/* passes the measured byte and constants to two callees */
void func_L00_002A6188(void) {
 func_L00_002A61C8_1(D_0018B2BC[1],0,1,0x32);
 func_L00_002B9C58(0x16,7,0);
}

INCLUDE_ASM("asm/levels", func_L00_002A61C8);
