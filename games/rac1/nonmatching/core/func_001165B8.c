/* strcmp: compares s1 and s2 as unsigned bytes up to the first difference or the
   terminating NUL; returns the difference of the two bytes there, or 0. The
   assembly's 8- and 16-byte paths only skip blocks whose bytes are equal, so the
   result is the same as this byte loop. Reads only. */
int func_001165B8(const char *s1, const char *s2) {
    unsigned char c1;
    unsigned char c2;
    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
    } while (c1 == c2 && c1 != 0);
    return (int)c1 - (int)c2;
}
