/* Copies n bytes from src to dst, safe when the two overlap: copies from the end when src is
   below dst and the source range reaches past dst, otherwise from the start. Returns dst. */
void *func_001152F8(void *dst, const void *src, unsigned int n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    unsigned int i;

    if ((unsigned int)s < (unsigned int)d && (unsigned int)d < (unsigned int)s + n) {
        for (i = n; i != 0; i--) {
            d[i - 1] = s[i - 1];
        }
    } else {
        for (i = 0; i < n; i++) {
            d[i] = s[i];
        }
    }
    return dst;
}
