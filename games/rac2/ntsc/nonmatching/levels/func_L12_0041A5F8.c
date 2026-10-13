/* func_L12_0041A5F8 (160 B) — best attempt, NOT matching.
 *
 * tools/check.py func_L12_0041A5F8 --diff -> "12_todano: 156 bytes, expected 160"
 * Residual: (a) the node pointer lives in $v1 where retail has $a3;
 *           (b) the 128-bit copy comes out as `lq $v1,0x210($v1)` / `sq $v0,0x10($s0)`
 *               where retail pre-computes both bases: `addiu $v1,$a3,0x210; lq $v0,0($v1)`
 *               and `addiu $a1,$s0,0x10; sq $v0,0($a1)`;
 *           (c) 4 bytes short (one instruction).
 * The lq/sq spelling itself is right (typedef int TI128 __attribute__((mode(TI)))).
 */

typedef int TI128 __attribute__((mode(TI)));

void func_L12_0041A5F8(u8 *self)
{
    u8 *dst = self + 0x10;
    u8 *n = *(u8 **)(self + 0x68);
    u8 *m = *(u8 **)(self + 0x24);

    *(TI128 *)dst = *(TI128 *)(n + 0x210);
    *(f32 *)(self + 0xF8) = *(f32 *)(n + 0x21C);
    *(u32 *)(n + 0x224) = 0;
    *(u16 *)(self + 0x34) = (*(u16 *)(self + 0x34) & 0xFFBE) | 0x1000;
    *(u32 *)(self + 0x98) = *(u32 *)(m + 0x10);
    func_L02_002EF388(self, dst, self + 0xF0, 0, 0, m);
    func_002A7750(self, 2, -1);
    if (*(u8 *)(self + 0x43) != 8)
        func_002A7470(self, 8, 0, 1);
}
