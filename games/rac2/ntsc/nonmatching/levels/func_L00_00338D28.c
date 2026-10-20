/* 0 of 19 places match - last line: 00_aranos_tutorial: 208 bytes, expected 204 (one word over).
   The record layout, the tag word, both byte fields, the quantised height and the class id are
   understood and transcribed; the wall is the first 16-byte copy. Retail computes the store
   address in the `beqz`'s DELAY SLOT (`addiu $v1,$s0,0x10`) and stores through it with a zero
   displacement, while this cc1 folds the offset into the store (`sq $v0,0x10($s0)`), which both
   loses the fill and hoists the following `addiu $a0,$zero,0x8` one slot early.
   Variants measured: member array, `(char *)o + 0x10` cast, both with the volatile qualifier
   that is otherwise needed (without it the store sinks into the next call's delay slot).
   The .text layout: w4 = ((func_002A77E0(8) + 0x28) << 24) | 0x00808080, b9 =
   func_00283CF0(2.0f) + 0x20, b3 = 0x48, b1 = 0, b2 = *(u8 *)*(int *)0x001FE9FC, f_c = the
   float argument, second copy from src2 into +0x20, b8 = func_002A77E0(0x100). */

typedef int V4_L00_00338D28 __attribute__((mode(TI)));

typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    int w4;
    unsigned char b8;
    unsigned char b9;
    short s_a;
    float f_c;
    char v10[16];
    char v20[16];
} Obj_L00_00338D28;

extern void *func_002B9030(int);
extern int func_002A77E0(int);
extern int func_00283CF0(float);
extern int *D_L00_001FE9FC;

Obj_L00_00338D28 *func_L00_00338D28(char *src1, char *src2, float value)
{
    Obj_L00_00338D28 *o = func_002B9030(0x2F);

    if (o == 0)
        return 0;
    *(volatile V4_L00_00338D28 *)((char *)o + 0x10) = *(V4_L00_00338D28 *)src1;
    o->w4 = ((func_002A77E0(8) + 0x28) << 24) | 0x00808080;
    o->b9 = func_00283CF0(2.0f) + 0x20;
    o->b3 = 0x48;
    o->b1 = 0;
    o->b2 = *(unsigned char *)D_L00_001FE9FC;
    o->f_c = value;
    *(volatile V4_L00_00338D28 *)((char *)o + 0x20) = *(V4_L00_00338D28 *)src2;
    o->b8 = func_002A77E0(0x100);
    return o;
}
