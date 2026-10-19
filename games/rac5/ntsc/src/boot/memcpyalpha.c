/* small forward byte copy, retail 0x01ECF4D8 (36 bytes + nop padding):
 *   if (n) { i = n - 1; do { *d++ = *s++; } while (i-- != 0); }
 */
void memcpyalpha(void *dst, const void *src, unsigned int n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--)
        *d++ = *s++;
}
