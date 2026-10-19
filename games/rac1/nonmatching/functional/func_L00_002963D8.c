/* functional: high. fcheck: unchecked. Tfrag texture upload list (no C caller; a0 = the VRAM base in bytes, returned as the new base, both in 256-byte units internally). Clears the 0x300-byte table at scratchpad 0x70002000, then for each of the D_L00_00161014 flag bytes at scratchpad 0x70003C00 (0 = skip) takes the word w from D_L00_001C4D00[i] (log2 size in the top 4 bits, address in the low 28) and appends to the packet at D_L00_00161280 one 0x70-byte record (flag bit 1 clear) or two (bit 1 set: level n, then n-1 at address + 4 << 2(n-1)), each the 5-qword template at D_L00_001608E0 with the base (+0x24), mip byte (+0x26, only if n > 6), sizes (+0x30/+0x34), and ref/qwc tags patched; the base each record uses is written to the table (u16 at 0x70002000 + 4i for the first of a pair, + 4i + 2 for the single or second record) and advanced by the record's size. Stops early, dropping the last record, once the base passes 0x3FFB. Then stores the packet pointer and runs the fromSPR DMA (channel 8 at 0x1000D000: MADR D_L00_001C5000, SADR 0x2000, QWC 0x30, CHCR 0x100) and waits for it. The overflow exit retail takes through func_L00_00296620 (a two-instruction tail of this routine) is inlined. Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model), with the DMA completion modelled by the test (CHCR STR clears and the 0x300 bytes are copied when polled). */
extern char *D_L00_00161280;
extern int D_L00_00161014;
extern unsigned int D_L00_001608E0[];
extern unsigned int D_L00_001C4D00[];
extern char D_L00_001C5000[];

static void tfrag_rec_2963D8(char *t8, unsigned int base, int n, unsigned int addr, unsigned int *size)
{
    int k;
    unsigned int s;
    for (k = 0; k < 20; k++) {
        ((unsigned int *)t8)[k] = D_L00_001608E0[k];
    }
    for (k = 0; k < 4; k++) {
        ((unsigned int *)(t8 + 0x40))[k] = 0;
        ((unsigned int *)(t8 + 0x50))[k] = D_L00_001608E0[16 + k];
        ((unsigned int *)(t8 + 0x60))[k] = 0;
    }
    *(unsigned int *)(t8 + 0x48) = 0x53;
    *(unsigned short *)(t8 + 0x24) = (unsigned short)base;
    if (n - 6 > 0) {
        *(unsigned char *)(t8 + 0x26) = (unsigned char)(1u << ((n - 6) & 31));
    }
    *(unsigned int *)(t8 + 0x30) = 1u << (n & 31);
    *(unsigned int *)(t8 + 0x34) = 1u << (n & 31);
    s = (1u << (n & 31)) << (n & 31);
    *size = s;
    s >>= 4;
    *(unsigned int *)(t8 + 0x50) = 0x8000 + s;
    *(unsigned int *)(t8 + 0x64) = addr;
    *(unsigned int *)(t8 + 0x60) = 0x30000000 + s;
    *(unsigned int *)(t8 + 0x6C) = 0x50000000 + s;
}

int func_L00_002963D8(unsigned int base)
{
    char *t8 = D_L00_00161280;
    signed char *flag = (signed char *)0x70003C00;
    signed char *end = flag + D_L00_00161014;
    unsigned short *tab = (unsigned short *)0x70002000;
    unsigned int *w = D_L00_001C4D00;
    volatile unsigned int *dma = (volatile unsigned int *)0x1000D000;
    int i;

    base >>= 8;
    for (i = 0; i < 0x300 / 4; i++) {
        ((unsigned int *)0x70002000)[i] = 0;
    }
    while (flag != end) {
        int f;
        unsigned int word;
        unsigned int addr;
        unsigned int size;
        int n;
        tab += 2;
        f = *flag++;
        word = *w++;
        if (f == 0) {
            continue;
        }
        n = (int)(word >> 28);
        addr = (word << 4) >> 4;
        if (f & 2) {
            tfrag_rec_2963D8(t8, base, n, addr, &size);
            tab[-2] = (unsigned short)base;
            base += size >> 8;
            t8 += 0x70;
            if ((int)(base - 0x3FFB) > 0) {
                t8 -= 0x70;
                break;
            }
        }
        n = n - 1;
        tfrag_rec_2963D8(t8, base, n, addr, &size);
        addr += size << 2;
        *(unsigned int *)(t8 + 0x64) = addr;
        tab[-1] = (unsigned short)base;
        base += size >> 8;
        t8 += 0x70;
        if ((int)(base - 0x3FFB) > 0) {
            t8 -= 0x70;
            break;
        }
    }
    D_L00_00161280 = t8;
    dma[4] = (unsigned int)(unsigned long)D_L00_001C5000;
    dma[0x20] = 0x2000;
    dma[8] = 0x30;
    dma[0] = 0x100;
    while (dma[0] & 0x100) {
    }
    return (int)(base << 8);
}
