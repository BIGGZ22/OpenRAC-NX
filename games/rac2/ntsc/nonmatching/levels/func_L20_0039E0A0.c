/* Near miss (wave 7 tour 2, agent lvl7-03): 156 bytes against retail's 184, level 20 only.
 *
 * The "list walk" family of L01_003CAFE0-g8.c / L04_00422A00-g8.c, with a counter and a
 * wrap-around tail. Decoded: index the slot table at 0x001FBB20 with the raw argument, bail
 * on `(arg & 0xFF) == 0xFF`, on `D_L20_001B2FB8 < arg` and on a null slot, then walk the
 * 0x7FFF-masked u16 id list hanging off that slot, `e = D_L20_001B2F9C + ((*p & 0x7FFF) << 8)`,
 * counting the entries whose class byte `e[0x20]` is not 6 (retail spells it `lbu`, so the
 * element pointer is `unsigned char *`), and return the count.
 *
 * Retail's loop tail is
 *     120 beq   $a2,$t2,END        ; (arg & 0xFF) == 0xFF -> exit
 *     124 movn  $a3,$v1,$v0        ; count += (e[0x20] != 6)
 *     128 bnez  $t0,END            ; $t0 is ZERO and never written
 *     12C nop
 *     130 beqz  $a1,END            ; $p is never null
 *     134 nop
 *     138 lh    $v0,-0x2($a1)      ; the entry just consumed
 *     13C bgezl $v0,LOOP           ; still inside the chain -> continue
 *     144 lw    $v0,0x0($a0)       ; the list head
 *     148 beql  $a1,$v0,LOOP       ; wrapped back to the head -> continue
 * i.e. `while (!((s16)p[-1] < 0 && p != list))`, plus THREE guards this cc1 folds away as
 * provably dead (retail materialises `$t0 = 0` and still tests it). That is the whole
 * 7-instruction / 28-byte gap; every other instruction matches. The two `continue` branches
 * are written as two separate `if (...) continue;` in the C below, which is what reproduces
 * the two backward branches, and `e` is `unsigned char *` for the `lbu`.
 *
 * What would unblock it: finding the construct that makes cc1 keep a guard test on a value it
 * proves is 0, and on a pointer it proves non-null. No call, volatile or memory read is
 * obvious in the body, so a macro or an inlined helper in the original is the likely source.
 */

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern int D_L20_001B2FB8;

int func_L20_0039E0A0(unsigned int a0)
{
    unsigned int slot = a0 & 0xFF;
    int count = 0;
    char *list;
    unsigned short *p;
    char *base;
    unsigned char *e;

    if (slot == 0xFF)
        return count;
    list = D_L20_001FBB20[a0];
    if (D_L20_001B2FB8 < (int)a0)
        return count;
    if (list == 0)
        return count;
    p = (unsigned short *)list;
    base = D_L20_001B2F9C;
    for (;;) {
        e = (unsigned char *)(base + ((*p & 0x7FFF) << 8));
        if (e == 0)
            break;
        if (e[0x20] != 6)
            count++;
        p++;
        if (slot == 0xFF)
            break;
        if ((short)*(p - 1) >= 0)
            continue;
        if ((char *)p == list)
            continue;
        break;
    }
    return count;
}

/* ---- second attempt (level-20 wave, 2026-10-20) ---------------------------------------
 * The same list walk, with the three guards the earlier attempt spelled as separate
 * `if (...) break;` statements inside the loop body and the element pointer computed
 * from the `u16` word of the current record.  Measured: still short -- the retail
 * materialises `$t0 = 0` and still tests it, and this compiler folds all three
 * provably-dead guards away.  Kept as the starting point; not linked, not counted.
 */

extern int D_L20_001B2FB8;

int func_L20_0039E0A0(int index)
{
    u16 *rec = (u16 *)D_L20_001FBB20[index];
    int count = 0;
    int x = index & 0xFF;
    int t = 0;
    u16 w;

    if (x == 0xFF) {
        return 0;
    }
    if (D_L20_001B2FB8 < index) {
        return 0;
    }
    if (rec == 0) {
        return 0;
    }
    w = *rec;
    do {
        char *p = D_L20_001B2F9C + ((w & 0x7FFF) << 8);
        if (p == 0) {
            break;
        }
        if (x == 0xFF) {
            break;
        }
        if (t != 0) {
            break;
        }
        if (*(u8 *)(p + 0x20) ^ 6) {
            count++;
        }
        rec++;
        if (rec == 0) {
            break;
        }
        w = *rec;
    } while (*(s16 *)(rec - 1) >= 0 || rec == (u16 *)D_L20_001FBB20[index]);
    return count;
}

/* ---- third attempt (level-20 wave, worker 06, 2026-10-20) --------------------------
 * The closest spelling yet: 168 bytes against the retail's 184 (42 instructions against
 * 46).  What the two earlier attempts got wrong, and what this one fixes:
 *
 *  - The parameter is 32 BIT (`s32 index`), and this compiler still copies it with
 *    `daddu $a2,$a0,$zero` (its move), so the retail's 64-bit-looking copies are not a
 *    64-bit type.  A 64-bit parameter instead makes gcc sign-extend the table index
 *    (`dsll32`/`dsra32`), which the retail does not have.
 *  - The table element is addressed as `u16 **tbl = &D_L20_001FBB20[index];` and the
 *    record read through it, so the address of the slot lives in a register and the
 *    walk's wrap-around test re-reads it (`lw $v0,0($a0)`), as the retail does.
 *  - ONE exit for every bail-out (`goto ret;` with a single `ret:` label at the end),
 *    not a chain of `return 0;`: the chain gives the branch-likely exit shape and an
 *    early `jr $ra`, the shared label gives the retail's "branch to a common exit".
 *  - The inner `(arg & 0xFF) == 0xFF` test is on a 64-bit COPY of the 32-bit slot
 *    (`s64 slot = x;`), the outer one on the 32-bit `x`: the two tests then compare in
 *    different widths and this compiler no longer folds the inner one away.  The same
 *    width for both is folded, and one name for the value is folded as well.  The
 *    `rec == 0` test after the increment survives on its own (the 32-bit increment can
 *    wrap to zero), so two of the three guards are back.
 *
 * What is still missing (4 instructions):
 *
 *  1. THE ALWAYS-ZERO FLAG.  Retail materialises `$t0 = 0` (`daddu $t0,$zero,$zero`)
 *     before the loop and then tests it (`bnez $t0,END` + `nop`).  The value is a known
 *     zero, so this compiler folds the test away; measured and folded: `s64 flag = 0;`,
 *     `u64 flag = 0;`, `s32 flag = 0;`, the flag assigned inside the loop, the flag
 *     compared against a 64-bit zero, the flag in a local, the flag in a global.
 *     A flag read from memory keeps the test but then the retail would have a load,
 *     which it does not have.  No spelling found that keeps the test AND the register
 *     form of the retail; this looks like a 2.9-compiler artefact.
 *  2. THE BRANCH-LIKELY BACK EDGES.  Retail closes the loop with `bgezl`/`beql` and the
 *     reload of the record word in BOTH delay slots (one instruction more than here);
 *     this compiler emits plain `bgez`/`beq` and keeps the word in a register across the
 *     loop.  Measured with the disjunctive condition, the negated conjunction
 *     (`!((s16)p[-1] < 0 && p != list)`), the bitwise or, and two `continue`s in a
 *     `for (;;)`: all four give the plain form.  The five level-20 functions that DO
 *     match and carry a branch-likely (func_L20_002DC390, func_L20_00330A68,
 *     func_L20_0039AA98, func_L20_003C0350, func_L20_00442F30) all have it on a FORWARD
 *     branch out of a `switch`.
 *  3. The count's 32-bit increment: retail has `addiu $v1,$a3,0x1` on a 64-bit count,
 *     this compiler has `daddiu`.
 *  4. The register allocation and a few instruction positions (retail keeps the slot in
 *     $t0 and copies it to $a2 only after the two other guards; the table address in
 *     $v1; the two 0xFF constants in $a0 and $t2; the count's zero before the first
 *     test, not in its delay slot).
 *
 * The candidate follows.
 */

typedef unsigned char u8;

typedef unsigned short u16;

typedef signed short s16;

typedef long long s64;

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern int D_L20_001B2FB8;

s64 func_L20_0039E0A0(s32 index) {
    s64 count = 0;
    s32 x = index & 0xFF;
    s64 slot = x;
    u16 **tbl = &D_L20_001FBB20[index];
    u16 *rec = (u16 *)*tbl;
    u8 *e;
    u16 w;

    if (x == 0xFF) {
        goto ret;
    }
    if (D_L20_001B2FB8 < index) {
        goto ret;
    }
    if (rec == 0) {
        goto ret;
    }
    w = *rec;
    do {
        e = (u8 *)(D_L20_001B2F9C + ((w & 0x7FFF) << 8));
        if (e == 0) {
            break;
        }
        count += (e[0x20] != 6);
        rec++;
        if (slot == 0xFF) {
            break;
        }
        if (rec == 0) {
            break;
        }
        w = *rec;
    } while ((s16)rec[-1] >= 0 || rec == (u16 *)*tbl);
ret:
    return count;
}

/* ---- level-20 wave 3, worker 05 (2026-10-20): the zero guard IS reachable -------
 *
 * The question the note above asks - "the construct that makes cc1 keep a guard test on
 * a value it proves is 0" - has an answer, and it is not a macro or an inlined helper:
 *
 *     s32 flag = arg0 & ~arg0;
 *
 * The front end does NOT fold `x & ~x` (fold() in gcc/fold-const.c has no case for
 * BIT_AND with a non-constant second operand, and `operand_equal_p` is not consulted
 * there), and neither does the CSE.  Only `combine` folds it - and combine runs LAST
 * (toplev.c: cse -> gcse -> loop -> cse2 -> flow -> combine_instructions), i.e. AFTER
 * gcse, whose cprop pass is what would otherwise propagate the constant 0 into the
 * loop and delete the branch.  So the zero is materialised in the register
 * (`daddu $t0,$zero,$zero`) and the test survives (`bnez $t0`).
 *
 * Measured on this function:
 *   - `flag = 0`          -> the guard and the zeroing are gone, 176 bytes instead of 184
 *   - `flag = arg0 & ~arg0` -> both are there, 184 bytes, and every branch matches
 * The same spelling is what the other three functions of the family need
 * (func_L20_0039DBC8, func_L20_00440398, func_L20_0044AC38 - the same `daddu $tX,$zero,
 * $zero` + `bnez $tX` pair in all four).
 *
 * Measured on a minimal body (`int f = <expr>; for (i = 0; i < n; i++) if (f) return 1;`,
 * 8 instructions with the guard folded, 14 with it kept):
 *
 *     int f = 0;                 -> folded        int f = x & ~x;      -> KEPT
 *     int f = x - x;             -> folded        unsigned char f = x & ~x;   -> KEPT
 *     int f = x ^ x;             -> folded        unsigned short f = x & ~x;  -> KEPT
 *     int f = x & 0;             -> folded        long long f = x & ~x;      -> KEPT
 *     int f = x & ~x;            -> KEPT          unsigned long long f = x & ~x; -> KEPT
 *                                                    int *f = (int *)(x & ~x);    -> KEPT
 *                                                    int f = x & ~x; if (f != 0) -> KEPT
 *                                                    volatile int f = x & ~x;     -> KEPT (22 insns)
 * The test has to be in another block than the assignment (a loop); with the test in the
 * same block the plain CSE folds it again and no spelling survives.
 *
 * What is still missing is the register allocation.  With the guard in place the body
 * matches instruction for instruction, but the retail gives the flag $t0 (the register
 * the masked index had), which forces the index to be copied to $a2 and leaves $a0 to
 * the &table copy; ours gives the flag $a0 and keeps the index in $t1.  The shape is
 * otherwise retail's, down to the parameter copy at the top (`daddu $a2,$a0,$zero`),
 * the shared `goto end` exit (which gives retail's `beq $t0,$a0` polarity and its
 * single exit) and the branch-likely loop tail.
 *
 * Tried and not enough for the allocation: the flag's zero as `idx & ~idx` and as
 * `arg0 & ~arg0`; the flag assigned at the top of the function and just before the
 * loop; the masked index as a variable and inline; `(u8)arg0` in the loop's test (the
 * compiler then computes the mask twice); all orders of the four locals (count, p,
 * base, flag).  What is left: a source whose loop test reads the masked index through
 * a *copy* (the retail's `daddu $a2,$t0,$zero`), which would let the allocator give
 * $t0 to the flag.
 *
 * The candidate below is that attempt; check.py (20_yeedil) prints 176 bytes against
 * retail's 184, with the differences above. */

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C;

extern int D_L20_001B2FB8;

int func_L20_0039E0A0(s32 arg0) {
    s32 count = 0;
    u16 *p = (u16 *)D_L20_001FBB20[arg0];
    char *base;
    s32 flag;

    if ((arg0 & 0xFF) == 0xFF) {
        goto end;
    }
    if (D_L20_001B2FB8 < arg0) {
        goto end;
    }
    if (p == 0) {
        goto end;
    }
    base = D_L20_001B2F9C;
    flag = arg0 & ~arg0;
    do {
        char *e = base + ((*p & 0x7FFF) << 8);

        if (e == 0) {
            goto end;
        }
        if ((*(u8 *)(e + 0x20) ^ 6) != 0) {
            count++;
        }
        p = (u16 *)((char *)p + 2);
        if ((arg0 & 0xFF) == 0xFF) {
            goto end;
        }
        if (flag) {
            goto end;
        }
        if (p == 0) {
            goto end;
        }
    } while (*(s16 *)((char *)p - 2) >= 0 || p == (u16 *)D_L20_001FBB20[arg0]);
end:
    return count;
}
