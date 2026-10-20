/* func_L00_004282F0 (216 bytes, 27 places) - only the TI128 copy's two `addiu` are left.
 * Measured with scratch/wslcmp.py: "00_aranos_tutorial: 208 bytes, expected 216"; the same
 * body with `&D` on unsized arrays measured 220.  `check.py` not run (the size is wrong).
 *
 * Everything else reproduces retail word for word, and two of the three fixes are measured:
 * (a) the three $gp arguments (`addiu $a1,$gp,0x25F0` / 0x2608 / 0x2620) need SIZE on the
 * objects - an incomplete `extern u8 D[]` goes out `lui`+`addiu` and costs one word each,
 * a sized 8-byte object with `sda` reaches the bare GPREL16 addiu;
 * (b) the 0x001B8F10 store is fine as a literal (its low half has bit 15 clear, which is the
 * case the sixth wave says the literal serves);
 * (c) the calls, their argument registers and the `mov.s $f12,$f20` placement all match.
 * The residue is `*(TI128 *)(p+0xB0) = *(TI128 *)(p+0x80);`: retail materialises both
 * pointers (`addiu $a0,$s0,0xB0` then `addiu $v1,$s0,0x80`) and copies with
 * `lq $v0,0x0($v1)` / `sq $v0,0x0($a0)`; this cc1 folds the two displacements into the
 * lq/sq (`lq $v0,0x80($s0)` / `sq $v0,0xB0($s0)`), 2 words short.  Naming both addresses
 * through pointer locals (`u8 *dst = p + 0xB0; u8 *src = p + 0x80;`) did not stop the fold.
 * To land it: a spelling that keeps the two pointer computations alive (worker 02's TI128
 * destination finding, confirmed here on a 27-place row).
 */
typedef unsigned char u8;
typedef signed short s16;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern void *func_002D7A88(int a0);
extern void func_002D7DA0(void *a0);
extern void func_00428000(void *a0, void *a1);
extern void func_002D7958(void *a0, void *a1, f32 f12);
extern void func_002D7940(void *a0);
extern void func_00427C78(void *a0);

extern u8 D_L00_001B15E0[8] __attribute__((sda));
extern u8 D_L00_001B15F8[8] __attribute__((sda));
extern u8 D_L00_001B1610[8] __attribute__((sda));

void *func_L00_004282F0(u8 *a0) {
    u8 *s0 = a0;
    u8 *s2;
    u8 *p;
    f32 f20 = 0.0f;

    s2 = func_002D7A88(6);
    *(s16 *)0x001B8F10 = 0;
    func_002D7DA0(s2);
    func_00428000(s2, s0);
    p = *(u8 **)(s2 + 0x70);
    *(TI128 *)(p + 0xB0) = *(TI128 *)(p + 0x80);
    func_002D7958(p + 0x14, D_L00_001B15E0, f20);
    func_002D7958(s2 + 0x50, p + 0x14, f20);
    func_00427C78(s2);
    *(int *)(p + 0x9C) = 1;
    func_002D7958(p + 0x28, D_L00_001B15F8, f20);
    func_002D7958(p + 0x3C, D_L00_001B1610, f20);
    func_002D7940(p + 0x50);
    return s2;
}
