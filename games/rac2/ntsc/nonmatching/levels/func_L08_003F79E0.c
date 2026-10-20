/* func_L08_003F79E0, 104 bytes, 1 placement (level 8).  Best candidate 124 bytes.
   Decoded: `v1 = *(u8 **)(a0 + 0x68) + 0x10`, four constants (7000000.0f = 0x4AD59F80,
   5500000.0f = 0x4AA7D8C0, 9000000.0f = 0x4B095440, 1.0f) stored with `swc1` at
   +0/+4/+8/+0xC of a 16-byte stack temporary, `D_L08_001B9E04 = 0` (a word store), then
   `lq $v0,0x0($sp); sq $v0,0x0($v1)`.
   Retail zeroes the temporary first with `pcpyld $a0,$zero,$zero; sq $a0,0x0($sp)` - the
   128-bit zero the README already records as folded to `sq $zero` by
   rac2_fold_zero_ti_store (func_L00_002A75D8), and indeed no spelling here emits it.
   The 104 -> 124 gap is bigger than that alone: the four constants land in different FPRs
   ($f3/$f0/$f1/$f2 here against retail's $f3/$f2/$f0/$f1) and the frame grows to 0x20 with
   $ra/$s0 saved.  `D_L08_001B9E04` must be declared a word type - a `u8` declaration
   makes the store an `sb`.
   The body (closest so far): */

typedef unsigned char u8;
typedef unsigned int u32;
typedef float f32;
typedef int TQ __attribute__((mode(TI)));

extern u32 D_L08_001B9E04;

void func_L08_003F79E0(u8 *a0) {
    u8 *v1 = (u8 *)*(void **)(a0 + 0x68) + 0x10;
    f32 buf[4] = { 0 };

    buf[0] = 7000000.0f;
    buf[1] = 5500000.0f;
    buf[2] = 9000000.0f;
    buf[3] = 1.0f;
    D_L08_001B9E04 = 0;
    *(TQ *)(v1 + 0x0) = *(TQ *)buf;
}
