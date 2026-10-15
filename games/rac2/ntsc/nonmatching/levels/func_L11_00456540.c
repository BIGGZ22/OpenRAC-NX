/* func_L11_00456540 -- draft w17 (wave 6, file src/levels/11_joba/L11_00453358.c)
 * last check.py line: 11_joba: instruction at +0x8 differs (same size 148 bytes; only the two callee-saved registers are swapped: retail p->s0, q->s1, mine p->s1, q->s0)
 */

void func_L11_00456540(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(short *)(q + 0x256) == 1) {
        func_L11_00441978(*(int *)(p + 0xB8), 0xDF9);
        *(int *)(q + 0x240) = 0;
        *(short *)(q + 0x256) = 0;
    } else {
        *(int *)(q + 0x240) = 0;
    }
    func_002A7470(p, 0, 0, 3);
    func_002A05A8(p);
    func_002A7750(p, 1, 0);
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(char *)(p + 0x31) = 0;
}
