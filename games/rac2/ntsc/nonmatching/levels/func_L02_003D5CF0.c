/* 268 bytes against retail's 264; level 2 only. Walks a line of trail nodes: the vector from arg1 to arg0 is
   split into steps of `step`, a node is made at each point (func_L02_0033C760) and the first one made is
   returned. The loop, the two conditional moves (`movn`), the likely branch with the `mul.s` in its slot and
   the frame match. The differences come from the one 16-byte copy of *arg0 into the local at sp+0, which
   retail does with a `lq $v0,0($s1)` / `sq $v0,0($sp)` pair that the scheduler treats as opaque:
   (1) the loop-invariant &diff (sp + 0x10) is copied to a second callee-saved register (`daddu $s4,$s0,$zero`,
   hoisted to the top by the scheduler, so $s4 is saved and restored); retail has a single register for it, so
   its second copy presumably sits after the pair, where it does not overlap the first;
   (2) `mtc1 $zero,$f20` is hoisted above the second call, where retail keeps it after the pair;
   (3) the `sq` goes into the delay slot of the `bc1f` that guards the loop, where retail has
   `daddu $v0,$s3,$zero`.
   Declaring `float v[4]` before `float diff[4]` gives retail's frame (v at sp+0).
   Tried: `float *d = diff` as a held pointer (removes the extra register but $s0 then goes to arg0), a TI128
   local instead of the array (the slots come out in the wrong order), `volatile` on the store (only fixes (3)).
   Measured, not usable: an `__asm__("lq $2,0(%1)\n\tsq $2,0(%0)" : : "r"(v), "r"(arg0) : "$2", "memory")` in
   place of the copy makes the whole body byte-identical (tools/check.py: 1 of 1 places match). */
typedef int TI128 __attribute__((mode(TI)));

extern void func_00282CF0(void *, void *, void *);

extern float func_00282DF0(void *);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(void *, void *, void *);

extern char *func_L02_0033C760(void *, char *, int, float);

char *func_L02_003D5CF0(char *arg0, char *arg1, float step) {
    float v[4];
    float diff[4];
    char *first = 0;
    char *node = 0;
    float len;
    float t;

    func_00282CF0(diff, arg1, arg0);
    len = func_00282DF0(diff);
    func_00282F20((char *)diff, (char *)diff, step);
    *(TI128 *)v = *(TI128 *)arg0;
    for (t = 0.0f; t < len; t += step) {
        node = func_L02_0033C760(v, node, 0, step * 1.3f);
        if (node != 0) {
            if (first == 0) first = node;
        }
        func_00282CC0(v, v, diff);
    }
    return first;
}
