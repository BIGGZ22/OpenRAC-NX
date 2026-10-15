/* func_L11_0045B5C8 -- draft w17 (wave 6, file src/levels/11_joba/L11_00453358.c)
 * last check.py line: 11_joba: 252 bytes, expected 256 (1 instruction missing: retail keeps $a0/$a1 for the two globals and puts the store q->f0 in the delay slot of the beq)
 */

void func_L11_0045B5C8(char *p)
{
    char *q = *(char **)(p + 0x68);
    unsigned short c;
    int v;

    if (D_L11_001CAE40 != 0)
        return;
    switch (*(unsigned char *)(p + 0x20)) {
    case 0:
        *(unsigned char *)(p + 0x30) = 0xFF;
        func_002A7750(p, 1, -1);
        *(int *)(q + 0x0) = 0x1000000;
        *(int *)(q + 0x8) = 1;
        *(int *)(q + 0x4) = 1;
        break;
    case 1:
        if (*(unsigned short *)(p + 0x96) >= 0x2D) {
            *(unsigned short *)(p + 0x96) = 0;
            *(int *)(q + 0x4) = -*(int *)(q + 0x4);
        }
        v = *(int *)(q + 0x8) + *(int *)(q + 0x4);
        *(int *)(q + 0x0) = v << 24;
        *(int *)(q + 0x8) = v;
        if (D_L11_001B46D4 != D_L11_001B30C8) {
            D_L11_001B46D4 = D_L11_001B30C8;
            func_L00_002E6CF0(func_L11_0045B6C8, p);
        }
        break;
    }
    c = *(unsigned short *)(p + 0x96);
    if (c <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = c + 1;
}
