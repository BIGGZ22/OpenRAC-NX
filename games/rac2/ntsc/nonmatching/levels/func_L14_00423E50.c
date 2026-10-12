/* NEAR MISS — 136 bytes emitted, 144 expected (two instructions short).
 * last check.py line: "14_aranos_prison: 136 bytes, expected 144"
 *
 * Twin of func_L14_004286F8 (same table at D_L14_001FFB60, same record base, same walk); the
 * difference is that it reads the record base absolutely instead of through $gp, and it returns
 * 1 when the tagged record's word at 0xAA is 0xF91 rather than marking it.
 *
 * Residual wall is the entry block, not the loop. Retail:
 *   00423E50 daddu $a1,$a0,$zero      ; the parameter is copied into $a1 and read through it
 *   00423E58 lbu   $v0,0x21($a1)
 *   00423E5C bne   $v0,$v1 -> L2
 *   00423E64 addiu $a0,$a2,-0x4FA0    ; base (hi in $a2 keeps 0x00200000 for the reuse at L2)
 *   00423E68 sll   $v0,$v0,2          ; idx*4 recomputed, NOT folded even though idx == 0xFF
 *   00423E6C addu  $v0,$v0,$a0
 *   00423E70 lw    $v1,0x0($v0)
 *   00423E74 bnel  $v1,$zero -> L2
 *   00423E78 lbu   $v0,0x21($a1)      ; delay slot: idx RELOADED (the lw above may alias it)
 *   L2: 00423E8C addiu $v1,$a2,-0x4FA0   ; base recomputed from the saved %hi
 * This compiler instead folds `idx * 4` to the constant 0x3FC in the `idx == 0xFF` arm and
 * materialises the base without keeping the %hi, which costs both missing words. The loop
 * (c = *(short *)p; p++; while (c >= 0)) is otherwise identical to retail.
 */
extern char D_L14_001FFB60[];
extern int D_L14_001B259C;

int func_L14_00423E50(char *a0)
{
    unsigned char idx;
    unsigned short w;
    short c;
    unsigned short *p;
    char *q;

    idx = *(unsigned char *)(a0 + 0x21);
    if (idx == 0xFF && *(int *)(D_L14_001FFB60 + idx * 4) == 0)
        return 0;
    p = *(unsigned short **)(D_L14_001FFB60 + idx * 4);
    do {
        w = *p;
        q = (char *)(D_L14_001B259C + ((w & 0x7FFF) << 8));
        if (*(char *)(q + 0x20) >= 0 && *(short *)(q + 0xAA) != 0xF91)
            return 1;
        c = *(short *)p;
        p++;
    } while (c >= 0);
    return 0;
}
