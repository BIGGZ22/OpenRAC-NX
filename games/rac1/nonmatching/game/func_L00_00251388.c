extern char D_L00_0017FFC0[];

/* LightMoby: a light vector for a moby. Its 0x38 word packs three 8-bit
   indices: the first picks a 64-byte entry in D_L00_0017FFC0 (its vector at
   +0x10), the second another entry, and the third a blend in 1/256 steps
   (0 gives the first vector). The result is written to out as 16 bytes. The
   blend keeps the first vector's w. */
void func_L00_00251388(char *moby, float *out) {
    unsigned int v = *(unsigned int *)(moby + 0x38);
    unsigned int ia = (v & 0xFF) << 6;
    unsigned int ib = ((v >> 8) & 0xFF) << 6;
    unsigned int k = (v >> 16) & 0xFF;
    float *a = (float *)(D_L00_0017FFC0 + ia + 0x10);
    float *b = (float *)(D_L00_0017FFC0 + ib + 0x10);
    float t;
    int i;

    if (k == 0) {
        for (i = 0; i < 4; i++) {
            out[i] = a[i];
        }
        return;
    }
    t = (float)(k << 4) / 4096.0f;
    out[0] = a[0] * (1.0f - t) + b[0] * t;
    out[1] = a[1] * (1.0f - t) + b[1] * t;
    out[2] = a[2] * (1.0f - t) + b[2] * t;
    out[3] = a[3];
}
