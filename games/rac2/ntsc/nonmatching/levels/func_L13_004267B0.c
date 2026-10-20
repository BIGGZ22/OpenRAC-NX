/* 0 of 1 places match - last line: 13_boldan: instruction at +0x40 differs
   Size exact (112). The whole first block is byte-identical, registers included:
   the 0x0019B2E8 table must be described as a 16-byte-entry array indexed by the
   0x001A79F0 counter (`Entry16 e[]; e[D_001A79F0].b[a0[0xB0]]` gives retail's
   `sll $v0,$v0,4` / `lbu $v1,0xB0` / `addu` trio with $v0/$v1/$a0 exactly).
   The flag test is the 0x80 bit of that byte.
   Second block: the same schedule swaps two registers and the branch delay slot.
   Retail runs `lw $a1,0x68($s0)` (the object pointer) in the beqz delay slot, then
   `lw $a0,0x0($a1)` (the index), then materialises the 0x001B291C base into $v0;
   this cc1 hoists the base's `lui` into the delay slot and gives the pointer $v1 /
   the base $a1. Fifteen spellings measured (base first/last/in a local, index in a
   local, both in locals, no locals, u8*/s32*/struct field, u32/u8 index,
   `base + off` and `off + base`); all emit the same block, so it is scheduling and
   register allocation, not source shape.
   The 0x001B291C base MUST NOT carry the `sda` attribute here: with it the load
   lands in the delay slot and gas emits one gp-relative word (108 bytes); the
   absolute second name (`extern u8 *D_L13_001B291C_2;`) gives retail's lui/lw. */

typedef unsigned char u8;
typedef int s32;

extern s32 D_001A79F0 __attribute__((sda));
extern u8 *D_L13_001B291C_2;
extern void func_31A718(u8 *);

typedef struct {
    u8 b[16];
} Entry16;

extern Entry16 D_0019B2E8_e[];

void func_L13_004267B0(u8 *a0) {
    if (D_0019B2E8_e[D_001A79F0].b[a0[0xB0]] & 0x80) {
        s32 *ptr = *(s32 **)(a0 + 0x68);

        func_31A718(D_L13_001B291C_2 + (ptr[0] << 8));
        func_31A718(a0);
    }
}
