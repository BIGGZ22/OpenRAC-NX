/* 1 of 12 instructions differs (27 places, all at +0x10): retail adds the element address as base + index (addu a2,a1); the compiler emits index + base (addu a1,a2). Reordering the operands, a cast, or a base local did not change it. */
typedef int s32;
typedef unsigned char u8;

extern u8 D_L00_00262E20[];

u8 *func_L00_0032A088(s32 a0, s32 a1)
{
    s32 idx = a1 * 0xA430;
    s32 *e = (s32 *)(D_L00_00262E20 + idx);

    if (e[1] != a0) {
        return 0;
    }
    return (D_L00_00262E20 + 0x20) + idx;
}
