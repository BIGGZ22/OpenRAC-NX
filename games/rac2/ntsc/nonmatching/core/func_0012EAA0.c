/* from src/core/0012EAA0.c: 136 bytes; the candidate is 96 (SDK profile) or 104 (game) */
/*
 * Places a 64-bit value into one of the ten physical regions the table at
 * 0x137F78 describes ({u64 base; u64 mask} entries, ending in 0xCDCDCDCD fill):
 * the mask says which bits the region number occupies, so its trailing zeros are
 * the shift - 0xFF00000000 -> 32, 0xFFFF000000 -> 24, 0xFFFFFFFFFFFF -> 0 - and
 * the result is `base | (value << shift)`. The switch over the mask is the
 * natural source for the three values the table holds.
 *
 * What retail does that this cc1 does not:
 *  - the case selection is a value tree: `beq $v1,$a0` (mask == 0xFFFF000000),
 *    then `sltu $v0,$a0,$v1` + `bnel` to the shift-0 default, then
 *    `beq $v1,$v0` (mask == 0xFF00000000). Both profiles here compile the same
 *    two-case switch to two equality tests in a row: the SDK profile to
 *    `beq`/`bne` (96 bytes), the game profile to `beql`/`bne` (104 bytes).
 *    Reordering the cases in the source does not change which one is tested
 *    first - both cc1s sort the two case values - and neither emits the range
 *    test.
 *  - retail's frame is 16 bytes (`addiu $sp,$sp,-0x10` ... `addiu $sp,$sp,
 *    0x10`) with nothing stored in it; neither profile allocates a frame for
 *    this body. Whatever holds that frame is not visible in the C (there is no
 *    call), so the source shape that draws it is unknown.
 *
 * The table read, the two `ori`/`dsll` constant pairs, the variable `dsllv` and
 * the `ld`/`or` tail are the same in all three.
 */
typedef struct {
    unsigned long long base;
    unsigned long long mask;
} PhysRegion;

extern PhysRegion D_00137F78[];

unsigned long long func_0012EAA0(unsigned int arg0, unsigned long long arg1) {
    unsigned long long result = 0;

    if (arg0 < 10) {
        int shift;

        switch (D_00137F78[arg0].mask) {
        case 0xFFFF000000ULL:
            shift = 24;
            break;
        case 0xFF00000000ULL:
            shift = 32;
            break;
        default:
            shift = 0;
            break;
        }
        result = D_00137F78[arg0].base | (arg1 << shift);
    }
    return result;
}
