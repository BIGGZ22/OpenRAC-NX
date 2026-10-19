/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Appends two GIF packets to the buffer at D_L01_00161240 (arg1 selects the scratchpad row): packet 1 from templates plus 46 words from 0x70001088 + arg1*4 and 23 12-byte pairs from 0x70002000 + arg1*12, packet 2 from templates plus 46 dwords from 0x70000780 + arg1*8, then a tag pointing at packet 1's second block; arg0 is OR-ed into a tag word. Not run in the capture; checked by hand against the assembly. */
typedef int Q_63338 __attribute__((mode(TI)));
extern unsigned int *D_L01_00161240_63338 __asm__("D_L01_00161240");
extern unsigned int D_L01_001CB3E0_63338[] __asm__("D_L01_001CB3E0");
extern unsigned int D_L01_001612C0_63338[] __asm__("D_L01_001612C0");
extern unsigned int D_L01_001CB0E0_63338[] __asm__("D_L01_001CB0E0");
extern unsigned int D_L01_001612E0_63338[] __asm__("D_L01_001612E0");
extern short D_L01_001CAFC0_63338[] __asm__("D_L01_001CAFC0");
extern unsigned int D_L01_00161300_63338[] __asm__("D_L01_00161300");
extern short D_L01_001CB080_63338[] __asm__("D_L01_001CB080");
extern short D_L01_001CB020_63338[] __asm__("D_L01_001CB020");

void func_L01_00263338(unsigned int arg0, int arg1) {
    unsigned int *r10 = D_L01_00161240_63338;
    unsigned int *r1 = r10;
    unsigned int *r6, *r7;
    unsigned int *src;
    short *sh;
    char *base;
    int i;

    src = D_L01_001CB3E0_63338;
    for (i = 5; i != 0; i--) {
        *(Q_63338 *)r1 = *(Q_63338 *)src;
        src += 4;
        r1 += 4;
    }
    src = D_L01_001612C0_63338;
    *(Q_63338 *)r1 = *(Q_63338 *)src;
    r1 += 4;
    src = D_L01_001CB0E0_63338;
    for (i = 0x17; i != 0; i--) {
        *(Q_63338 *)r1 = *(Q_63338 *)src;
        src += 4;
        r1 += 4;
    }
    r6 = r1;
    src = D_L01_001612E0_63338;
    *(Q_63338 *)r1 = *(Q_63338 *)src;
    r1 += 4;
    sh = D_L01_001CAFC0_63338;
    base = (char *)0x70001088 + arg1 * 4;
    for (i = 0x2E; i != 0; i--) {
        *r1 = *(unsigned int *)(base + *sh);
        sh++;
        r1++;
    }
    *(long *)r1 = 0;
    r1 += 2;
    r7 = r1;
    (void)r7;
    src = D_L01_00161300_63338;
    *(Q_63338 *)r1 = *(Q_63338 *)src;
    r1 += 4;
    sh = D_L01_001CB080_63338;
    base = (char *)0x70002000 + (arg1 * 8 + arg1 * 4);
    for (i = 0x17; i != 0; i--) {
        unsigned int *p = (unsigned int *)(base + sh[0]);
        unsigned int w0 = p[0], w1 = p[1], w2 = p[2];
        unsigned int *q;
        r1[0] = w0; r1[1] = w1; r1[2] = w2;
        q = (unsigned int *)(base + sh[1]);
        w0 = q[0]; w1 = q[1]; w2 = q[2];
        r1[3] = w0; r1[4] = w1; r1[5] = w2;
        sh += 2;
        r1 += 6;
    }
    r1[0] = 0x14000000 | arg0;
    r1[1] = 0;
    {
        *(Q_63338 *)(((unsigned int)(r1 + 2)) & ~0xFu) = 0;
    }
    r1[5] = 0x11000000;
    r1 += 6;
    *r10 = *r10 | ((((unsigned int)r1 - (unsigned int)r10) >> 4) - 1);
    D_L01_00161240_63338 = r1;
    r10 = r1;

    src = D_L01_001CB3E0_63338;
    for (i = 5; i != 0; i--) {
        *(Q_63338 *)r1 = *(Q_63338 *)src;
        src += 4;
        r1 += 4;
    }
    {
        unsigned int a = r1[-0xF] | 0x01000000;
        unsigned int b = r1[-0xA] | 0x01000000;
        r1[-0xF] = a;
        r1[-0xA] = b;
    }
    src = D_L01_001612C0_63338;
    *(Q_63338 *)r1 = *(Q_63338 *)src;
    r1 += 4;
    sh = D_L01_001CB020_63338;
    base = (char *)0x70000780 + arg1 * 8;
    for (i = 0x2E; i != 0; i--) {
        *(long *)r1 = *(long *)(base + *sh);
        sh++;
        r1 += 2;
    }
    *r10 = *r10 | ((((unsigned int)r1 - (unsigned int)r10) >> 4) - 1);
    D_L01_00161240_63338 = r1;
    r10 = r1;
    *(long *)(r10 + 2) = 0;
    r10[1] = (unsigned int)r6;
    r10[0] = 0x30000031;
    r10 += 4;
    D_L01_00161240_63338 = r10;
}
