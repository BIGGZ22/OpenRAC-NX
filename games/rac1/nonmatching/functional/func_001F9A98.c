/* functional: high. fcheck: unchecked. Quadword copy: copies 16 bytes at a time from src to dst until n (bytes) runs out, always copying at least one quadword (retail loops on n > 0 after the first copy, so n <= 0 still copies 16 bytes). Retail uses lq/sq, which ignore the low four address bits; callers pass 16-byte-aligned buffers. Differential test: 9 cases (n from -5 to 256) agree with retail in an EE emulator. */
void func_001F9A98(void *dst, void *src, int n) {
    unsigned int *d = (unsigned int *)dst;
    const unsigned int *s = (const unsigned int *)src;
    do {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
        d += 4;
        s += 4;
        n -= 16;
    } while (n > 0);
}
