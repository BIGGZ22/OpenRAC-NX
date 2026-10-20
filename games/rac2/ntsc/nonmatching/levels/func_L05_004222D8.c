/* Near miss (wave 7, agent lvl7-03): 172 bytes against retail's 164, level 5 only.
 *
 * Decoded in full: clamp four channel counters to 0x7F, then publish their low bytes into
 * D_L05_001395B8 + 0x61..0x64.  Needs the -G8 sibling (level 5's $gp base is 0x1AEFF0, so
 * the retail `lw $v0,0x2360($gp)` is D_L05_001B1350 while the three neighbours at
 * 0x1B1354/58/5C stay absolute - `extern s32 D_L05_001B1350 __attribute__((sda));`).
 * With that unit every instruction agrees except the address materialisation: retail builds
 * `lui $v0,0x14; addiu $v0,$v0,-0x6A48` at the very end, right before the four `sb`s, while
 * this cc1 hoists the same pair to the top of the body into `$a2` and then rematerialises a
 * dead `lw $v0,0x0($gp)` into the delay slots of the 2nd and 3rd clamp - the two extra words.
 * Tried: the pointer as a named local at the top / assigned after the clamps / inside a nested
 * block, the array indexed inline, `D + 0x61` with p[0..3], `> 0x7F` for `>= 0x80`, an empty
 * then-arm, the constant 0x7F in a named local, and a `u8 *q` neighbour base. All land on 172
 * or 176. What would unblock it: a way to make cc1 create the base pseudo only at the stores.
 */

typedef signed int s32;

typedef unsigned char u8;

extern u8 D_L05_001395B8[];

extern s32 D_L05_001B1350 __attribute__((sda));

extern s32 D_L05_001B1354;

extern s32 D_L05_001B1358;

extern s32 D_L05_001B135C;

void func_L05_004222D8(void)
{
    u8 *p = D_L05_001395B8;

    if (D_L05_001B1350 >= 0x80) D_L05_001B1350 = 0x7F;
    if (D_L05_001B1354 >= 0x80) D_L05_001B1354 = 0x7F;
    if (D_L05_001B1358 >= 0x80) D_L05_001B1358 = 0x7F;
    if (D_L05_001B135C >= 0x80) D_L05_001B135C = 0x7F;
    p[0x61] = D_L05_001B1350;
    p[0x62] = D_L05_001B1354;
    p[0x63] = D_L05_001B1358;
    p[0x64] = D_L05_001B135C;
}
