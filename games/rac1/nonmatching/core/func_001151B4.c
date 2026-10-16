/* newlib memcmp: compares n bytes at a and b and returns the difference of the
   first two bytes that differ, or 0. When both pointers are 16-byte aligned and
   at least 16 bytes remain, whole 16-byte blocks are skipped while they are
   equal (the first differing block is left for the byte loop). */
int func_001151B4(const void *a, const void *b, unsigned int n) {
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;
    unsigned int i;

    if (n >= 16 && (((unsigned int)p | (unsigned int)q) & 0xF) == 0) {
        while (n >= 16) {
            for (i = 0; i < 16; i++) {
                if (p[i] != q[i]) {
                    break;
                }
            }
            if (i != 16) {
                break;
            }
            p += 16;
            q += 16;
            n -= 16;
        }
    }

    for (i = 0; i < n; i++) {
        unsigned int c1 = p[i];
        unsigned int c2 = q[i];
        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }
    }
    return 0;
}
