/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001150D4.c (games/rac1): func_001150D4, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: high. memchr: a plain forward byte loop in place of retail's 16-byte SIMD pre-scan (which only skips blocks that hold no match and then hands over to the same byte loop), so the returned pointer, the reads and the n == 0 case are the same; compares against (c & 0xFF), n counted unsigned. */

void *memchr(const void *s, int c, unsigned int n) {
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
