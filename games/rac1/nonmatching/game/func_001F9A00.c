/* Copies len bytes from src to dst (no overlap). It picks the widest unit the
   alignment of dst, src and the end allows: 16-byte, then 4-byte, then bytes. */
void func_001F9A00(void *dst_, void *src_, int len)
{
    unsigned char *d = (unsigned char *)dst_;
    const unsigned char *s = (const unsigned char *)src_;
    unsigned char *end = d + len;
    unsigned long bits = (unsigned long)d | (unsigned long)s | (unsigned long)end;
    int i;

    if ((bits & 0xF) == 0) {
        do {
            for (i = 0; i < 16; i++) {
                d[i] = s[i];
            }
            s += 16;
            d += 16;
        } while (d != end);
    } else if ((bits & 0x3) == 0) {
        do {
            *(unsigned int *)d = *(const unsigned int *)s;
            s += 4;
            d += 4;
        } while (d != end);
    } else {
        do {
            *d = *s;
            s++;
            d++;
        } while (d != end);
    }
}
