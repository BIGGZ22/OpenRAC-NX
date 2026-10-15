/* w13 ABANDON — func_L01_003990E8, 132 bytes, 5 places.
 * last check.py line (5th compilation):
 *   func_L01_003990E8: 0 of 5 places match
 *   (retail) "instruction at +0xc differs"
 * ONE single instruction differs: the materialisation order of the two
 * preheader constants. Retail: [addiu $t1,$0,1 ; lhu $v0,0x34 ; addiu
 * $t0,$0,2]. This candidate: [addiu $t0,$0,2 ; lhu ; addiu $t1,$0,1]. The
 * registers, the order of the 3 stores (0x98, 0x34, 0x31) and the whole loop
 * are identical to the retail.
 * Tried: inverse store order, `p = ...` before/after the store 0x31,
 * explicit `i = 2;` before the RMW -- none moves the hoisted constant.
 */
void func_L01_003990E8(char *a3)
{
    int i;
    char **p;
    char *q;

    q = *(char **)(a3 + 0x24);
    *(unsigned int *)(a3 + 0x98) = *(unsigned int *)(q + 0x10);
    *(unsigned short *)(a3 + 0x34) &= 0xFFFE;
    p = (char **)(*(char **)(a3 + 0x68) + 0x14);
    *(unsigned char *)(a3 + 0x31) = 1;
    for (i = 2; i >= 0; i--) {
        if (*p != 0) {
            *(unsigned short *)(*p + 0x34) &= 0xFFFE;
            *(unsigned char *)(*p + 0x31) = 1;
            *(unsigned short *)(*p + 0x34) &= 0xFF7F;
            *(long long *)(a3 + 0x38) = *(long long *)(*p + 0x38);
        }
        p++;
    }
}
