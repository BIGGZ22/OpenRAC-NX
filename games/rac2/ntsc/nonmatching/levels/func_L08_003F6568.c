/* DRAFT (non matching) - func_L08_003F6568, 168 bytes, 2 places
   from src/levels/shared/L08_003D3928.c

   last check.py line:
     08_tabora: 152 bytes, expected 168
     func_L08_003F6568: 0 of 2 places match

   The whole 4-instruction gap is the accumulator `sel`: the retail keeps
   -1 in $s2 from before the two func_002A8600 calls (`addiu $s2,$zero,-1`,
   `sd $s2,0x20($sp)`, and at the end `sra $s2,$v0,8; daddu $v0,$s2,$zero`),
   so it is live across those calls; GCC here rematerialises the constant
   (`addiu $v0,$zero,-1` in the bnez delay slot) and never allocates $s2.
   Everything else (two calls, `sb $s0,0x131($s1)`, the shared `tmp` on the
   stack, the `(tmp - D) >> 8`) is already in the retail's shape and order.
   Tried: `s32 sel = -1;` declared second, declared last with a separate
   `sel = -1;` assignment, and a plain `return -1` (no variable at all). */
extern s32 D_L08_001B29DC __attribute__((sda));
extern s32 func_002A8508(s32, s32);
extern s32 func_002A8600(s32 *, s32, s32, s32);

s32 func_L08_003F6568(s32 a0, u8 *a1)
{
    s32 total;
    s32 tmp = 0;
    s32 sel;

    sel = -1;
    total = func_002A8508(*(s32 *)(a1 + 0x134), -1);
    total += func_002A8508(*(s32 *)(a1 + 0x138), -1);
    *(u8 *)(a1 + 0x131) = total;
    if (func_002A8600(&tmp, *(s32 *)(a1 + 0x134), 1, 1) != 0) {
        if (func_002A8600(&tmp, *(s32 *)(a1 + 0x138), 1, 1) != 0)
            return sel;
    }
    sel = (tmp - D_L08_001B29DC) >> 8;
    return sel;
}
