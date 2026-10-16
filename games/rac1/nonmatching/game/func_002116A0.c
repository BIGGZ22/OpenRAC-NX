/* moby_mark_joint_chain(moby, count, joints, out): marks in D_001B3080 every joint on the chain of each
   requested joint (the class's table at class+0x1C: entry j+1 points at {u16 n; ...; u8 chain[n] at +4},
   the chain ending in the joint itself), keeps each chain's last joint at D_001B3080[0x80 + i], ends the
   marks with 0xFF after the highest such joint and that joint + 1 at [0x7F], has func_00211808 evaluate
   the marked joints' matrices into the scratchpad (0x70000000 + joint * 64), and copies the requested
   joints' 64-byte matrices to out[i]. The chain loop runs at least once, as retail's does. */
extern unsigned char D_001B3080_b[] __asm__("D_001B3080");
extern void func_00211808_m(void *, void *) __asm__("func_00211808");

void func_002116A0(void *moby, int count, int *joints, void *out) {
    char *tbl = *(char **)(*(char **)((char *)moby + 0x24) + 0x1C);
    int maxj = 0;
    int i, k, n, b = 0;
    unsigned char *rec;
    unsigned int *src;
    unsigned int *dst;

    for (i = 0; i < 0x80; i++) {
        D_001B3080_b[i] = 0;
    }
    for (i = 0; i < count; i++) {
        rec = *(unsigned char **)(tbl + joints[i] * 4 + 4);
        n = *(unsigned short *)rec;
        k = 0;
        do {
            b = rec[4 + k];
            k++;
            n--;
            D_001B3080_b[b] = 1;
        } while (n > 0);
        D_001B3080_b[0x80 + i] = (unsigned char)b;
        if (b > maxj) {
            maxj = b;
        }
    }
    D_001B3080_b[0x7F] = (unsigned char)(maxj + 1);
    D_001B3080_b[maxj] = 0xFF;
    func_00211808_m(moby, D_001B3080_b);
    for (i = 0; i < count; i++) {
        src = (unsigned int *)(0x70000000 + D_001B3080_b[0x80 + i] * 64);
        dst = (unsigned int *)((char *)out + i * 64);
        for (k = 0; k < 16; k++) {
            dst[k] = src[k];
        }
    }
}
