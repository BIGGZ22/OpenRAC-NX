/* from src/core/0012C508.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/00125630.c: func_00128A58 */
/* _nextBit (libmpeg.a:mpc.o), Get_Bits: waits for the IPU, refills the
 * bit cache when it is empty or short, returns the top n bits and tells
 * the IPU to consume them. The r1_D_00132F70 index needs its own local,
 * computed before the command store, or the allocator swaps $v1 and $a0.
 */
int func_0012C878(void *arg0, int n) {
    char *s = (char *)arg0;
    int counter = 0;
    unsigned int cmd;
    unsigned int idx;
    unsigned int oldBits;
    unsigned int result;

    while ((*(volatile int *)0x10002010 & 0x80004000) == 0x80000000) {
        if (counter++ >= 0x1389) {
            r1_func_0012BCC8(*(int *)(s + 0x858));
            counter = 0;
        }
    }

    if (*(int *)(s + 0x818) != 0 || *(int *)(s + 0x83C) < n) {
        *(volatile int *)0x10002000 = 0x40000000;
        *(int *)(s + 0x818) = r1_D_00132F70[4];
        *(int *)(s + 0x838) = func_0012C458(s);
    }

    *(int *)(s + 0x83C) = 0x20;
    oldBits = *(unsigned int *)(s + 0x838);
    cmd = n | 0x40000000;
    idx = cmd >> 28;
    *(volatile unsigned int *)0x10002000 = cmd;
    result = oldBits >> (0x20 - n);
    *(int *)(s + 0x818) = r1_D_00132F70[idx];
    *(int *)(s + 0x838) = func_0012C458(s);
    return result;
}
