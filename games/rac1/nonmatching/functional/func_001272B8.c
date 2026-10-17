/* functional: high. Adds 24 blocks of 16 halfwords from arg1 and arg2 (paddh, wrapping), clamps each sum to 0..255 (pminh, pmaxh with 255 from the constant table and 0) and packs the low bytes to 16 bytes at arg0 per block (ppacb). The clamp table lives at D_00127360 in retail; its value 255 is used directly. */
void func_001272B8(int arg0, int arg1, int arg2) {
    unsigned char *dst = (unsigned char *)arg0;
    const short *a = (const short *)arg1;
    const short *b = (const short *)arg2;
    int blk, i;

    for (blk = 0; blk < 24; blk++) {
        for (i = 0; i < 16; i++) {
            int s = (short)(a[i] + b[i]);
            if (s > 255) {
                s = 255;
            }
            if (s < 0) {
                s = 0;
            }
            dst[i] = (unsigned char)s;
        }
        dst += 16;
        a += 16;
        b += 16;
    }
}
