/* functional: medium. Written from the assembly (hand-written DMA-packet builder, no C existed): copies, appends and pads as the assembly does, the quadword moves done as four-word moves. The count loops are do-while as in retail (a count of 0 or less would run away there too; the callers pass at least 1). */
extern int *D_L00_00161280;
extern unsigned int D_L00_001C4320[];
extern unsigned int D_L00_00160F80[];
extern unsigned int D_L00_00160FA0[];
extern unsigned int D_L00_00160FC0[];

/* Appends to the packet at D_L00_00161280 a chain of four sections, each a 16-byte header copied
   from a template with n or'd in (the first with n in word 1 of the second quadword, the
   others with n << 16 in word 3), followed by n elements: 8 bytes each from a3, 4 bytes
   each from a2, 12 bytes each from a1; every section is padded with zero words to 16 bytes.
   The first template is 5 quadwords. A last quadword (0x15000000 | ((flag & 1) * 2 + 12),
   0x11000000, 0, 0) closes it and the first word of the packet gets the quadword count - 1
   or'd in. D_L00_00161280 is advanced past the packet. */
void func_L00_001FDE48(int n, int a1, int a2, void *a3, int flag)
{
    unsigned int *start = (unsigned int *)D_L00_00161280;
    unsigned int *w = start;
    unsigned int *src;
    unsigned int shifted = (unsigned int)n << 16;
    unsigned int tail = ((unsigned int)(flag & 1) << 1) + 0xC;
    int i, k;

    for (k = 0; k < 5; k++) {
        for (i = 0; i < 4; i++) {
            w[i] = D_L00_001C4320[k * 4 + i];
        }
        w += 4;
    }
    w[-16] |= (unsigned int)n;

    for (i = 0; i < 4; i++) {
        w[i] = D_L00_00160F80[i];
    }
    w += 4;
    w[-1] |= shifted;
    src = (unsigned int *)a3;
    i = n;
    do {
        w[0] = src[0];
        w[1] = src[1];
        src += 2;
        w += 2;
        i--;
    } while (i != 0);
    while ((unsigned int)w & 0xF) {
        *w++ = 0;
    }

    for (i = 0; i < 4; i++) {
        w[i] = D_L00_00160FA0[i];
    }
    w += 4;
    w[-1] |= shifted;
    src = (unsigned int *)a2;
    i = n;
    do {
        *w++ = *src++;
        i--;
    } while (i != 0);
    while ((unsigned int)w & 0xF) {
        *w++ = 0;
    }

    for (i = 0; i < 4; i++) {
        w[i] = D_L00_00160FC0[i];
    }
    w += 4;
    w[-1] |= shifted;
    src = (unsigned int *)a1;
    i = n;
    do {
        w[0] = src[0];
        w[1] = src[1];
        w[2] = src[2];
        src += 3;
        w += 3;
        i--;
    } while (i != 0);
    while ((unsigned int)w & 0xF) {
        *w++ = 0;
    }

    w[0] = 0x15000000 | tail;
    w[1] = 0x11000000;
    w[2] = 0;
    w[3] = 0;
    w += 4;
    start[0] |= (unsigned int)(((char *)w - (char *)start) >> 4) - 1;
    D_L00_00161280 = (int *)w;
}
