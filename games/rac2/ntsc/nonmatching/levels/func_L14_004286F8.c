/* NEAR MISS — 120 bytes emitted, 124 expected (one instruction short).
 * Belongs in the -g8 sibling: retail reaches the record base with `lw $a2,0x35AC($gp)`.
 * gp is 0x001AEFF0 (docs/COMPILER-NOTES.md), so the global is at 0x001B259C — the same
 * address the twin func_L14_00423E50 reads absolutely (`lui $a2,0x1B; lw $a2,0x259C($a2)`).
 *
 * last check.py line: "14_aranos_prison: 120 bytes, expected 124"
 *
 * The body is otherwise instruction-for-instruction identical to retail (bltzl / bnel / bgez
 * shapes all reproduced). Residual differences, all allocation:
 *   - retail materialises idx in $v0, the 0xFF constant in $v1, and then copies
 *     `daddu $v1,$v0,$zero` before the base clobbers $v0; this compiler puts idx in $v1
 *     directly and needs no copy -> that copy is the missing word.
 *   - the loop constants land in $a3/$a2 here and in $t0/$a3 in retail; p/q are $a1/$a0 here
 *     and $a0/$a1 in retail (swapping the declaration order of p and q changed nothing).
 * The loop itself: `c = *(short *)p; p++; while (c >= 0);` is what reproduces the retail order
 * `lh $v0,0($a0) / bgez / addiu $a0,$a0,2` (the test reads the element before p advances).
 */
extern char D_L14_001FFB60[];
extern int D_L14_001B259C __attribute__((sda));

void func_L14_004286F8(char *a0)
{
    unsigned char idx;
    unsigned short w;
    short c;
    unsigned short *p;
    char *q;

    idx = *(unsigned char *)(a0 + 0x21);
    if (idx == 0xFF)
        return;
    p = *(unsigned short **)(D_L14_001FFB60 + idx * 4);
    if (p == 0)
        return;
    do {
        w = *p;
        q = (char *)(D_L14_001B259C + ((w & 0x7FFF) << 8));
        if (*(char *)(q + 0x20) >= 0 && *(short *)(q + 0xAA) == 0x112B)
            *(unsigned short *)(*(char **)(q + 0x68) + 0x316) = 1;
        c = *(short *)p;
        p++;
    } while (c >= 0);
}
