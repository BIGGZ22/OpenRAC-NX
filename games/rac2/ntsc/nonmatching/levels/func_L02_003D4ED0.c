/* 248 bytes against retail's 252; level 2 only. Everything but the one 16-byte copy is exact: the frame, the
   calls, the rotation arithmetic and the order of the stores. Retail copies the target vector with
   `lq $v0,0($s0)` / `sq $v0,0($s2)` through the two held pointers (state + 0x2C0 and state + 0x2D0, the ones
   also passed to the calls), then `b` to the epilogue with the epilogue's first `ld` in the branch slot. This
   cc1 folds the constant offsets into the copy (`lq $v0,0x2C0($s0)`, `sq $v0,0x2D0($s0)`: $s0 then holds the
   state, where retail has the state in $s1 and the target in $s0) and fills the branch slot with the `sq`
   (4 bytes shorter). That is the cse tie between `reg` and `reg + const` addresses (find_best_addr).
   Tried (all fold the same way): `*(TI128 *)dst = *(TI128 *)src` through char *, TI128 * and unsigned address
   variables, through a TI128 temporary, through a struct holding one TI128, and through two sets of pointers.
   Measured, not usable:
   an `__asm__("lq $2,0(%1)\n\tsq $2,0(%0)" : : "r"(dst), "r"(src) : "$2", "memory")` in place of the copy makes
   the whole body byte-identical (tools/check.py: 1 of 1 places match); the asm is what keeps the pointers in
   registers and keeps the `sq` out of the delay slot, so the original copy was written that way. */
typedef int TI128 __attribute__((mode(TI)));

extern float func_00282E48(char *, char *);

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(char *, char *, float);

extern void func_00282CC0(void *, void *, void *);

extern float func_00283180(float);

extern float func_00283198(float);

void func_L02_003D4ED0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *dst = state + 0x2D0;
    char *src = state + 0x2C0;
    float v[4];

    if (func_00282E48(dst, src) < 0.8333334f) {
        *(TI128 *)dst = *(TI128 *)src;
    } else {
        func_00282CF0(v, src, dst);
        func_00282F20((char *)v, (char *)v, 0.8333334f);
        v[0] = v[0] * func_00283180(*(float *)(state + 0x308)) -
               v[1] * func_00283198(*(float *)(state + 0x308));
        v[1] = v[0] * func_00283198(*(float *)(state + 0x308)) +
               v[1] * func_00283180(*(float *)(state + 0x308));
        func_00282CC0(dst, dst, v);
    }
}
