/* Near miss (wave 7 tour 2, agent lvl7-03): size EXACT at 196 bytes, level 30 only.
 *
 * Decoded: refuses while the display state at 0x001A63A8 is busy (`[+0x68] != 0`) or its
 * current exhibit is still up (`[+0x24] != -1`) or the entity has no index (`[+0x144] == -1`);
 * otherwise looks the index up in the class table at 0x00295C60, and for class 6 picks
 * `q->fF8` when the index is 1 else `q->f124`, while any other class picks
 * `q[cls*4 + 0xE0]` and marks the class visited in the `$gp` bitmask (0x001AEC90 = the shared
 * base 0x1AEFF0 minus 0x360), or `q[cls*4 + 0x10C]` when the bit was already set; finally arms
 * `[0x001A63A8+0x24]` with `(v & 0x3FFF) + 0xBB8`, clears `[+0x28]` and returns 1.
 * Needs the -G8 sibling for the bitmask; at -G0 the attribute only gives `lui $at`.
 *
 * MEASURED: the boolean negation must be spelled `(((D >> cls) & 1) ^ 1) != 0`, NOT
 * `!(D & (1 << cls))`. The `^ 1` is what makes cc1 emit retail's `srav; xori $v0,$v0,1;
 * andi $v0,$v0,1; beqz` - with `!` it drops the `xori` and the body comes out 192 instead of
 * 196. With the `^ 1` spelling the SIZE IS EXACT.
 *
 * What is left is layout: retail branches all three guards to ONE return-0 block placed after
 * the body's `jr $ra; addiu $v0,1` (branches of +0x24), this cc1 puts the shared return-0
 * block immediately after the guard chain (branches of +0x7), which reshuffles everything from
 * +0x10 on. Retail also keeps only the HIGH half of 0x001A63A8 in `$t1` and rebuilds the
 * address at the tail with `addiu $v1,$t1,0x63A8`, where this cc1 materialises it again.
 * Tried for the layout: the three guards as separate `if (...) return 0;`, and as a nested
 * `if (a == 0) if (b == -1) if (c != -1) { ... }` with a single trailing `return 0;` -
 * identical 192/196 either way.
 */

typedef signed int s32;

typedef unsigned char u8;

extern char D_L30_001A63A8[];

extern int D_L30_00295C60[];

extern int D_L30_001AEC90 __attribute__((sda));

int func_L30_003848B8(u8 *a0)
{
    u8 *q = *(u8 **)(a0 + 0x68);
    s32 idx = *(s32 *)(q + 0x144);
    s32 cls;
    s32 v;

    if (*(s32 *)(D_L30_001A63A8 + 0x68) != 0)
        return 0;
    if (*(s32 *)(D_L30_001A63A8 + 0x24) != -1)
        return 0;
    if (idx == -1)
        return 0;
    cls = D_L30_00295C60[idx];
    if (cls == 6) {
        v = (idx == 1) ? *(s32 *)(q + 0xF8) : *(s32 *)(q + 0x124);
    } else if ((((D_L30_001AEC90 >> cls) & 1) ^ 1) != 0) {
        D_L30_001AEC90 |= 1 << cls;
        v = *(s32 *)(q + cls * 4 + 0xE0);
    } else {
        v = *(s32 *)(q + cls * 4 + 0x10C);
    }
    *(s32 *)(D_L30_001A63A8 + 0x24) = (v & 0x3FFF) + 0xBB8;
    *(s32 *)(D_L30_001A63A8 + 0x28) = 0;
    return 1;
}
