/* func_L23_003C0A60 (532 bytes, level 23 only) -- NEAR MISS, not integrated.
   State: 532 bytes against retail's 532, 73 words differ, the first at +0x10: the register roles
   (retail: obj `$s7`, state `$s1`, p1 `$s2`, p2 `$s0`, loop pointers `$s3`/`$s4`/`$s5`/`$fp`, done
   `$s6`; this body allocates the roles differently and reuses a register copy `daddu` three times
   where retail recomputes `addiu $a0,$sp,0x20`: 12 `addiu` and 41 `daddu` against 15 and 38). Needs
   the -G8 unit (src/levels/23_damosel_orbit/L23_0035CE90-g8.c: `lwc1 0xA80($gp)` / `0xAC0($gp)`).
   Walks the segment p1 -> p2 in steps of D_L23_001AFAB0, testing each step with func_002761F8, and
   returns 1 when the segment is clear.

   Cause of the residue (measured): retail's four 16-byte copies (`lq $v0,0($s0) ; sq $v0,0($sp)` for
   loc0 = *p2, `addiu $v1,$sp,0x40 ; lq $v0,0($s2) ; sq $v0,0($v1)` for loc4 = *p1, `addiu $a0,$sp,0x20
   ; lq ; sq $v0,0($a0)` for loc2 = *p1, and loc2 = *q at the end of the loop) are volatile inline asm
   with register operands and a `$v0` scratch, of the shape
       __asm__ __volatile__("lq $2,0(%1)\n\tsq $2,0(%0)" :: "r"(dst), "r"(src) : "$2")
   (same finding as in func_L23_003C1E88, where that macro makes the whole body exact). Tell-tales
   here: the unfolded `0($reg)` addressing of locals; the `daddu $s3,$v1,$zero` / `daddu $fp,$s3,$zero`
   chain (the asm operand pseudo `$v1`, then two copies that cse cannot merge across the loop head);
   `daddu $s5,$s0,$zero` (a second name for &loc1); and the unmoved `mtc1 $zero,$f20` (the volatile asm
   is a full scheduling barrier, so the pseudo-only instructions around it stay in source order).
   A scratch variant with COPY128 on the four copies is 520 bytes (three instructions short): the
   loop register chain above was not reproduced, so this function is NOT shown to match with the
   macro.
   Inline asm is not allowed here.

   Plain-C levers used: a volatile TI128 access reaches lq/sq; `if (c) p = &x; else p = &x;` (or
   `p = &x; if (c) p = &x;`) makes the pointer multi-assigned so the access stays `0($reg)` instead
   of folding `0x40($sp)`. Choosing the condition register (`state` here) decides whether a no-op
   `movz` remains (none here). Arrays take their stack slot at their declaration, ascending
   (loc[0..5] at sp+0x00..0x50 as retail). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001AFA70 __attribute__((sda));
extern float D_L23_001AFAB0 __attribute__((sda));

extern float func_00282E48(void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern int func_002761F8(void *, void *, int, int, int);
extern void func_L23_00418650(void *, float);

extern int D_L23_001C0A80[];
extern int D_00189E20[];

int func_L23_003C0A60(char *obj, TI128 *p1, TI128 *p2) {
    char *state = *(char **)(obj + 0x68);
    TI128 loc[6];
    TI128 *pos;
    TI128 *q;
    TI128 *dst;
    float d;
    int done;
    int hit;

    if (D_L23_001AFA70 < func_00282E48(p1, p2))
        return 0;
    *(volatile TI128 *)&loc[0] = *(volatile TI128 *)p2;
    func_L23_00418650(&loc[0], *(float *)(state + 0x54) + *(float *)(state + 0x50));
    func_00282CF0(&loc[1], &loc[0], p1);
    func_00282F20(&loc[1], &loc[1], D_L23_001AFAB0);
    if (state)
        pos = &loc[4];
    else
        pos = &loc[4];
    *(volatile TI128 *)pos = *(volatile TI128 *)p1;
    dst = &loc[2];
    *dst = *p1;
    func_L23_00418650(dst, *(float *)(state + 0x54) + *(float *)(state + 0x50));
    done = 0;
    do {
        d = func_00282E48(pos, &loc[0]);
        if (d < D_L23_001AFAB0) {
            if (d < 0.0f)
                return 1;
            func_00282F20(&loc[5], &loc[1], d);
            done = 1;
            func_00282CC0(&loc[3], pos, &loc[5]);
            q = &loc[3];
            func_L23_00418650(&loc[3], *(float *)(state + 0x54) + *(float *)(state + 0x50));
        } else {
            func_00282CC0(&loc[3], pos, &loc[1]);
            q = &loc[3];
            func_L23_00418650(&loc[3], *(float *)(state + 0x54) + *(float *)(state + 0x50));
        }
        if (func_002761F8(&loc[2], q, 0x10, 0, 0)) {
            hit = D_L23_001C0A80[6];
            if (hit != (int)obj && hit != D_00189E20[0x8A4])
                return 0;
        }
        if (state)
            dst = &loc[2];
        else
            dst = &loc[2];
        *(volatile TI128 *)dst = *(volatile TI128 *)q;
        func_00282CC0(pos, pos, &loc[1]);
    } while (!done);
    return 1;
}
