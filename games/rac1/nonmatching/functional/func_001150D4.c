/* functional: high. memchr: a plain forward byte loop in place of retail's 16-byte SIMD pre-scan (which only skips blocks that hold no match and then hands over to the same byte loop), so the returned pointer, the reads and the n == 0 case are the same; compares against (c & 0xFF), n counted unsigned. */
void *func_001150D4(const void *s, int c, unsigned int n) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned char ch = (unsigned char)c;
    while (n != 0) {
        if (*p == ch) {
            return (void *)p;
        }
        p++;
        n--;
    }
    return 0;
}
