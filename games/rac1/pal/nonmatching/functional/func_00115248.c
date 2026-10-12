/* functional: high. memcpy: 32-byte and 8-byte moves when dest and src are both 16-byte aligned and n >= 32, then byte moves for the rest; returns dest. Reads and writes are the same bytes in the same order as the assembly; counting is unsigned, so n >= 2^31 is handled too. */
typedef int u128 __attribute__((mode(TI)));

void *func_00115248(void *dest, const void *src, unsigned int n) {
    char *ret = dest;
    char *d = dest;
    const char *s = src;
    u128 a;
    u128 b;
    if (n >= 0x20 && (((unsigned long)s | (unsigned long)d) & 0xF) == 0) {
        do {
            a = *(u128 *)s;
            n -= 0x20;
            s += 0x10;
            *(u128 *)d = a;
            d += 0x10;
            b = *(u128 *)s;
            s += 0x10;
            *(u128 *)d = b;
            d += 0x10;
        } while (n >= 0x20);
        if (n >= 8) {
            do {
                long v = *(long *)s;
                n -= 8;
                s += 8;
                *(long *)d = v;
                d += 8;
            } while (n >= 8);
        }
    }
    while (n != 0) {
        *d++ = *s++;
        n--;
    }
    return ret;
}
