/* func_L23_003C16D8 (644 bytes, level 23 only) -- NEAR MISS, not integrated.
   State: 644 bytes against retail's 644, 39 words differ, the first at +0x8 (the prologue: this body
   keeps `obj` in `$s2` and saves a third callee-saved register, retail keeps it in `$s0`). The equal
   size is a coincidence: the body lacks retail's one `por $v0,$zero,$zero` (block 2, `sq $v0,0($sp)`
   with a live `por`) and spends the four bytes on the extra `$s2` save/restore. Needs the -G8 unit
   (src/levels/23_damosel_orbit/L23_0035CE90-g8.c: `lw`/`lhu`/`lwc1 0xA74..($gp)`).

   Cause of the residue (measured): retail's quadword zero stores are volatile inline asm with
   register operands, in the two forms
       #define ZERO128(dst)  __asm__ __volatile__("sq $0,0(%0)" :: "r"(dst))
       #define ZERO128P(dst) __asm__ __volatile__("por $2,$0,$0\n\tsq $2,0(%0)" :: "r"(dst) : "$2")
   ZERO128P is the first zero store of block 2 (`por $v0,$zero,$zero ; sq $v0,0($sp)`, the operand is
   `$sp` itself); ZERO128 is the three others (`addiu $v0,$sp,0x40 ; sq $zero,0($v0)` and the same at
   0x50 in both blocks; the 0x40 store of block 2 is dead and stays). Evidence: a plain TImode zero
   store is folded to `sq $zero,0x40($sp)` by the compiler and its `por` is deleted (the project's zero
   fold pass); the asm keeps `por`, keeps the pointer in `$v0`, and, being a volatile asm without a
   memory clobber, makes cse forget the shared `addiu` for the vector at sp+0x10 between its two
   calls (retail recomputes `addiu $a2,$sp,0x10` for the second call; plain C shares one `$s0`).
   Scratch variant (not committable): this body with `*p = 0` replaced by ZERO128(t) / ZERO128(u)
   and the first zero of block 2 by ZERO128P(v0), the pointer trick and the volatile removed, is 644
   bytes with `$s0 = obj`, `$s1 = state` as retail; with the float arguments below also computed
   into locals first (and `float one = 1.0f;` assigned before the ZERO128P, so its `lui/mtc1` stays
   before the asm as in retail) only 6 to 7 words differ: the order of the two lane stores against the
   `func_0029FC20` argument setup in block 2. Inline asm is not allowed here.

   Plain-C levers measured on the way: computing the float arguments of func_002AAA00 into locals
   before the `fz` store keeps `sw $zero,0xA0($sp)` after the three `lwc1`s as retail (56 -> 39
   words); `*(volatile TI128 *)p = 0` plus `p = &x; if (c) p = &x;` (or
   `if (c) p = &x; else p = &x;`) keeps the zero store through a materialised pointer
   (`addiu $v0,$sp,0x40 ; sq $zero,0($v0)`) instead of `sq $zero,0x40($sp)`. Scalar TI128 locals take
   their slot where their address is first taken, arrays where they are declared, ascending: the
   arrays below put v0/v1/t/u/r3/r4 at sp+0x00/0x10/0x40/0x50/0x60/0x90 and the floats at 0xA0/0xA4,
   as retail (the two unused pairs are the unexplained 0x20..0x3F and 0x70..0x8F holes). */

typedef int TI128 __attribute__((mode(TI)));

extern int D_L23_001AFA74 __attribute__((sda));
extern char D_00189EA0[];

extern void func_0029FCC0(void *, int, void *);
extern void func_002AF1D0(void *, void *, int, int);
extern void func_0029FC20(void *, int, void *);
extern void func_00283098(void *, void *, void *);
extern float func_002B04E0(void *, void *, int);
extern void func_002AAA00(void *, void *, int, float, float, float, float);
extern void func_002833F0(void *, void *);
extern void func_00283738(void *, void *, void *);
extern void func_002A05A8(void *);
extern void func_002A0F48(void *);
extern void func_00282D30(void *, void *, float);
extern void func_002B0408(void *, void *, int);
extern void func_00282978(void *);
extern void func_00282CF0(void *, void *, void *);

void func_L23_003C16D8(char *obj) {
    TI128 v0[1], v1[1], padA[2], t[1], u[1], r3[1], padB[2], r4[1];
    float fz[2];
    char *state = *(char **)(obj + 0x68);
    float r;
    float a1;
    float b1;
    float c1;
    TI128 *p;

    if (*(unsigned char *)(obj + 0x95) == 2 || *(short *)(state + 0x2A6) != 0)
        func_0029FCC0(obj, 1, *(char **)(state + 0x2A0) + 0x10);
    else
        func_002AF1D0(obj, *(char **)(state + 0x2A0), 1, 0);
    if (*(unsigned char *)(obj + 0x95) == 2) {
        if (*(short *)(state + 0x2A6) == 0)
            *(int *)(state + 0x2AC) = 0;
        func_00282CF0(v0, D_00189EA0, *(char **)(state + 0x2A0) + 0x10);
        r = func_002B04E0(*(char **)(state + 0x2A0), v0, 0);
        b1 = *(float *)(state + 0x68) + *(float *)(state + 0x68);
        a1 = *(float *)(state + 0x64);
        c1 = *(float *)(state + 0x6C);
        fz[0] = 0;
        func_002AAA00(fz, state + 0x2AC, 0, r, a1, b1, c1);
        p = t;
        *p = 0;
        ((float *)t)[2] = *(float *)(state + 0x2AC);
        func_002833F0(v1, t);
        func_00283738(*(char **)(state + 0x2A0) + 0xC0, *(char **)(state + 0x2A0) + 0xC0, v1);
        func_002A05A8(*(char **)(state + 0x2A0));
        *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) |= 0x106;
        func_002A0F48(*(char **)(state + 0x2A0));
        func_00282D30(u, state + 0x280, -1.0f);
        func_002B0408(*(char **)(state + 0x2A0) + 0xC0, u, 0);
        *(short *)(state + 0x2A6) = D_L23_001AFA74;
    } else {
        short v = *(short *)(state + 0x2A6);

        if (v != 0) {
            if (v == D_L23_001AFA74)
                *(int *)(state + 0x2AC) = 0;
            *v0 = 0;
            ((float *)v0)[0] = 1.0f;
            ((float *)v0)[3] = 1.0f;
            func_0029FC20(obj, 1, v1);
            p = t;
            if (state)
                p = t;
            else
                p = t;
            *(volatile TI128 *)p = 0;
            func_00283098(v0, v0, v1);
            r = func_002B04E0(*(char **)(state + 0x2A0), v0, 0);
            a1 = *(float *)(state + 0x64) + *(float *)(state + 0x64);
            c1 = *(float *)(state + 0x6C);
            b1 = *(float *)(state + 0x68) * 4.0f;
            fz[1] = 0;
            func_002AAA00(&fz[1], state + 0x2AC, 0, r, a1, b1, c1);
            p = u;
            if (v)
                p = u;
            *(volatile TI128 *)p = 0;
            ((float *)u)[2] = *(float *)(state + 0x2AC);
            func_002833F0(r3, u);
            func_00283738(*(char **)(state + 0x2A0) + 0xC0, *(char **)(state + 0x2A0) + 0xC0, r3);
            func_002A05A8(*(char **)(state + 0x2A0));
            *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) |= 0x106;
            func_002A0F48(*(char **)(state + 0x2A0));
            func_00282D30(r4, state + 0x280, -1.0f);
            func_002B0408(*(char **)(state + 0x2A0) + 0xC0, r4, 0);
            func_00282978(state + 0x2A6);
        }
    }
}
