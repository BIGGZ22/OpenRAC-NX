/* functional: high. fcheck: unchecked. Builds a GS grid packet for MOBY (retail: VU0 + lq/sq). Writes an 8x8 grid of (x, y) float pairs to scratchpad 0x70000000 for each enabled cell, then appends to the DMA chain at D_L02_00161240 (gp-0x5AC0): the 5-qword header from D_L02_001C48A0 (word 4 |= 0x7C), the qword from D_L02_00160F80 (word 3 |= 0x7C << 16), 124 vertices {the grid pair at 0x70000000 | idx[k] (idx: s16 table at scratchpad 0x70002800), z}, zero padding to 16 bytes, the qword count - 1 or'd into the first tag, then three tags (ref of the cell's texture block, ref of D_L02_001CBC40, a 0x15000000 | (2 * flag + 10) qword). Cells come from moby+0x18/+0x1A (u16 extents, step 7), one signed flag byte each at scratchpad 0x70002000 (0 = skip), the texture address at moby+0x34 advancing 0x200 per cell; the origin is moby+0x00 and the step moby+0x10.xy (grid step), 7 * that per cell. Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model), bit-exact (fltol 0). Negative scratchpad indices (which would fault on retail) and a packet pointer not 16-aligned (retail sq ignores the low bits) are not reproduced. */
extern unsigned int *D_L02_00161240;
extern unsigned int D_L02_001C48A0[];
extern unsigned int D_L02_00160F80[];
extern unsigned int D_L02_001CBC40[];

void func_L02_00250A58(char *moby)
{
    float bx = ((float *)moby)[0] + 0.0f;
    float by = ((float *)moby)[1] + 0.0f;
    float bz = ((float *)moby)[2] + 0.0f;
    float ax = ((float *)(moby + 0x10))[0];
    float ay = ((float *)(moby + 0x10))[1];
    float sx = ax * 7.0f;
    float sy = ay * 7.0f;
    unsigned int tex = *(unsigned int *)(moby + 0x34);
    int rows = *(unsigned short *)(moby + 0x1A);
    signed char *flag = (signed char *)0x70002000;
    unsigned int zbits = *(unsigned int *)&bz;
    float cy = by;

    do {
        float cx = bx;
        int cols = *(unsigned short *)(moby + 0x18);
        do {
            int n = *flag++;
            if (n != 0) {
                float *g = (float *)0x70000000;
                float y = cy + 0.0f;
                unsigned int *p0;
                unsigned int *p;
                short *idx;
                int r, c, k;
                for (r = 0; r < 8; r++) {
                    float x = cx + 0.0f;
                    for (c = 0; c < 8; c++) {
                        g[0] = x;
                        g[1] = y;
                        g += 2;
                        x = x + ax;
                    }
                    y = y + ay;
                }
                p0 = D_L02_00161240;
                p = p0;
                for (k = 0; k < 20; k++) {
                    p[k] = D_L02_001C48A0[k];
                }
                p[4] |= 0x7C;
                for (k = 0; k < 4; k++) {
                    p[20 + k] = D_L02_00160F80[k];
                }
                p[23] |= 0x7C << 16;
                p += 24;
                idx = (short *)0x70002800;
                for (k = 0; k < 124; k++) {
                    unsigned int *src = (unsigned int *)(unsigned long)(0x70000000u | (unsigned int)(int)idx[k]);
                    p[0] = src[0];
                    p[1] = src[1];
                    p[2] = zbits;
                    p += 3;
                }
                while (((unsigned long)p & 0xF) != 0) {
                    *p++ = 0;
                }
                p0[0] |= (unsigned int)(((p - p0) >> 2) - 1);
                p[0] = 0x30000020;
                p[1] = tex;
                p[2] = 0;
                p[3] = 0;
                p[4] = 0x3000003F;
                p[5] = (unsigned int)(unsigned long)D_L02_001CBC40;
                p[6] = 0;
                p[7] = 0;
                p[8] = 0x10000000;
                p[9] = 0;
                p[10] = 0x15000000 | (unsigned int)((n - 1) * 2 + 0xC);
                p[11] = 0;
                D_L02_00161240 = p + 12;
            }
            tex += 0x200;
            cols -= 7;
            cx = cx + sx;
        } while (cols > 0);
        rows -= 7;
        cy = cy + sy;
    } while (rows > 0);
}
