/* functional: low. Three-level sparse table lookup in assembly with its own register convention: it reads $24 = table base and $3, $2, $1 = the three indices (outer first), and returns the word in $1 (0 when any level is out of range or empty). Written with the registers as parameters in that order (i1 = $1, i2 = $2, i3 = $3, tab = $24); the port must bind them that way, which is a guess about how callers are bound. Each level: u16 first-index, u16 count, then entries (u16 word offsets at level 1, u32 byte offsets at level 2, final u32 values at level 3). */
unsigned int func_001EFD70(int i1, int i2, int i3, void *tab) {
    unsigned char *t = (unsigned char *)tab;
    unsigned char *l2;
    unsigned char *l3;
    unsigned int off;
    int idx, cnt;

    idx = i3 - *(unsigned short *)(t + 0);
    cnt = *(unsigned short *)(t + 2) - idx;
    if (idx < 0 || cnt <= 0) {
        return 0;
    }
    off = (unsigned int)*(unsigned short *)(t + 4 + idx * 2) << 2;
    if (off == 0) {
        return 0;
    }
    l2 = t + off;
    idx = i2 - *(unsigned short *)(l2 + 0);
    cnt = *(unsigned short *)(l2 + 2) - idx;
    if (idx < 0 || cnt <= 0) {
        return 0;
    }
    off = *(unsigned int *)(l2 + 4 + idx * 4);
    if (off == 0) {
        return 0;
    }
    l3 = t + off;
    idx = i1 - *(unsigned short *)(l3 + 0);
    cnt = *(unsigned short *)(l3 + 2) - idx;
    if (idx < 0 || cnt <= 0) {
        return 0;
    }
    return *(unsigned int *)(l3 + 4 + idx * 4);
}
