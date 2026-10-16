/* strcpy: copies src to dst including the terminating NUL and returns dst.
   The aligned fast paths in the assembly copy only whole 8- or 16-byte blocks
   that hold no NUL; everything else goes byte by byte, so nothing past the NUL
   is written. */
char *func_001166FC(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++) != 0) {
    }
    return dst;
}
