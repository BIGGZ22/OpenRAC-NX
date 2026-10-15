/* memcpy(dst, src, n). Retail picks its loop by the low bits of dst, src and dst+n:
   a 16-byte loop when all three are 16-byte aligned, a 4-byte loop when all three are
   4-byte aligned, else a byte loop. Each step loads its unit fully before storing it. */
void func_L00_001FF040(void *dst, const void *src, unsigned int n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    unsigned char *end = d + n;
    unsigned long bits = (unsigned long)d | (unsigned long)s | (unsigned long)end;
    unsigned char t[16];
    unsigned int i;

    if ((bits & 0xF) == 0) {
        while (d != end) {
            for (i = 0; i < 16; i++) {
                t[i] = s[i];
            }
            for (i = 0; i < 16; i++) {
                d[i] = t[i];
            }
            d += 16;
            s += 16;
        }
    } else if ((bits & 0x3) == 0) {
        while (d != end) {
            for (i = 0; i < 4; i++) {
                t[i] = s[i];
            }
            for (i = 0; i < 4; i++) {
                d[i] = t[i];
            }
            d += 4;
            s += 4;
        }
    } else {
        while (d != end) {
            unsigned char c = *s;
            s++;
            *d = c;
            d++;
        }
    }
}
