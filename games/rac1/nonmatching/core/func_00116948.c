/* newlib strncmp: compares at most n bytes of s1 and s2. Returns 0 when n is 0,
   or when the bytes are equal up to a NUL or up to n; otherwise the difference of
   the first two bytes that differ, as unsigned chars. Retail compares 8- and
   16-byte blocks where both pointers are aligned and at least that many bytes
   remain; the result is the same as the byte loop. */
int func_00116948(const char *s1, const char *s2, unsigned int n) {
    unsigned int i;

    for (i = 0; i < n; i++) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];

        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }
        if (c1 == 0) {
            return 0;
        }
    }
    return 0;
}
