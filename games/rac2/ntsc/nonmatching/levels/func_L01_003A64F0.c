/* 0 of 1 places match - last line: 01_oozla: 176 bytes, expected 188
   The whole call structure is recovered and the 128-bit copies come out as retail's
   lq/sq pairs once the locals are mode(TI): v = *(u128 *)(a0 + 0x10) (lq/sq to sp),
   func_00282F20(&t, a0 + 0xE0, <$gp float>), func_00282CC0(&v, &v, &t),
   func_00282CF0(&v, &v, a0 + 0xC0), p = *(u8 **)D_001B9500 (ABSOLUTE, so `nosda` in
   the -g8 file), w = *(u128 *)(a0 + 0xF0), then the `*(s16 *)(p + 0x86) == 5` split
   with func_00335370(&v) / func_003353C8(&w) against func_00335438(&v, &w, 0, 0, 0).
   12 bytes are missing: retail materialises the locals' addresses into registers
   (`addiu $v1,$s0,0x10` before the first lq, `addiu $s1,$sp,0x20` for the t base,
   `addiu $s1,$sp,0x10` for the w base) where this cc1 folds the offsets into the
   lq/sq (`lq $v0,0x10($s0)`). Frame, branch layout and both arms measured with four
   spellings (`mode(TI)` locals and u8[16] arrays, `!= 5` and `== 5` polarity, the else
   arm written first). */

typedef unsigned char u8;
typedef int s32;
typedef signed short s16;
typedef float f32;
typedef unsigned int u128 __attribute__((mode(TI)));

extern float D_L01_00000000 __attribute__((sda));
extern u8 *D_L01_001B9500 __attribute__((nosda));
extern void func_00282F20(u8 *, u8 *, f32);
extern void func_00282CC0(u8 *, u8 *, u8 *);
extern void func_00282CF0(u8 *, u8 *, u8 *);
extern void func_00335370(u8 *);
extern void func_003353C8(u8 *);
extern void func_00335438(u8 *, u8 *, s32, s32, s32);

void func_L01_003A64F0(u8 *a0) {
    u128 v;
    u128 w;
    u128 t;
    u8 *p;

    v = *(u128 *)(a0 + 0x10);
    func_00282F20((u8 *)&t, a0 + 0xE0, D_L01_00000000);
    func_00282CC0((u8 *)&v, (u8 *)&v, (u8 *)&t);
    func_00282CF0((u8 *)&v, (u8 *)&v, a0 + 0xC0);
    p = D_L01_001B9500;
    w = *(u128 *)(a0 + 0xF0);
    if (*(s16 *)(p + 0x86) == 5) {
        func_00335370((u8 *)&v);
        func_003353C8((u8 *)&w);
    } else {
        func_00335438((u8 *)&v, (u8 *)&w, 0, 0, 0);
    }
}
