/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Starts the DMA channel at 0x1000D400 (CHCR 0x100, no DIR bit) with SADR (+0x80) = a1, MADR (+0x10) = a0, QWC (+0x20) = a2, in that order, then returns the uncached word at 0x20100000 | 1. Read as a copy of qwc quadwords from the scratchpad address a1 to main memory a0 (the direction the callers need: they stage an SPR buffer out to RAM); the copy is synchronous here and the retail status word is returned, not waited on. Written from the retail instructions (48 bytes); the DMA direction is inferred from the callers, not from hardware. */
int func_L02_002100E8(void *a0, unsigned int a1, int a2)
{
    unsigned char *dst = (unsigned char *)a0;
    const unsigned char *src = (const unsigned char *)a1;
    unsigned int status;
    int i, n;

    n = a2 * 16;
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }
    status = *(volatile unsigned int *)0x20100000;
    return (int)(status | 1);
}
