/* functional: high. setImageTag(buf, addr, w, h): writes the DMA/GIF tag chain for a w/16 x h/16 grid of 16x16 blocks (header qwords, one 0x60-byte entry per block, then the 0x20-byte trailer and end tag) into buf; same words at the same offsets as retail. */
extern int D_0015EF84;
extern int D_0015EFB8;
extern char D_00151EF0[];

void func_0023C5E0(u64 *buf, unsigned int addr, int w, int h) {
    u64 *p = buf;
    int rows = w >> 4;
    int cols = h >> 4;
    int row, col;
    u64 g;

    p[2] = (1ULL << 60) | 2;
    p[3] = 0xE;
    p[0] = 0x10000003;
    p[5] = 0x50;
    p[6] = (1ULL << 36) | 0x10;
    p[1] = 0;
    p[7] = 0x52;
    g = (u64)(long long)D_0015EF84;
    g = (u64)((long long)g >> 8);
    p[4] = ((g << 32)) | (1ULL << 51);
    p += 8;

    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            *(u32 *)((char *)p + 0x54) = addr & 0xFFFFFFF;
            p[0] = 0x10000004;
            p[4] = ((u64)(row << 4) << 32) | ((u64)(col << 4) << 48);
            p[1] = 0;
            addr += 0x400;
            p[2] = (1ULL << 60) | 2;
            p[3] = 0xE;
            p[5] = 0x51;
            p[6] = 0;
            p[7] = 0x53;
            p[8] = (1ULL << 59) | 0x40;
            p[9] = 0;
            *(u32 *)((char *)p + 0x50) = 0x30000040;
            p[11] = 0;
            p += 12;
        }
    }

    {
        u32 *q = (u32 *)p;
        q[0] = 0x30000009;
        q[3] = 0x50000009;
        q[1] = (u32)(D_0015EFB8 + 0xC0) & 0xFFFFFFF;
        q[2] = 0;
        q[4] = 0x30000025;
        q[5] = (u32)D_00151EF0;
        q[7] = 0x50000025;
        q[6] = 0;
        p += 4;
    }
    p[0] = 0x70000000;
    p[1] = 0;
}
