/* func_L11_00456378 -- draft w17 (wave 6, file src/levels/11_joba/L11_00453358.c)
 * BEST ATTEMPT (log2): 11_joba: 184 bytes, expected 192 -- register allocation
 * CONFORMING to the retail (p->$s1, q->$s0) and the whole body identical EXCEPT the branch
 * form (mine `beq $v0,$s2` + nop, retail `bnel $v0,$s2` with a useful delay slot) and the
 * "then" block placed on the wrong side. Variant 1 call = 160 bytes, variant 2 calls = 200 bytes (2 jal).
 * Retail = 192 bytes with A SINGLE jal shared by the two paths.
 */

void func_L11_00456378(char *p)
{
    char *q = *(char **)(p + 0x68);
    int v;

    if (*(unsigned char *)(p + 0x20) < 4) {
        *(int *)(q + 0x240) = 0;
    } else {
        if (*(short *)(q + 0x256) != 1) {
            if (func_L11_00441868(*(int *)(p + 0xB8), *(short *)(p + 0xAA)) == 0) {
                *(short *)(q + 0x256) = 0;
                return;
            }
            *(short *)(q + 0x256) = 1;
        }
        v = func_002B3BA8(p, *(int *)(q + 0x24C), 0, 0, 100.0f, 100.0f);
        *(int *)(q + 0x240) = *(int *)(q + 0x220);
        *(short *)(q + 0x254) = v;
    }
}
