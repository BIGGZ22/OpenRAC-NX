/* Attempts 1-4, no match: the compiled body is 100 bytes against a 116-byte
   retail body. The structure matches except the loop's iteration control:
   retail keeps a signed counter that runs 0,1,2 with `slti $v0,$a1,0x3`
   + `bnez`, while every source shape tried below is transformed by the
   pinned cc1 into a countdown (`addiu $a2,$zero,0x2` ... `bgez $a2`).
   A `unsigned int` counter does keep the incrementing layout, but cc1 then
   emits `sltiu` (+ 3 more instructions of peeling difference), not `slti`.
   The rest of the body (the peeled 16-byte copy, the early `lw $a3,0x68`,
   the `beqz` on +4 and the `func_00282D30(b+0xD0, b+0xD0, -1.0f)` call)
   reproduces correctly in the countdown form. */

typedef int TI128 __attribute__((mode(TI)));

extern void func_00282D30(char *, char *, float);

void func_L05_0041C7E8(char *a, char *b)
{
    char *state = *(char **)(b + 0x68);
    TI128 *src = (TI128 *)(a + 0xC0);
    TI128 *dst = (TI128 *)(b + 0xC0);
    unsigned int i;

    *(TI128 *)(b + 0x10) = *(TI128 *)(a + 0x10);
    for (i = 0; i < 3; i++)
        dst[i] = src[i];
    if (*(int *)(state + 4) != 0)
        func_00282D30(b + 0xD0, b + 0xD0, -1.0f);
}
