#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

/* func_L23_0041E418 is defined in L23_0041E418-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L23_0041E538);

INCLUDE_ASM("asm/levels", func_L23_0041E5B0);

INCLUDE_ASM("asm/levels", func_L23_00422540);

INCLUDE_ASM("asm/levels", func_L23_00422668);

INCLUDE_ASM("asm/levels", func_L23_00422798);

INCLUDE_ASM("asm/levels", func_L23_00422A30);

INCLUDE_ASM("asm/levels", func_L23_00422E08);

INCLUDE_ASM("asm/levels", func_L23_00423378);

/* func_L23_00423380 is defined in L23_0041E418-g8.c (this unit has no $gp access for it). */

/* func_L23_004234C0 is defined in L23_0041E418-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L23_004236B8);

INCLUDE_ASM("asm/levels", func_L23_00425568);

INCLUDE_ASM("asm/levels", func_L23_004256F0);

INCLUDE_ASM("asm/levels", func_L23_00425848);

INCLUDE_ASM("asm/levels", func_L23_00425AF8);

typedef int TI16 __attribute__((mode(TI)));

typedef short s16;

extern void func_00282CF0(float *, float *, float *);

extern void func_00282DD8(float *, float *, float *);

extern void func_00282F20(float *, float *, float);

extern int func_002A7820(int, int);

extern float func_00283C28(float);

extern float func_00283198(float);

extern void func_00282D30(float *, float *, float);

extern void func_00282CC0(float *, float *, float *);

extern float D_L23_001BA640[];

/* builds two vectors from the sources, rotates one by the measured angle and writes the result */
void func_L23_004260F0(u8 *obj, float *dst, TI16 *src1, TI16 *src2, float scale) {
    TI16 loc[6];
    TI16 va;
    TI16 vb;
    u8 *o68;
    s16 x;
    s32 t;
    float f;
    float h;

    va = *src2;
    vb = *src1;
    loc[1] = va;
    loc[0] = vb;
    o68 = *(u8 **)(obj + 0x68);
    func_00282CF0((float *)&loc[2], (float *)&loc[0], (float *)&loc[1]);
    func_00282CF0((float *)&loc[3], (float *)&loc[0], D_L23_001BA640);
    func_00282DD8((float *)&loc[4], (float *)&loc[3], (float *)&loc[2]);
    func_00282F20((float *)&loc[4], (float *)&loc[4], 1.0f);
    x = *(s16 *)(o68 + 0xC);
    t = func_002A7820(-x, x);
    t = (((s32)*(u16 *)(obj + 0x96) << *(s16 *)(o68 + 0xE)) + t) % 360 - 180;
    f = func_00283C28((float)t * 0.017453294f);
    h = func_00283198(f) * scale;
    func_00282D30((float *)&loc[5], (float *)&loc[4], h);
    func_00282CC0(dst, (float *)&loc[0], (float *)&loc[5]);
}

INCLUDE_ASM("asm/levels", func_L23_00426248);

INCLUDE_ASM("asm/levels", func_L23_004265D0);

INCLUDE_ASM("asm/levels", func_L23_004277B0);

/* func_L23_004277E8 is matched in L23_0041E418-g8.c */


/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_004278B0(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_004278D8);

INCLUDE_ASM("asm/levels", func_L23_00427CE8);

INCLUDE_ASM("asm/levels", func_L23_004280D8);

INCLUDE_ASM("asm/levels", func_L23_00428490);

INCLUDE_ASM("asm/levels", func_L23_00428CF8);

INCLUDE_ASM("asm/levels", func_L23_00429098);

INCLUDE_ASM("asm/levels", func_L23_00429430);


INCLUDE_ASM("asm/levels", func_L23_00429DC8);

/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_00429FF8(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_0042A018);

INCLUDE_ASM("asm/levels", func_L23_0042A438);

INCLUDE_ASM("asm/levels", func_L23_0042A8C0);

INCLUDE_ASM("asm/levels", func_L23_0042AD10);

/* increments the 16-bit counter at object + 0x96 while it stays below 0xFFFF */
void func_L23_0042AD90(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}

INCLUDE_ASM("asm/levels", func_L23_0042ADB0);

extern void func_L00_004287E8(void);

extern void func_L00_00428CE0(s32);

/* prepares the shared object then hands it to the shared starter */
void func_L23_0043F930(s32 arg0) {
    func_L00_004287E8();
    func_L00_00428CE0(arg0);
}

/* func_L23_004402D8 is matched in L23_004402D8-g8.c (built with -g). */
