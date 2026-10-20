/* func_L23_003C01F8 (204 bytes, 1 place: 23:003C01F8) -- NEAR MISS, not integrated.
   Not an MMI row: the five `.word`s of the earlier record are R5900 `lq`/`sq` at offset 0
   (`lq $v0,0($s1)`, `sq $v0,0($sp)`, `sq $v0,0($s0)`, `lq $v0,0($v1)`, `sq $v0,0($s1)`).
   The two guards do not return: both branch to the `beqz $s1`, so they only skip the call.

   This C compiles to 204 bytes, the retail size; check.py reports "instruction at +0x54 differs";
   12 words differ, all in the 16-byte copies:
   - the copy into b is folded to `sq $v0,0x10($sp)` where retail stores through $s0 = sp+0x10
     (`addiu $s0,$sp,0x10 ; lq $v0,0($s1) ; sq $v0,0($s0)`), which also moves `daddu $a0,$s0` out
     of the first call's delay slot;
   - the -2.0f argument (`lui $at,0xC000 ; mtc1 $at,$f12`) is scheduled first in the block, before
     the copies; retail keeps it after them;
   - the final copy has the address in $v0 and the data in $v1; retail the reverse
     (`lui $v1 ; addiu $v1 ; lq $v0,0($v1)`). By my reading of local-alloc.c, the tied
     high/lo_sum qty and the TImode qty compute the same priority and the address wins on qty
     order. -mno-split-addresses swaps the two registers but makes `la` a macro that cannot fill
     the delay slot (208 bytes).
   Cause (measured): the original copies are the volatile inline-asm macro
       __asm__ __volatile__("lq $2,0(%1)\n\tsq $2,0(%0)" :: "r"(dst), "r"(src) : "$2")
   With COPY128(&a, arg0 + 0x10), COPY128(&b, arg0 + 0x10) and
   COPY128(arg0 + 0x10, &D_L23_001C0AA0) in this body, a scratch copy of the unit is exact (0 words
   differ, derive OK). Inline asm is not allowed in src/, so this stays a near miss.

   Plain-C levers found on the way (each measured here):
   - `arg0 = (char *)&b;` after `pos = arg0 + 0x10` kills cse's equivalence pos == arg0+0x10, so
     the loads stay `lq $v0,0($s1)` (no fold) and arg0's register $s0 is reused for &b as in
     retail. With pos a separate pseudo its base is unknown to alias.c (find_base_value has no
     ADDRESS case for the argument copy), so the store to `a` between the copies keeps the second
     `lq` (cse2 merged them while the address was folded to arg0+0x10).
   - `p = &a;` before `arg0 = (char *)&b;` gives a sp+0 and b sp+0x10: purge_addressof allocates
     the slots in the order the addresses are first taken as values.
   Tried without effect (all 12 words or worse): 32 permutations of the copy spellings (direct
   assignment or through pointers, kill before or between the copies, &b or arg0 in the calls),
   the global copy through a local pointer or a TI16 temporary, an inline helper `copy(d, s)`
   (copies merged and folded), a reassigned destination pointer, a `TI16 v[2]` array, -g, -G8. */

typedef int TI16 __attribute__((mode(TI)));

extern float D_L23_001AFA10 __attribute__((sda));

extern TI16 D_L23_001C0AA0;

extern void func_L23_003BFFC0(char *arg0, int arg1, int arg2, int arg3, int arg4, float arg5);

extern void func_L23_00418650(TI16 *, float);

extern int func_002761F8(TI16 *, TI16 *, int, int, int);

void func_L23_003C01F8(char *arg0, int arg1) {
    TI16 a;
    TI16 b;
    TI16 *p;

    if ((unsigned char)(*(unsigned char *)(arg0 + 0x43) - 5) >= 2 && *(unsigned char *)(arg0 + 0x20) != 2)
        func_L23_003BFFC0(arg0, 5, 0, 6, 1, D_L23_001AFA10);
    if (arg1 != 0) {
        TI16 *pos = (TI16 *)(arg0 + 0x10);

        p = &a;
        arg0 = (char *)&b;
        a = *pos;
        b = *pos;
        func_L23_00418650((TI16 *)arg0, -2.0f);
        func_L23_00418650(p, 0.1f);
        if (func_002761F8((TI16 *)arg0, p, 0x12, 0, 0) != 0)
            *pos = D_L23_001C0AA0;
    }
}
